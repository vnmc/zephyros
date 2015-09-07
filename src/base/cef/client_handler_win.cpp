// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <string>
#include <windows.h>
#include <shlobj.h> 

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"

#include "base/cef/client_handler.h"


extern UINT g_nCmdShow;


namespace Zephyros {

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    CEF_REQUIRE_UI_THREAD();
    
    if (m_nBrowserId == browser->GetIdentifier() && frame->IsMain())
    {
        // we've just finished loading a page
		if (!IsWindowVisible(m_mainHwnd))
		{
			ShowWindow(m_mainHwnd, g_nCmdShow);
			UpdateWindow(m_mainHwnd);
		}
    }
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
	if (!os_event)
		return false;
	return TranslateAccelerator(m_mainHwnd, m_hAccelTable, os_event) ? true : false;
}

} // namespace Zephyros