// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

#include "include/cef_client.h"

#include "types.h"
#include "util.h"


class ClientExtensionHandler;


// ClientHandler implementation.
class ClientHandler : public CefClient, public CefDisplayHandler, public CefKeyboardHandler, public CefDragHandler, public CefLifeSpanHandler, public CefLoadHandler, public CefContextMenuHandler, public CefRequestHandler
{
public:
                          
    // Interface for process message delegates. Do not perform work in the
    // RenderDelegate constructor.
    class ProcessMessageDelegate : public virtual CefBase
    {
    public:
        // Called when a process message is received. Return true if the message was
        // handled and should not be passed on to other handlers.
        // ProcessMessageDelegates should check for unique message names to avoid
        // interfering with each other.
        virtual bool OnProcessMessageReceived(
            CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
        {
            return false;
        }
        
        virtual void ReleaseCefObjects()
        {
        }
    };

    
    typedef std::set<CefRefPtr<ProcessMessageDelegate> > ProcessMessageDelegateSet;

    
    ClientHandler();
    virtual ~ClientHandler();

    void ReleaseCefObjects();

    
    // CefClient methods
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE
    {
        return this;
    }

	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE
	{
		return this;
	}
    
    virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE
    {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
    {
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE
    {
        return this;
    }

    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE
    {
        return this;
    }

    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE
    {
        return this;
    }

    
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

    // CefDisplayHandler methods
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE;

	// CefKeyboardHandler methods
	virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event) OVERRIDE;

    // CefDragHandler methods
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, DragOperationsMask mask) OVERRIDE;

    // CefLifeSpanHandler methods
    virtual bool OnBeforePopup(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        const CefString& target_url, const CefString& target_frame_name,
        const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings, bool* no_javascript_access) OVERRIDE;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

    // CefLoadHandler methods
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) OVERRIDE;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) OVERRIDE;
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;

    // CefContextMenuHandler methods
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) OVERRIDE;
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) OVERRIDE;

    // CefRequestHandler methods
    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request) OVERRIDE;

    
    void SetMainHwnd(CefWindowHandle hwnd);

    CefWindowHandle GetMainHwnd()
    {
        return m_mainHwnd;
    }

#if OS_WIN
	void SetAccelTable(HACCEL hAccelTable)
	{
		m_hAccelTable = hAccelTable;
	}
#endif

    CefRefPtr<CefBrowser> GetBrowser()
    {
        return m_browser;
    }
    
    int GetBrowserId()
    {
        return m_browserId;
    }
    
    CefRefPtr<ClientExtensionHandler> GetClientExtensionHandler();

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    // Returns true if the main browser window is currently closing. Used in
    // combination with DoClose() and the OS close notification to properly handle
    // 'onbeforeunload' JavaScript events during window close.
    bool IsClosing()
    {
        return m_isClosing;
    }
    
    // Returns the startup URL.
    String GetStartupURL()
    {
        return m_startupURL;
    }

    // Create an external browser window that loads the specified URL.
    static void LaunchExternalBrowser(const String& url);


protected:    
    // The child browser window
    CefRefPtr<CefBrowser> m_browser;

    // List of any popup browser windows. Only accessed on the CEF UI thread.
    std::list< CefRefPtr<CefBrowser> > m_popupBrowsers;

    // The main frame window handle
    CefWindowHandle m_mainHwnd;

#if OS_WIN
	HACCEL m_hAccelTable;
#endif

    // The child browser id
    int m_browserId;

    // True if the main browser window is currently closing.
    bool m_isClosing;

    // Registered delegates.
    ProcessMessageDelegateSet m_processMessageDelegates;

    // If true DevTools will be opened in an external browser window.
    bool m_useExternalDevTools;

    // The startup URL.
    String m_startupURL;

    // Number of currently existing browser windows. The application will exit
    // when the number of windows reaches 0.
    static int m_browserCount;
    
    CefRefPtr<ClientExtensionHandler> m_clientExtensionHandler;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(ClientHandler);
    
    // Include the default locking implementation.
    IMPLEMENT_LOCKING(ClientHandler);
};

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
