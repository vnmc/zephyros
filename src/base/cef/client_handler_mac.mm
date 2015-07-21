// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"

#include "base/app.h"
#include "base/cef/client_handler.h"


extern bool g_isWindowLoaded;
extern bool g_isWindowBeingLoaded;


namespace Zephyros {

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    REQUIRE_UI_THREAD();
    
    if (m_browserId == browser->GetIdentifier() && frame->IsMain())
    {
        // We've just finished loading a page

        // show the window if it isn't visible yet
        if (![m_mainHwnd.window isVisible])
            [m_mainHwnd.window makeKeyAndOrderFront: nil];
        
        g_isWindowLoaded = true;
        g_isWindowBeingLoaded = false;
    }
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
	return false;
}

} // namespace Zephyros