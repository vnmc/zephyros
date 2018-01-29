/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
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


#include <map>

#ifdef OS_WIN
#include <tchar.h>
#endif

#ifdef OS_LINUX
#include <X11/Xlib.h>
#endif

#include "base/types.h"
/// TODO: XXX remove
#include "logging.h"
///
#include "native_extensions/path.h"
#include "util/string_util.h"

#ifdef USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#include "base/cef/mime_types.h"
#endif

#include "zephyros.h"
#include "zephyros_strings.h"
#include "native_extensions.h"


#define SET_DEFAULT(stringId, str) if (g_mapStrings.find(stringId) == g_mapStrings.end()) Zephyros::SetString(stringId, str)


namespace Zephyros {

TCHAR* g_szCompanyName = NULL;
TCHAR* g_szAppName = NULL;
TCHAR* g_szAppVersion = NULL;
TCHAR* g_szAppURL = NULL;
TCHAR* g_szUpdaterURL = NULL;
TCHAR* g_szCrashReportingURL = NULL;
TCHAR* g_szCrashReportingPrivacyPolicyURL = NULL;
Size g_defaultWindowSize = { 800, 600 };
WindowsInfo g_windowsInfo = { 0 };
OSXInfo g_osxInfo = { 0 };

AbstractLicenseManager* g_pLicenseManager = NULL;
NativeExtensions* g_pNativeExtensions = NULL;

std::map<int, String> g_mapMenuCommands;
std::map<String, int> g_mapMenuIDs;
std::map<int, String> g_mapStrings;

#ifdef OS_WIN
std::map<String, int> g_mapResourceIDs;
#endif

#ifdef OS_LINUX
typedef struct {
    char* pData;
    int nLength;
} Resource;
std::map<String, Resource> g_mapResources;
#endif

bool g_bUseLogging = false;


void InitDefaultStrings()
{
    SET_DEFAULT(ZS_DEMODLG_WNDTITLE, TEXT("{{appName}} Demo"));
    SET_DEFAULT(ZS_DEMODLG_TITLE, TEXT("Thank you for trying {{appName}}."));
    SET_DEFAULT(ZS_DEMODLG_DESCRIPTION, TEXT("You are using {{appName}} in demo mode.\nThe software will stop functioning when the end of the trial period is reached."));
    SET_DEFAULT(ZS_DEMODLG_START_DEMO, TEXT("Start Demo"));
    SET_DEFAULT(ZS_DEMODLG_CONTINUE_DEMO, TEXT("Continue Demo"));
    SET_DEFAULT(ZS_DEMODLG_CLOSE, TEXT("Close"));
    SET_DEFAULT(ZS_DEMODLG_DAYS_LEFT, TEXT("days left."));
    SET_DEFAULT(ZS_DEMODLG_1DAY_LEFT, TEXT("1 day left."));
    SET_DEFAULT(ZS_DEMODLG_NO_DAYS_LEFT, TEXT("No days left. Please purchase a license."));
    SET_DEFAULT(ZS_DEMODLG_REMAINING_TIME, TEXT("Remaining Time"));
    SET_DEFAULT(ZS_DEMODLG_REMAINING_TIME_DESC, TEXT("Only the days of actual usage are counted."));
    SET_DEFAULT(ZS_DEMODLG_PURCHASE_LICENSE, TEXT("Purchase License"));
    SET_DEFAULT(ZS_DEMODLG_ENTER_LICKEY, TEXT("Enter License Key"));

    SET_DEFAULT(ZS_ENTERLICKEYDLG_WNDTITLE, TEXT("Enter License Key"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_TITLE, TEXT("Please enter your license information below:"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_FULL_NAME, TEXT("Full Name:"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_ORGANIZATION, TEXT("Organization:"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_LICKEY, TEXT("License Key:"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_CANCEL, TEXT("Cancel"));
    SET_DEFAULT(ZS_ENTERLICKEYDLG_ACTIVATE, TEXT("Activate"));

    SET_DEFAULT(ZS_PREVVERSIONDLG_WNDTITLE, TEXT("Obsolete License Detected"));
    SET_DEFAULT(ZS_PREVVERSIONDLG_TITLE, TEXT("You need to upgrade your license."));
    SET_DEFAULT(ZS_PREVVERSIONDLG_DESCRIPTION, TEXT("You have entered an obsolete license key.\nThis version requires a new license key."));
    SET_DEFAULT(ZS_PREVVERSIONDLG_BACK, TEXT("Back"));
    SET_DEFAULT(ZS_PREVVERSIONDLG_UPGRADE, TEXT("Take me to the upgrade page"));

    SET_DEFAULT(ZS_LIC_LICERROR, TEXT("{{appName}} Licensing Error"));
    SET_DEFAULT(ZS_LIC_CONNECTION_FAILED, TEXT("Failed to contact the licensing server.\n\nPlease try again later."));
    SET_DEFAULT(ZS_LIC_PARSE_ERROR, TEXT("Failed to parse response from the licensing server.\n\nPlease try again later."));
    SET_DEFAULT(ZS_LIC_UNEXPECTED_RESPONSE, TEXT("Unexpected response from the licensing server:\n"));

    SET_DEFAULT(ZS_LIC_ACTIVATION, TEXT("{{appName}} Activation"));
    SET_DEFAULT(ZS_LIC_LICINVALID, TEXT("This license key is not valid. Please try again."));
    SET_DEFAULT(ZS_LIC_ACTIVATION_ERROR, TEXT("Unfortunately, an error occurred during the activation of {{appName}}:\n\n"));
    SET_DEFAULT(ZS_LIC_ACTIVATION_SUCCESS, TEXT("Thank you for activating {{appName}}."));

    SET_DEFAULT(ZS_LIC_DEACTIVATION, TEXT("{{appName}} Deactivation"));
    SET_DEFAULT(ZS_LIC_DEACTIVATION_ERROR, TEXT("Unfortunately, an error occurred during deactivating {{appName}}:\n\n"));
    SET_DEFAULT(ZS_LIC_DEACTIVATION_SUCCESS, TEXT("You have successfully deactivated {{appName}}.\nThe application will quit now."));

    SET_DEFAULT(ZS_LIC_DEMO_ERROR, TEXT("Unfortunately, an error while retrieving a demo license from the licensing server:\n"));

    SET_DEFAULT(ZS_UPDATES_CHECK, TEXT("Check for Updates"));

    SET_DEFAULT(ZS_OPEN_FINDER, TEXT("Open in Finder"));
    SET_DEFAULT(ZS_OPEN_EXPLORER, TEXT("Open in Explorer"));
    SET_DEFAULT(ZS_OPEN_FILEMANAGER, TEXT("Open in file manager"));
    SET_DEFAULT(ZS_OPEN_PATH_DOESNT_EXIST, TEXT("The path %s does not exist."));

    SET_DEFAULT(ZS_DIALOG_OPEN_FILE, TEXT("Select file"));
    SET_DEFAULT(ZS_DIALOG_OPEN_FOLDER, TEXT("Select folder"));
    SET_DEFAULT(ZS_DIALOG_SAVE_FILE, TEXT("Save file"));
    SET_DEFAULT(ZS_DIALOG_FILE_OPEN_BUTTON, TEXT("_Open"));
    SET_DEFAULT(ZS_DIALOG_FILE_CANCEL_BUTTON, TEXT("_Cancel"));
    SET_DEFAULT(ZS_DIALOG_FILE_SAVE_BUTTON, TEXT("_Save"));


}

int Run(MAIN_ARGS, void (*fnxSetResources)(), const TCHAR* szAppName, const TCHAR* szAppVersion, const TCHAR* szAppURL)
{
#ifdef OS_LINUX
    XInitThreads();
#endif

    // set the resources if a function was provided
    if (fnxSetResources)
        fnxSetResources();
    
    if (GetLicenseManager())
        GetLicenseManager()->LoadLicenseData();

    size_t lenAppName = _tcslen(szAppName);
    g_szAppName = new TCHAR[lenAppName + 1];
    g_szAppVersion = new TCHAR[_tcslen(szAppVersion) + 1];
    g_szAppURL = new TCHAR[_tcslen(szAppURL) + 8];

    _tcscpy(g_szAppName, szAppName);
    _tcscpy(g_szAppVersion, szAppVersion);

#ifdef USE_WEBVIEW
    _tcscpy(g_szAppURL, szAppURL);
#endif
#ifdef USE_CEF
    _tcscpy(g_szAppURL, TEXT("http://"));
    _tcscat(g_szAppURL, szAppURL);
#endif

    // initialize with default values
    InitDefaultStrings();
    
#ifdef USE_CEF
    InitializeMIMETypes();
#endif

#ifdef OS_WIN
    // - set the registry key to "Software\<AppName>"
    if (g_windowsInfo.szRegistryKey == NULL)
    {
        g_windowsInfo.szRegistryKey = new TCHAR[9 + lenAppName + 1];
        _tcscpy(g_windowsInfo.szRegistryKey, TEXT("Software\\"));
        _tcscat(g_windowsInfo.szRegistryKey, g_szAppName);
    }
#endif

    if (g_pNativeExtensions == NULL)
        g_pNativeExtensions = new DefaultNativeExtensions();

    // initialize and start the application
    return RunApplication(INIT_APPLICATION_ARGS);
}

void Shutdown()
{
    if (g_szCompanyName != NULL)
        delete[] g_szCompanyName;

    if (g_szAppName != NULL)
        delete[] g_szAppName;

    if (g_szAppVersion != NULL)
        delete[] g_szAppVersion;

    if (g_szAppURL != NULL)
        delete[] g_szAppURL;

    if (g_windowsInfo.szRegistryKey != NULL)
        delete[] g_windowsInfo.szRegistryKey;

    if (g_osxInfo.szMainNibName != NULL)
        delete[] g_osxInfo.szMainNibName;

    if (g_szUpdaterURL != NULL)
        delete[] g_szUpdaterURL;

    if (g_szCrashReportingURL != NULL)
        delete[] g_szCrashReportingURL;

    if (g_szCrashReportingPrivacyPolicyURL != NULL)
        delete[] g_szCrashReportingPrivacyPolicyURL;

    if (g_pLicenseManager != NULL)
        delete g_pLicenseManager;

    if (g_pNativeExtensions != NULL)
        delete g_pNativeExtensions;

    g_szCompanyName = NULL;
    g_szAppName = NULL;
    g_szAppVersion = NULL;
    g_szAppURL = NULL;
    g_windowsInfo.szRegistryKey = NULL;
    g_osxInfo.szMainNibName = NULL;
    g_szUpdaterURL = NULL;
    g_szCrashReportingURL = NULL;
    g_szCrashReportingPrivacyPolicyURL = NULL;
    g_pLicenseManager = NULL;
    g_pNativeExtensions = NULL;
}

const TCHAR* GetAppName()
{
    return g_szAppName;
}

const TCHAR* GetAppVersion()
{
    return g_szAppVersion;
}

const TCHAR* GetAppURL()
{
    return g_szAppURL;
}

const TCHAR* GetCompanyName()
{
    return g_szCompanyName;
}

void SetCompanyName(const TCHAR* szCompanyName)
{
    if (g_szCompanyName)
        delete[] g_szCompanyName;
    g_szCompanyName = new TCHAR[_tcslen(szCompanyName) + 1];
    _tcscpy(g_szCompanyName, szCompanyName);
}

int GetMenuIDForCommand(const TCHAR* szCommand)
{
    std::map<String, int>::iterator it = g_mapMenuIDs.find(szCommand);
    return it == g_mapMenuIDs.end() ? 0 : it->second;
}

const TCHAR* GetMenuCommandForID(int nMenuID)
{
    std::map<int, String>::iterator it = g_mapMenuCommands.find(nMenuID);
    return it == g_mapMenuCommands.end() ? NULL : it->second.c_str();
}

void SetMenuIDForCommand(const TCHAR* szCommand, int nMenuID)
{
    g_mapMenuCommands[nMenuID] = szCommand;
    g_mapMenuIDs[szCommand] = nMenuID;
}

Size GetDefaultWindowSize()
{
    return g_defaultWindowSize;
}

void SetDefaultWindowSize(int nWidth, int nHeight)
{
    g_defaultWindowSize.nWidth = nWidth;
    g_defaultWindowSize.nHeight = nHeight;
}

WindowsInfo GetWindowsInfo()
{
    return g_windowsInfo;
}

void SetWindowsInfo(const TCHAR* szRegistryKey, int nIconID, int nMenuID, int nAccelID)
{
    if (g_windowsInfo.szRegistryKey)
        delete g_windowsInfo.szRegistryKey;
    g_windowsInfo.szRegistryKey = new TCHAR[_tcslen(szRegistryKey) + 1];
    _tcscpy(g_windowsInfo.szRegistryKey, szRegistryKey);

    g_windowsInfo.nIconID = nIconID;
    g_windowsInfo.nMenuID = nMenuID;
    g_windowsInfo.nAccelID = nAccelID;
}

OSXInfo GetOSXInfo()
{
    return g_osxInfo;
}

void SetOSXInfo(const TCHAR* szMainNibName)
{
    if (g_osxInfo.szMainNibName)
        delete g_osxInfo.szMainNibName;
    g_osxInfo.szMainNibName = new TCHAR[_tcslen(szMainNibName) + 1];
    _tcscpy(g_osxInfo.szMainNibName, szMainNibName);
}

const TCHAR* GetCrashReportingURL()
{
    return g_szCrashReportingURL;
}

const TCHAR* GetCrashReportingPrivacyPolicyURL()
{
    return g_szCrashReportingPrivacyPolicyURL;
}

void SetCrashReportingURL(const TCHAR* szReportingURL, const TCHAR* szPrivacyPolicyURL)
{
    if (g_szCrashReportingURL)
        delete[] g_szCrashReportingURL;
    if (g_szCrashReportingPrivacyPolicyURL)
        delete[] g_szCrashReportingPrivacyPolicyURL;

    g_szCrashReportingURL = new TCHAR[_tcslen(szReportingURL) + 1];
    _tcscpy(g_szCrashReportingURL, szReportingURL);

    if (szPrivacyPolicyURL)
    {
        g_szCrashReportingPrivacyPolicyURL = new TCHAR[_tcslen(szPrivacyPolicyURL) + 1];
        _tcscpy(g_szCrashReportingPrivacyPolicyURL, szPrivacyPolicyURL);
    }
    else
        g_szCrashReportingPrivacyPolicyURL = NULL;
}

const TCHAR* GetUpdaterURL()
{
    return g_szUpdaterURL;
}

void SetUpdaterURL(const TCHAR* szURL)
{
    if (g_szUpdaterURL)
        delete[] g_szUpdaterURL;
    g_szUpdaterURL = new TCHAR[_tcslen(szURL) + 1];
    _tcscpy(g_szUpdaterURL, szURL);
}

AbstractLicenseManager* GetLicenseManager()
{
    DEBUG_LOGC(g_pLicenseManager ? "GetLicenseManager: exists" : "GetLicenseManager: does not exist")
    return g_pLicenseManager;
}

void SetLicenseManager(AbstractLicenseManager* pLicenseManager)
{
    DEBUG_LOGC(pLicenseManager ? "SetLicenseManager: exists" : "SetLicenseManager: does not exist")
    g_pLicenseManager = pLicenseManager;
}

NativeExtensions* GetNativeExtensions()
{
    return g_pNativeExtensions;
}

void SetNativeExtensions(NativeExtensions* pNativeExtensions)
{
    g_pNativeExtensions = pNativeExtensions;
}

String GetString(int stringId)
{
    std::map<int, String>::iterator it = g_mapStrings.find(stringId);

    if (it == g_mapStrings.end())
        return TEXT("");

    if (it->second.find(TEXT("{{")) != String::npos)
    {
        return g_mapStrings[stringId] = StringReplace(StringReplace(it->second,
            TEXT("{{appName}}"), g_szAppName),
            TEXT("{{appVersion}}"), g_szAppVersion);
    }

    return it->second;
}

void SetString(int stringId, String str)
{
    g_mapStrings[stringId] = str;
}

#ifdef OS_WIN
int GetResourceID(const TCHAR* szResourceName)
{
    std::map<String, int>::iterator it = g_mapResourceIDs.find(szResourceName);
    return it == g_mapResourceIDs.end() ? -1 : it->second;
}

void SetResourceID(const TCHAR* szResourceName, int nID)
{
    g_mapResourceIDs[szResourceName] = nID;
}
#endif // OS_WIN

#ifdef OS_LINUX
bool GetResource(const TCHAR* szResourceName, char*& pData, int& nLen)
{
    std::map<String, Resource>::iterator it = g_mapResources.find(szResourceName);
    if (it == g_mapResources.end())
        return false;

    pData = it->second.pData;
    nLen = it->second.nLength;
    return true;
}

void SetResource(const TCHAR* szResourceName, char* pData, int nLen)
{
    Resource resource;
    resource.pData = pData;
    resource.nLength = nLen;

    g_mapResources[szResourceName] = resource;
}
#endif // OS_LINUX

bool UseLogging()
{
    return g_bUseLogging;
}

void UseLogging(bool bUseLogging)
{
    g_bUseLogging = bUseLogging;
}

} // namespace Zephyros
