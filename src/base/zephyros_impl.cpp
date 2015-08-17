//
//  zephyros.cpp
//  Zephyros
//
//  Created by Matthias Christen on 13.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#include <map>

#include "base/zephyros_impl.h"
#include "base/zephyros_strings.h"

#include "util/string_util.h"

#ifdef USE_CEF
#include "base/cef/zephyros_cef.h"
#endif


#define SET_DEFAULT(stringId, str) if (g_mapStrings.find(stringId) == g_mapStrings.end()) Zephyros::SetString(stringId, str)


namespace Zephyros {

TCHAR* g_szAppName = NULL;
TCHAR* g_szAppVersion = NULL;
TCHAR* g_szAppURL = NULL;
TCHAR* g_szRegistryKey = NULL;
TCHAR* g_szUpdaterURL = NULL;

AbstractLicenseManager* g_pLicenseManager = NULL;
NativeExtensions* g_pNativeExtensions = NULL;
    
std::map<int, String> g_mapStrings;

    
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
}
    
int Init(int argc, const char* argv[], const TCHAR* appName, const TCHAR* appVersion, const TCHAR* appURL)
{
    size_t lenAppName = _tcslen(appName);
    g_szAppName = new TCHAR[lenAppName + 1];
    g_szAppVersion = new TCHAR[_tcslen(appVersion) + 1];
    g_szAppURL = new TCHAR[_tcslen(appURL) + 8];
    
    _tcscpy(g_szAppName, appName);
    _tcscpy(g_szAppVersion, appVersion);

#ifdef USE_WEBVIEW
    _tcscpy(g_szAppURL, appURL);
#endif
#ifdef USE_CEF
    _tcscpy(g_szAppURL, TEXT("http://"));
    _tcscat(g_szAppURL, appURL);
#endif
    
    
    // initialize with default values
    
    InitDefaultStrings();
    
    // - set the registry key to "Software\<AppName>"
    if (g_szRegistryKey == NULL)
    {
        g_szRegistryKey = new TCHAR[9 + lenAppName + 1];
        _tcscpy(g_szRegistryKey, TEXT("Software\\"));
        _tcscat(g_szRegistryKey, g_szAppName);
    }
    
    // - set the updater URL to the empty string
    if (g_szUpdaterURL == NULL)
    {
        g_szUpdaterURL = new TCHAR[1];
        g_szUpdaterURL[0] = TEXT('\0');
    }
    
    if (g_pNativeExtensions == NULL)
        g_pNativeExtensions = new DefaultNativeExtensions();
    
    
    // initialize and start the application
    return InitApplication(argc, argv);
}
    
void Shutdown()
{
    if (g_szAppName != NULL)
        delete[] g_szAppName;
    
    if (g_szAppVersion != NULL)
        delete[] g_szAppVersion;
    
    if (g_szAppURL != NULL)
        delete[] g_szAppURL;
    
    if (g_szRegistryKey != NULL)
        delete[] g_szRegistryKey;
    
    if (g_szUpdaterURL != NULL)
        delete[] g_szUpdaterURL;
    
    if (g_pLicenseManager != NULL)
        delete g_pLicenseManager;
    
    if (g_pNativeExtensions != NULL)
        delete g_pNativeExtensions;
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

const TCHAR* GetRegistryKey()
{
    return g_szRegistryKey;
}
    
void SetRegistryKey(const TCHAR* registryKey)
{
    if (g_szRegistryKey)
        delete g_szRegistryKey;
    g_szRegistryKey = new TCHAR[_tcslen(registryKey) + 1];
    _tcscpy(g_szRegistryKey, registryKey);
}
    
const TCHAR* GetUpdaterURL()
{
    return g_szUpdaterURL;
}
    
void SetUpdaterURL(const TCHAR* url)
{
    if (g_szUpdaterURL)
        delete[] g_szUpdaterURL;
    g_szUpdaterURL = new TCHAR[_tcslen(url) + 1];
    _tcscpy(g_szUpdaterURL, url);
}
    
AbstractLicenseManager* GetLicenseManager()
{
    return g_pLicenseManager;
}
    
void SetLicenseManager(AbstractLicenseManager* pLicenseManager)
{
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
    return it == g_mapStrings.end() ? TEXT("") : it->second;
}
    
void SetString(int stringId, String str)
{
    g_mapStrings[stringId] = StringReplace(StringReplace(str,
        TEXT("{{appName}}"), g_szAppName),
        TEXT("{{appVersion}}"), g_szAppVersion);
}

} // namespace Zephyros
