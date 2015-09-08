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


#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "lib/cef/include/base/cef_bind.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/cef_path_util.h"
#include "lib/cef/include/cef_process_util.h"
#include "lib/cef/include/cef_trace.h"
#include "lib/cef/include/wrapper/cef_closure_task.h"
#include "lib/cef/include/wrapper/cef_stream_resource_handler.h"

#include "zephyros.h"
#include "base/app.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#include "base/cef/resource_util.h"

#include "native_extensions/file_util.h"
#include "native_extensions/path.h"

#include "util/string_util.h"


bool ProcessMessageDelegate::OnProcessMessageReceived(
    CefRefPtr<Zephyros::ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	return false;
}


namespace Zephyros {

int ClientHandler::m_nBrowserCount = 0;


ClientHandler::ClientHandler()
  : m_nBrowserId(0),
    m_bIsClosing(false),
    m_mainHwnd(NULL),
	m_clientExtensionHandler(new ClientExtensionHandler())
{
    m_processMessageDelegates.insert(static_cast< CefRefPtr<ProcessMessageDelegate> >(m_clientExtensionHandler));
    Zephyros::GetNativeExtensions()->AddNativeExtensions(m_clientExtensionHandler.get());
	InitializeMIMETypes();
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::InitializeMIMETypes()
{
    m_mimeTypes[TEXT(".html")] = TEXT("text/html");
    m_mimeTypes[TEXT(".htm")] = TEXT("text/html");
    m_mimeTypes[TEXT(".css")] = TEXT("text/css");
    m_mimeTypes[TEXT(".json")] = TEXT("application/json");
    m_mimeTypes[TEXT(".js")] = TEXT("text/javascript");
    m_mimeTypes[TEXT(".xml")] = TEXT("application/xml");
    m_mimeTypes[TEXT(".txt")] = TEXT("text/plain");
    m_mimeTypes[TEXT(".log")] = TEXT("text/plain");
    
    // fonts
    m_mimeTypes[TEXT(".pdf")] = TEXT("application/pdf");
    m_mimeTypes[TEXT(".otf")] = TEXT("application/x-font-otf");
    m_mimeTypes[TEXT(".ttf")] = TEXT("application/x-font-ttf");
    m_mimeTypes[TEXT(".pfa")] = TEXT("application/x-font-type1");
    m_mimeTypes[TEXT(".pfb")] = TEXT("application/x-font-type1");
    m_mimeTypes[TEXT(".pfm")] = TEXT("application/x-font-type1");
    m_mimeTypes[TEXT(".afm")] = TEXT("application/x-font-type1");
    m_mimeTypes[TEXT(".woff")] = TEXT("application/font-woff");
    
    // images
    m_mimeTypes[TEXT(".bmp'")] = TEXT("image/bmp");
    m_mimeTypes[TEXT(".cgm")] = TEXT("image/cgm");
    m_mimeTypes[TEXT(".gif")] = TEXT("image/gif");
    m_mimeTypes[TEXT(".ief")] = TEXT("image/ief");
    m_mimeTypes[TEXT(".jpeg")] = TEXT("image/jpeg");
    m_mimeTypes[TEXT(".jpg")] = TEXT("image/jpeg");
    m_mimeTypes[TEXT(".jpe")] = TEXT("image/jpeg");
    m_mimeTypes[TEXT(".png")] = TEXT("image/png");
    m_mimeTypes[TEXT(".sgi")] = TEXT("image/sgi");
    m_mimeTypes[TEXT(".svg")] = TEXT("image/svg+xml");
    m_mimeTypes[TEXT(".svgz")] = TEXT("image/svg+xml");
    m_mimeTypes[TEXT(".tiff")] = TEXT("image/tiff");
    m_mimeTypes[TEXT(".tif")] = TEXT("image/tiff");
    
    // audio
    m_mimeTypes[TEXT(".au")] = TEXT("audio/basic");
    m_mimeTypes[TEXT(".snd")] = TEXT("audio/basic");
    m_mimeTypes[TEXT(".mid")] = TEXT("audio/midi");
    m_mimeTypes[TEXT(".midi")] = TEXT("audio/midi");
    m_mimeTypes[TEXT(".rmi")] = TEXT("audio/midi");
    m_mimeTypes[TEXT(".mp4a")] = TEXT("audio/mp4");
    m_mimeTypes[TEXT(".mpga")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".mp2")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".mp2a")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".mp3")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".m2a")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".m3a")] = TEXT("audio/mpeg");
    m_mimeTypes[TEXT(".oga")] = TEXT("audio/ogg");
    m_mimeTypes[TEXT(".ogg")] = TEXT("audio/ogg");
    m_mimeTypes[TEXT(".spx")] = TEXT("audio/ogg");
    m_mimeTypes[TEXT(".s3m")] = TEXT("audio/s3m");
    m_mimeTypes[TEXT(".sil")] = TEXT("audio/silk");
    m_mimeTypes[TEXT(".rip")] = TEXT("audio/vnd.rip");
    m_mimeTypes[TEXT(".weba")] = TEXT("audio/webm");
    m_mimeTypes[TEXT(".aac")] = TEXT("audio/x-aac");
    m_mimeTypes[TEXT(".aif")] = TEXT("audio/x-aiff");
    m_mimeTypes[TEXT(".aiff")] = TEXT("audio/x-aiff");
    m_mimeTypes[TEXT(".aifc")] = TEXT("audio/x-aiff");
    m_mimeTypes[TEXT(".caf")] = TEXT("audio/x-caf");
    m_mimeTypes[TEXT(".flac")] = TEXT("audio/x-flac");
    m_mimeTypes[TEXT(".m3u")] = TEXT("audio/x-mpegurl");
    m_mimeTypes[TEXT(".wax")] = TEXT("audio/x-ms-wax");
    m_mimeTypes[TEXT(".wma")] = TEXT("audio/x-ms-wma");
    m_mimeTypes[TEXT(".ram")] = TEXT("audio/x-pn-realaudio");
    m_mimeTypes[TEXT(".ra")] = TEXT("audio/x-pn-realaudio");
    m_mimeTypes[TEXT(".rmp")] = TEXT("audio/x-pn-realaudio-plugin");
    m_mimeTypes[TEXT(".wav")] = TEXT("audio/x-wav");
    m_mimeTypes[TEXT(".xm")] = TEXT("audio/xm");

    // video
    m_mimeTypes[TEXT(".3gp")] = TEXT("video/3gpp");
    m_mimeTypes[TEXT(".3g2")] = TEXT("video/3gpp2");
    m_mimeTypes[TEXT(".h261")] = TEXT("video/h261");
    m_mimeTypes[TEXT(".h263")] = TEXT("video/h263");
    m_mimeTypes[TEXT(".h264")] = TEXT("video/h264");
    m_mimeTypes[TEXT(".jpgv")] = TEXT("video/jpeg");
    m_mimeTypes[TEXT(".jpm")] = TEXT("video/jpm");
    m_mimeTypes[TEXT(".jpgm")] = TEXT("video/jpm");
    m_mimeTypes[TEXT(".mj2")] = TEXT("video/mj2");
    m_mimeTypes[TEXT(".mjp2")] = TEXT("video/mj2");
    m_mimeTypes[TEXT(".mp4")] = TEXT("video/mp4");
    m_mimeTypes[TEXT(".mp4v")] = TEXT("video/mp4");
    m_mimeTypes[TEXT(".mpg4")] = TEXT("video/mp4");
    m_mimeTypes[TEXT(".mpeg")] = TEXT("video/mpeg");
    m_mimeTypes[TEXT(".mpg")] = TEXT("video/mpeg");
    m_mimeTypes[TEXT(".mpe")] = TEXT("video/mpeg");
    m_mimeTypes[TEXT(".m1v")] = TEXT("video/mpeg");
    m_mimeTypes[TEXT(".m2v")] = TEXT("video/mpeg");
    m_mimeTypes[TEXT(".ogv")] = TEXT("video/ogg");
    m_mimeTypes[TEXT(".qt")] = TEXT("video/quicktime");
    m_mimeTypes[TEXT(".mov")] = TEXT("video/quicktime");
    m_mimeTypes[TEXT(".mxu")] = TEXT("video/vnd.mpegurl");
    m_mimeTypes[TEXT(".m4u")] = TEXT("video/vnd.mpegurl");
    m_mimeTypes[TEXT(".pyv")] = TEXT("video/vnd.ms-playready.media.pyv");
    m_mimeTypes[TEXT(".uvu")] = TEXT("video/vnd.uvvu.mp4");
    m_mimeTypes[TEXT(".uvvu")] = TEXT("video/vnd.uvvu.mp4");
    m_mimeTypes[TEXT(".viv")] = TEXT("video/vnd.vivo");
    m_mimeTypes[TEXT(".webm")] = TEXT("video/webm");
    m_mimeTypes[TEXT(".f4v")] = TEXT("video/x-f4v");
    m_mimeTypes[TEXT(".fli")] = TEXT("video/x-fli");
    m_mimeTypes[TEXT(".flv")] = TEXT("video/x-flv");
    m_mimeTypes[TEXT(".m4v")] = TEXT("video/x-m4v");
    m_mimeTypes[TEXT(".mng")] = TEXT("video/x-mng");
    m_mimeTypes[TEXT(".asf")] = TEXT("video/x-ms-asf");
    m_mimeTypes[TEXT(".asx")] = TEXT("video/x-ms-asf");
    m_mimeTypes[TEXT(".vob")] = TEXT("video/x-ms-vob");
    m_mimeTypes[TEXT(".wm")] = TEXT("video/x-ms-wm");
    m_mimeTypes[TEXT(".wmv")] = TEXT("video/x-ms-wmv");
    m_mimeTypes[TEXT(".wmx")] = TEXT("video/x-ms-wmx");
    m_mimeTypes[TEXT(".wvx")] = TEXT("video/x-ms-wvx");
    m_mimeTypes[TEXT(".avi")] = TEXT("video/x-msvideo");
    m_mimeTypes[TEXT(".movie")] = TEXT("video/x-sgi-movie");
    m_mimeTypes[TEXT(".smv")] = TEXT("video/x-smv");
}

