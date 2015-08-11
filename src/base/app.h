// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef APP_H_
#define APP_H_
#pragma once

#include <string>

#include "base/types.h"
#include "base/licensing.h"


#ifdef USE_CEF
class CefApp;
class CefBrowser;
class CefCommandLine;
#endif


namespace Zephyros {
namespace App {

enum AlertStyle
{
	AlertInfo,
	AlertWarning,
	AlertError
};


#ifdef USE_CEF
    
/**
 * Returns the main browser window instance.
 */
CefRefPtr<CefBrowser> GetBrowser();

/**
 * Returns the main application window handle.
 */
CefWindowHandle GetMainHwnd();

/**
 * Initialize the application command line.
 */
void InitCommandLine(int argc, const char* const* argv);

/**
 * Returns the application command line object.
 */
CefRefPtr<CefCommandLine> GetCommandLine();

/**
 * Returns the application settings based on command line arguments.
 */
void GetSettings(CefSettings& settings);
    
#endif

    
/**
 * Quits the app.
 */
void Quit();

/**
 * Quit the application message loop.
 */
void QuitMessageLoop();

/**
 * Show an alert box.
 */
void Alert(String title, String msg, AlertStyle style);

/**
 * Set the app to a "waiting" state (e.g., by displaying an hourglass cursor).
 */
void BeginWait();

/**
 * Restores the default state.
 */
void EndWait();

/**
 * Localize a string.
 */
String LocalizeString(String str);

/**
 * Log.
 */
void Log(String msg);

/**
 * Displays an error message.
 */
String ShowErrorMessage();
    
/**
 * Sets the statuses (checked, enabled, ...) of app menu items.
 */
void SetMenuItemStatuses(JavaScript::Object items);
    
/**
 * Returns a reference to the license manager or NULL if there is none.
 */
LicenseManager* GetLicenseManager();

} // namespace App
} // namespace Zephyros


#endif  // APP_H_
