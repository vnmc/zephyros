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


#ifndef Zephyros_App_h
#define Zephyros_App_h
#pragma once

#include <string>
#include <map>

#include "base/types.h"

#include "zephyros.h"


#ifdef USE_CEF
class CefApp;
class CefBrowser;
class CefCommandLine;
#endif


namespace Zephyros {
namespace App {


typedef std::map<String, int> MenuItemStatuses;


#ifdef USE_CEF

/**
 * Returns the main browser window instance.
 */
CefRefPtr<CefBrowser> GetBrowser();

/**
 * Returns the main application window handle.
 */
ClientWindowHandle GetMainHwnd();

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
 * Returns the user-agent string to use.
 */
String GetUserAgent();

/**
 * Initiates closing the application window.
 */
void CloseWindow();

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
void SetMenuItemStatuses(MenuItemStatuses& statuses);

/**
 * Returns a reference to the license manager or NULL if there is none.
 */
Zephyros::LicenseManager* GetLicenseManager();

/**
 * Removes all the menu items from the app menu related to the demo mode.
 */
void RemoveDemoMenuItems();

} // namespace App
} // namespace Zephyros


#endif // Zephyros_App_h
