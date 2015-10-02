/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 *******************************************************************************/


#include <queue>

#include <Windows.h>
#include <minmax.h>
#include <objidl.h>
#include <gdiplus.h>

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/cef_render_handler.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "native_extensions/image_util_win.h"
#include "native_extensions/pageimage.h"


void CALLBACK PaintComplete(HWND, UINT, UINT_PTR, DWORD);
class OffscreenClientHandler;


extern CefRefPtr<Zephyros::ClientHandler> g_handler;
CefRefPtr<OffscreenClientHandler> g_offscreenHandler = NULL;


typedef struct
{
	CallbackId callbackId;
	String url;
	int width;
} QueueItem;


class OffscreenClientHandler : public CefClient, public CefLifeSpanHandler, public CefRenderHandler
{
private:
	std::queue<QueueItem*> m_queue;
	Gdiplus::Bitmap* m_image;
	UINT_PTR m_timerId;
	int m_numPainted;
	CefRefPtr<CefBrowser> m_browser;

public:
	OffscreenClientHandler()
		: m_timerId(0), m_numPainted(0), m_browser(NULL)
	{
		m_image = new Gdiplus::Bitmap(Zephyros::PageImage::ImageWidth, Zephyros::PageImage::ImageHeight, PixelFormat32bppARGB);
	}

	~OffscreenClientHandler()
	{
	}

	void AddToQueue(String url, int width, CallbackId callbackId)
	{
		QueueItem* item = new QueueItem;

		item->callbackId = callbackId;
		item->url = url;
		item->width = min(width, Zephyros::PageImage::ImageWidth);

		m_queue.push(item);

		if (m_queue.size() == 1)
		{
			m_timerId = 0;
			m_numPainted = 0;

			if (m_browser && m_browser.get())
				m_browser->GetMainFrame()->LoadURL(url);
		}
	}

	void ReturnResult()
	{
		if (m_timerId != 0)
			KillTimer(NULL, m_timerId);

		if (m_queue.size() == 0)
			return;

		QueueItem* item = m_queue.front();
		m_queue.pop();

		// resize the image if needed
		Gdiplus::Bitmap* pImage = m_image;
		if (item->width < Zephyros::PageImage::ImageWidth)
		{
			int imageHeight = (int) (((long) item->width * Zephyros::PageImage::ImageHeight) / Zephyros::PageImage::ImageWidth);
			pImage = new Gdiplus::Bitmap(item->width, imageHeight, PixelFormat32bppARGB);

			Gdiplus::Graphics g(pImage);
			g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
			g.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic);
			g.DrawImage(m_image, 0, 0, item->width, imageHeight);
		}

		/*
		// for debugging
		CLSID clsidEncoder;
		ImageUtil::GetEncoderClsid(L"image/png", &clsidEncoder);
		pImage->Save(TEXT("C:\\Users\\ghost\\Documents\\tmp.png"), &clsidEncoder);
		//*/

		// create a PNG and base64-encode it
		BYTE* pData = NULL;
		DWORD length = 0;
		ImageUtil::BitmapToPNGData(pImage, &pData, &length);
		Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
		args->SetString(0, TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length));
		
		g_handler->GetClientExtensionHandler()->InvokeCallback(item->callbackId, args);

		delete[] pData;
		if (pImage != m_image)
			delete pImage;
		delete item;

		// process the next item in the queue
		m_numPainted = 0;
		m_timerId = 0;
		if (m_queue.size() > 0)
			m_browser->GetMainFrame()->LoadURL(m_queue.front()->url);
	}

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
    {
        return this;
    }

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		m_browser = browser;
	}

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser)
	{
		m_browser = NULL;

		if (m_image != NULL)
		{
			delete m_image;
			m_image = NULL;
		}
	}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
	{
        return this;
    }

    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
    {
        rect = CefRect(0, 0, 1024, 768);
        return true;
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
    {
		if (m_image == NULL || m_queue.size() == 0)
			return;

		// copy the buffer into the bitmap
		Gdiplus::BitmapData data;
		Gdiplus::Rect rect(0, 0, Zephyros::PageImage::ImageWidth, Zephyros::PageImage::ImageHeight);
		m_image->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &data);
		int w = min(Zephyros::PageImage::ImageWidth, width);
		int h = min(Zephyros::PageImage::ImageHeight, height);
		for (int y = 0; y < h; ++y)
			memcpy((BYTE*) data.Scan0 + y * data.Stride, (BYTE*) buffer + y * 4 * w, 4 * w);
		m_image->UnlockBits(&data);

		m_numPainted++;
		if (m_numPainted >= 100)
			ReturnResult();
		else
			m_timerId = SetTimer(NULL, m_timerId, 3000, PaintComplete);
    }

public:
    IMPLEMENT_REFCOUNTING(OffscreenClientHandler);
};


void CALLBACK PaintComplete(HWND hWnd, UINT uMsg, UINT_PTR timerId, DWORD dwTime)
{
	if (g_offscreenHandler && g_offscreenHandler.get())
		g_offscreenHandler->ReturnResult();
}


namespace Zephyros {
namespace PageImage {

void GetPageImageForURL(CallbackId callback, String url, int width)
{
	if (g_offscreenHandler == NULL)
	{
		g_offscreenHandler = new OffscreenClientHandler();

		CefWindowInfo info;
		info.SetAsWindowless(NULL, false);

		CefBrowserSettings settings;		

		CefBrowserHost::CreateBrowser(info, g_offscreenHandler.get(), url, settings, NULL);
	}

	g_offscreenHandler->AddToQueue(url, width, callback);
}

} // namespace PageImage
} // namespace Zephyros
