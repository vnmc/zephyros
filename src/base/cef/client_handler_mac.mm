// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "lib/cef/include/cef_browser.h"
#import "lib/cef/include/cef_frame.h"
#import "lib/cef/include/cef_helpers.h"

#import "base/app.h"
#import "base/cef/client_handler.h"
//#import "base/cef/ZPYCEFAppDelegate.h"


extern bool g_isWindowLoaded;
extern bool g_isWindowBeingLoaded;
//extern ZPYCEFAppDelegate* g_appDelegate;


namespace Zephyros {

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    REQUIRE_UI_THREAD();
    
    if (m_nBrowserId == browser->GetIdentifier() && frame->IsMain())
    {
        // We've just finished loading a page

        // show the window if it isn't visible yet
        if (![m_mainHwnd.window isVisible])
        {
            //[m_mainHwnd.window makeKeyAndOrderFront: nil];
            [m_mainHwnd.window orderFront: m_mainHwnd.window];
            //[g_appDelegate performSelectorOnMainThread: @selector(showWindow) withObject: nil waitUntilDone: YES];

        }
        
        g_isWindowLoaded = true;
        g_isWindowBeingLoaded = false;
    }
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
	return false;
}

} // namespace Zephyros