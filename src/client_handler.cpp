// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

//
// Copyright (C) 2013-2014 Vanamco AG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "lib\Libcef\Include/cef_browser.h"
#include "lib\Libcef\Include/cef_frame.h"
#include "lib\Libcef\Include/cef_path_util.h"
#include "lib\Libcef\Include/cef_process_util.h"
#include "lib\Libcef\Include/cef_runnable.h"
#include "lib\Libcef\Include/cef_trace.h"
#include "lib\Libcef\Include/cef_url.h"
#include "lib\Libcef\Include/wrapper/cef_stream_resource_handler.h"

#include "app.h"
#include "client_handler.h"
#include "extension_handler.h"
#include "resource_util.h"
#include "string_util.h"
#include "file_util.h"


int ClientHandler::m_browserCount = 0;


ClientHandler::ClientHandler()
  : m_mainHwnd(NULL),
    m_browserId(0),
    m_isClosing(false),
    m_clientExtensionHandler(new ClientExtensionHandler)
{
    m_processMessageDelegates.insert(static_cast< CefRefPtr<ProcessMessageDelegate> >(m_clientExtensionHandler));
    AddNativeExtensions(m_clientExtensionHandler.get());

    m_startupURL = TEXT("http://index.html");
}

ClientHandler::~ClientHandler()
{
}

CefRefPtr<ClientExtensionHandler> ClientHandler::GetClientExtensionHandler()
{
    return m_clientExtensionHandler;
}

void ClientHandler::ReleaseCefObjects()
{
    for (CefRefPtr<ProcessMessageDelegate> delegate : m_processMessageDelegates)
        delegate->ReleaseCefObjects();
}

bool ClientHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    // execute delegate callbacks
    for (CefRefPtr<ProcessMessageDelegate> delegate : m_processMessageDelegates)
        if (delegate->OnProcessMessageReceived(this, browser, source_process, message))
            return true;
    
    return false;
}

void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    REQUIRE_UI_THREAD();
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
    REQUIRE_UI_THREAD();
    App::Log(String(message));
    
    return false;
}

void ClientHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    // remove all popup menu items: don't show the native context menu
    int numItems = model->GetCount();
    for (int i = 0; i < numItems; ++i)
        model->RemoveAt(0);
}

bool ClientHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
    return true;
}

bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, DragOperationsMask mask)
{
    return true;
}

bool ClientHandler::OnBeforePopup(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    const CefString& target_url, const CefString& target_frame_name,
    const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings, bool* no_javascript_access)
{
    // EXAMPLE
    // Cancel popups in off-screen rendering mode.
    
    if (browser->GetHost()->IsWindowRenderingDisabled())
        return true;
    return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    AutoLock lock_scope(this);
    if (!m_browser.get())
    {
        // We need to keep the main child window, but not popup windows
        m_browser = browser;
        m_browserId = browser->GetIdentifier();
    }
    else if (browser->IsPopup())
    {
        // Add to the list of popup browsers.
        m_popupBrowsers.push_back(browser);
    }

    m_browserCount++;
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this process.
    if (m_browserId == browser->GetIdentifier())
    {
        // Notify the browser that the parent window is about to close.
        browser->GetHost()->ParentWindowWillClose();

        // Set a flag to indicate that the window close should be allowed.
        m_isClosing = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close event being sent.
    return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    if (m_browserId == browser->GetIdentifier())
    {
        // Free the browser pointer so that the browser can be destroyed
        m_browser = NULL;
    }
    else if (browser->IsPopup())
    {
        // Remove from the browser popup list.
        for (std::list< CefRefPtr<CefBrowser> >::iterator bit = m_popupBrowsers.begin(); bit != m_popupBrowsers.end(); ++bit)
        {
            if ((*bit)->IsSame(browser))
            {
                m_popupBrowsers.erase(bit);
                break;
            }
        }
    }

	m_browserCount--;

#ifdef OS_WIN
    // if all browser windows have been closed, quit the application message loop
	if (m_browserCount == 0)
		App::QuitMessageLoop();
#endif
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    REQUIRE_UI_THREAD();

    if (m_browserId == browser->GetIdentifier() && frame->IsMain())
    {
        // We've just started loading a page
    }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;
 
	StringStream ssMsg;
	ssMsg << TEXT("Failed to load URL ") << String(failedUrl) << TEXT(" with error ") << String(errorText) << TEXT(" (") << errorCode << TEXT(")");
	App::Log(ssMsg.str());
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    // Load the startup URL if that's not the website that we terminated on.
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    String url = frame->GetURL();
    std::transform(url.begin(), url.end(), url.begin(), tolower);

    String startupURL = GetStartupURL();
    if (url.find(startupURL) != 0)
        frame->LoadURL(startupURL);
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
    String url = request->GetURL();
    if (url == TEXT("http://index.html/"))
        url = TEXT("index.html");
    else
    {
        String from = TEXT("http://index.html/");
        size_t startPos = url.find(from);
        if (startPos == String::npos)
            return NULL;
        
        url.replace(startPos, from.length(), TEXT(""));
    }

	String mimeType;
    if (url.find(TEXT(".html")) != String::npos)
        mimeType = TEXT("text/html");
    else if (url.find(TEXT(".css")) != String::npos)
        mimeType = TEXT("text/css");
    else if (url.find(TEXT(".js")) != String::npos)
        mimeType = TEXT("text/javascript");
    else if (url.find(TEXT(".png")) != String::npos)
        mimeType = TEXT("image/png");
    else if (url.find(TEXT(".jpg")) != String::npos || url.find(TEXT(".jpeg")) != String::npos)
        mimeType = TEXT("image/jpeg");
    else if (url.find(TEXT(".woff")) != String::npos)
        mimeType = TEXT("application/font-woff");
    else if (url.find(TEXT(".ttf")) != String::npos)
        mimeType = TEXT("application/x-font-ttf");
	else if (url.find(TEXT(".svg")) != String::npos)
		mimeType = TEXT("image/svg+xml");
    
    CefRefPtr<CefStreamReader> stream = GetBinaryResourceReader(url.c_str());
    if (stream.get())
        return new CefStreamResourceHandler(mimeType, stream);
        
    return NULL;
}

void ClientHandler::SetMainHwnd(CefWindowHandle hwnd)
{
    AutoLock lock_scope(this);
    m_mainHwnd = hwnd;
}

void ClientHandler::CloseAllBrowsers(bool forceClose)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, NewCefRunnableMethod(this, &ClientHandler::CloseAllBrowsers, forceClose));
        return;
    }

    // Request that any popup browsers close.
    if (!m_popupBrowsers.empty())
    {
        for (CefRefPtr<CefBrowser> browser : m_popupBrowsers)
            browser->GetHost()->CloseBrowser(forceClose);
    }

    // Request that the main browser close.
    if (m_browser.get())
        m_browser->GetHost()->CloseBrowser(forceClose);
}
