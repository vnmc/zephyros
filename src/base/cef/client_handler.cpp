/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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
#include "base/cef/mime_types.h"
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
  : m_clientExtensionHandler(new ClientExtensionHandler()),
    m_nBrowserId(0),
    m_bIsClosing(false),
    m_mainHwnd(NULL)
{
#ifdef OS_WIN
    m_hAccelTable = NULL;
#endif

    m_processMessageDelegates.insert(static_cast< CefRefPtr<ProcessMessageDelegate> >(m_clientExtensionHandler));
    Zephyros::GetNativeExtensions()->AddNativeExtensions(m_clientExtensionHandler.get());
    Zephyros::GetNativeExtensions()->SetNativeExtensionsAdded();
}

ClientHandler::~ClientHandler()
{
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
	if (url.substr(0, 7) != TEXT("http://"))
		return NULL;

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

    // load the data
    CefRefPtr<CefStreamReader> stream = GetBinaryResourceReader(url.c_str());
    if (stream.get())
        return new CefStreamResourceHandler(GetMIMETypeForFilename(url), stream);

    return NULL;
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    CEF_REQUIRE_UI_THREAD();

    // load the startup URL if that's not the website that we terminated on
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    String url = ToLower(frame->GetURL());

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

#ifdef OS_WIN
    windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

    browser->GetHost()->ShowDevTools(windowInfo, this, settings, inspect_element_at);
}

void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser)
{
    browser->GetHost()->CloseDevTools();
}

} // namespace Zephyros