bool ClientHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	CEF_REQUIRE_UI_THREAD();

	// execute delegate callbacks
    for (CefRefPtr<ProcessMessageDelegate> delegate : m_processMessageDelegates)
        if (delegate->OnProcessMessageReceived(this, browser, source_process, message))
            return true;
    
    return false;
}

void ClientHandler::ReleaseCefObjects()
{
    for (CefRefPtr<ProcessMessageDelegate> delegate : m_processMessageDelegates)
        delegate->ReleaseCefObjects();
}

void ClientHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	CEF_REQUIRE_UI_THREAD();

    // remove all popup menu items: don't show the native context menu
    int numItems = model->GetCount();
    for (int i = 0; i < numItems; ++i)
        model->RemoveAt(0);
}

bool ClientHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
    return true;
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
	CEF_REQUIRE_UI_THREAD();
	App::Log(String(message));

	return false;
}

bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask)
{
	CEF_REQUIRE_UI_THREAD();

	Zephyros::GetNativeExtensions()->GetDroppedURLs().clear();
    
    std::vector<CefString> names;
    dragData->GetFileNames(names);
    for (CefString name : names)
    {
        String path = name;
        if (FileUtil::ExistsFile(path))
            Zephyros::GetNativeExtensions()->GetDroppedURLs().push_back(Path(path));
    }
    
    String linkUrl = dragData->GetLinkURL();
    if (!linkUrl.empty())
        Zephyros::GetNativeExtensions()->GetDroppedURLs().push_back(Path(linkUrl));

	return false;
}

