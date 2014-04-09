// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CEFCLIENT_H_
#define CEF_TESTS_CEFCLIENT_CEFCLIENT_H_
#pragma once

#include <string>
#include "types.h"


#ifndef USE_WEBVIEW
class CefApp;
class CefBrowser;
class CefCommandLine;
#endif


namespace App {

enum AlertStyle
{
	AlertInfo,
	AlertWarning,
	AlertError
};


#ifndef USE_WEBVIEW
    
//
// Returns the main browser window instance.
//
CefRefPtr<CefBrowser> GetBrowser();

//
// Returns the main application window handle.
//
CefWindowHandle GetMainHwnd();

//
// Initialize the application command line.
//
void InitCommandLine(int argc, const char* const* argv);

//
// Returns the application command line object.
//
CefRefPtr<CefCommandLine> GetCommandLine();

//
// Returns the application settings based on command line arguments.
//
void GetSettings(CefSettings& settings);
    
#endif


//
// Quit the application message loop.
//
void QuitMessageLoop();

//
// Show an alert box.
//
void Alert(String title, String msg, AlertStyle style);

//
// Set the app to a "waiting" state (e.g., by displaying an hourglass cursor).
//
void BeginWait();

//
// Restores the default state.
//
void EndWait();

//
// Removes the demo-related menu items from the help menu.
//
void RemoveDemoMenuItems(WindowHandle wnd);

//
// Log.
//
void Log(String msg);

//
// Displays an error message.
//
String ShowErrorMessage();
    
void SetMenuItemStatuses(JavaScript::Object items);

} // namespace App


#endif  // CEF_TESTS_CEFCLIENT_CEFCLIENT_H_
