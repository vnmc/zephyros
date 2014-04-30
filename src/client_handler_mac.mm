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

#import <Cocoa/Cocoa.h>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "app.h"
#include "client_handler.h"

extern bool g_isWindowLoaded;
extern bool g_isWindowBeingLoaded;


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