bool ClientHandler::OnPreKeyEvent(
    CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut)
{
	CEF_REQUIRE_UI_THREAD();
	return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	if (!GetBrowser())
	{
		base::AutoLock lock_scope(m_lock);
		// we need to keep the main child window, but not popup windows
		m_browser = browser;
		m_nBrowserId = browser->GetIdentifier();
	}
	else if (browser->IsPopup())
	{
		// add to the list of popup browsers.
		m_popupBrowsers.push_back(browser);

		// give focus to the popup browser. Perform asynchronously because the
		// parent window may attempt to keep focus after launching the popup.
		CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SetFocus, browser->GetHost().get(), true));
	}

	m_nBrowserCount++;
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this process.
	if (GetBrowserId() == browser->GetIdentifier())
	{
		// set a flag to indicate that the window close should be allowed
		base::AutoLock lock_scope(m_lock);
		m_bIsClosing = true;
	}

	// allow the close. For windowed browsers this will result in the OS close event being sent
	return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	if (GetBrowserId() == browser->GetIdentifier())
	{
		{
			// free the browser pointer so that the browser can be destroyed
			base::AutoLock lock_scope(m_lock);			
			m_browser = NULL;
		}
	}
	else if (browser->IsPopup())
	{
		// remove from the browser popup list
		for (std::list<CefRefPtr<CefBrowser> >::iterator bit = m_popupBrowsers.begin(); bit != m_popupBrowsers.end(); ++bit)
		{
			if ((*bit)->IsSame(browser))
			{
				m_popupBrowsers.erase(bit);
				break;
			}
		}
	}

	m_nBrowserCount--;

