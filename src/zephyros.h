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


#ifndef Zephyros_h
#define Zephyros_h

#include <string>
#include <vector>
#include <map>


//////////////////////////////////////////////////////////////////////////
// Constants

#define MENUCOMMAND_TERMINATE         "terminate"
#define MENUCOMMAND_CHECK_UPDATE      "check_update"
#define MENUCOMMAND_ENTER_LICENSE     "enter_license"
#define MENUCOMMAND_PURCHASE_LICENSE  "purchase_license"


//////////////////////////////////////////////////////////////////////////
// Types

#ifdef OS_WIN

#include <Windows.h>
#include <tchar.h>

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

#define MAIN _tWinMain
#define MAIN_ARGS HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow
#define INIT_APPLICATION_ARGS hInstance, hPrevInstance, lpCmdLine, nCmdShow
#define RUN_APPLICATION_ARGS hInstance, hPrevInstance, lpCmdLine, nCmdShow, SetResources

#else

typedef std::string String;

#define TCHAR        char
#define TEXT(string) string
#define BYTE         unsigned char

#define _tprintf     printf
#define _tcscpy      strcpy
#define _tcscat      strcat
#define _tcslen      strlen
#define _ttoi        atoi
#define _tcsnccmp    strncmp
#define _wtoi64      atoi

#define MAIN main
#define MAIN_ARGS int argc, char* argv[]
#define INIT_APPLICATION_ARGS argc, argv

#ifdef OS_MACOSX
#define RUN_APPLICATION_ARGS argc, argv, NULL
#else
#define RUN_APPLICATION_ARGS argc, argv, SetResources
#endif

#endif  // OS_WIN


#ifdef USE_CEF

#ifndef CefWindowHandle
#define CefWindowHandle void*
#endif

#ifdef OS_LINUX
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

// TODO: is this still needed?
//typedef CefWindowHandle WindowHandle;

#endif


namespace Zephyros {

class NativeExtensions;

} // namespace Zephyros


//////////////////////////////////////////////////////////////////////////
// Zephyros Structures

namespace Zephyros {

typedef struct
{
    int nWidth;
    int nHeight;
} Size;

typedef struct
{
    TCHAR* szRegistryKey;
    int nIconID;
    int nMenuID;
    int nAccelID;
} WindowsInfo;

typedef struct
{
    TCHAR* szMainNibName;
} OSXInfo;

} // namespace Zephyros


#include "licensing.h"


namespace Zephyros {
namespace App {

enum AlertStyle
{
    AlertInfo,
    AlertWarning,
    AlertError
};


typedef std::map<String, int> MenuItemStatuses;


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



//////////////////////////////////////////////////////////////////////////
// Zephyros Functions

namespace Zephyros {

int Run(MAIN_ARGS, void (*fnxSetResources)(), const TCHAR* szAppName, const TCHAR* szAppVersion, const TCHAR* szAppURL);
int RunApplication(MAIN_ARGS);
void Shutdown();

const TCHAR* GetAppName();
const TCHAR* GetAppVersion();
const TCHAR* GetAppURL();

const TCHAR* GetCompanyName();
void SetCompanyName(const TCHAR* szCompanyName);

int GetMenuIDForCommand(const TCHAR* szCommand);
const TCHAR* GetMenuCommandForID(int nMenuID);
void SetMenuIDForCommand(const TCHAR* szCommand, int nMenuID);

Size GetDefaultWindowSize();
void SetDefaultWindowSize(int nWidth, int nHeight);

WindowsInfo GetWindowsInfo();
void SetWindowsInfo(const TCHAR* szRegistryKey, int nIconID, int nMenuID = 0, int nAccelID = 0);
OSXInfo GetOSXInfo();
void SetOSXInfo(const TCHAR* szMainNibName);

#ifndef NO_CRASHRPT
const TCHAR* GetCrashReportingURL();
const TCHAR* GetCrashReportingPrivacyPolicyURL();
void SetCrashReportingURL(const TCHAR* szReportingURL, const TCHAR* szPrivacyPolicyURL);
#endif

#ifndef NO_SPARKLE
const TCHAR* GetUpdaterURL();
void SetUpdaterURL(const TCHAR* szURL);
#endif

AbstractLicenseManager* GetLicenseManager();
void SetLicenseManager(AbstractLicenseManager* pLicenseManager);

NativeExtensions* GetNativeExtensions();
void SetNativeExtensions(NativeExtensions* pNativeExtensions);

String GetString(int stringId);
void SetString(int stringId, String str);

#ifdef OS_WIN
int GetResourceID(const TCHAR* szResourceName);
void SetResourceID(const TCHAR* szResourceName, int nID);
#endif
#ifdef OS_LINUX
bool GetResource(const TCHAR* szResourceName, char*& pData, int& nLen);
void SetResource(const TCHAR* szResourceName, char* pData, int nLen);
#endif

bool UseLogging();
void UseLogging(bool bUseLogging);

} // namespace Zephyros


#endif // Zephyros_h
