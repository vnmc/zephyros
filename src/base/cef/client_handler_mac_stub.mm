// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"

#include "base/cef/client_handler.h"


namespace Zephyros {

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
	return false;
}

} // namespace Zephyros