#ifdef OS_WIN
    // if all browser windows have been closed, quit the application message loop
	if (m_nBrowserCount == 0)
		App::QuitMessageLoop();
#endif
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
	CEF_REQUIRE_UI_THREAD();

	// don't display an error for downloaded files
	if (errorCode == ERR_ABORTED)
		return;

  	StringStream ssMsg;
	ssMsg << TEXT("Failed to load URL ") << String(failedUrl) << TEXT(" with error ") << String(errorText) << TEXT(" (") << errorCode << TEXT(")");
	App::Log(ssMsg.str());
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
	CEF_REQUIRE_IO_THREAD();

	String url = request->GetURL();

    // construct the path to the resource
    String appURL(Zephyros::GetAppURL());
    size_t startPos = url.find(appURL);
    if (url != appURL && startPos != String::npos)
        url.replace(startPos, appURL.length(), TEXT(""));
    else
    {
       // remove the "http://" added artificially when the app URL is constructed
       url = url.substr(7);
    }

    // find the MIME type
    size_t pos = url.rfind(TEXT('.'));
    String extension = pos == String::npos ? TEXT("") : url.substr(pos);
    std::map<String, String>::iterator it = m_mimeTypes.find(extension);
    String mimeType = it == m_mimeTypes.end() ? TEXT("application/octet-stream") : it->second;

    // load the data
    CefRefPtr<CefStreamReader> stream = GetBinaryResourceReader(url.c_str());
    if (stream.get())
        return new CefStreamResourceHandler(mimeType, stream);

	return NULL;
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
	CEF_REQUIRE_UI_THREAD();

	// load the startup URL if that's not the website that we terminated on
	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	String url = frame->GetURL();
	std::transform(url.begin(), url.end(), url.begin(), tolower);

	String startupURL(Zephyros::GetAppURL());
	if (startupURL != TEXT("chrome://crash") && !url.empty() && url.find(startupURL) != 0)
		frame->LoadURL(startupURL);
}

void ClientHandler::SetMainHwnd(ClientWindowHandle handle)
{
	if (!CefCurrentlyOn(TID_UI))
	{
		// execute on the UI thread
		CefPostTask(TID_UI, base::Bind(&ClientHandler::SetMainHwnd, this, handle));
		return;
	}

	m_mainHwnd = handle;
}

ClientWindowHandle ClientHandler::GetMainHwnd() const
{
	CEF_REQUIRE_UI_THREAD();
	return m_mainHwnd;
}

CefRefPtr<ClientExtensionHandler> ClientHandler::GetClientExtensionHandler()
{
    return m_clientExtensionHandler;
}

CefRefPtr<CefBrowser> ClientHandler::GetBrowser() const
{
	base::AutoLock lock_scope(m_lock);
	return m_browser;
}

int ClientHandler::GetBrowserId() const
{
	base::AutoLock lock_scope(m_lock);
	return m_nBrowserId;
}

void ClientHandler::CloseAllBrowsers(bool force_close)
{
	if (!CefCurrentlyOn(TID_UI))
	{
		// execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&ClientHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	// request that any popup browsers close
	if (!m_popupBrowsers.empty())
	{
		for (CefRefPtr<CefBrowser> popup : m_popupBrowsers)
			popup->GetHost()->CloseBrowser(force_close);
	}

	// request that the main browser close
	if (m_browser.get())    
		m_browser->GetHost()->CloseBrowser(force_close);
}

bool ClientHandler::IsClosing() const
{
	base::AutoLock lock_scope(m_lock);
	return m_bIsClosing;
}

void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser, const CefPoint& inspect_element_at)
{
	CefWindowInfo windowInfo;
	CefBrowserSettings settings;

#if defined(OS_WIN)
	windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

	browser->GetHost()->ShowDevTools(windowInfo, this, settings, inspect_element_at);
}

void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser)
{
	browser->GetHost()->CloseDevTools();
}

} // namespace Zephyros