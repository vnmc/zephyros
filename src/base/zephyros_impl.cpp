//
//  zephyros.cpp
//  Zephyros
//
//  Created by Matthias Christen on 13.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#include "base/zephyros_impl.h"

#ifdef USE_CEF
#include "base/cef/zephyros_cef.h"
#endif


namespace Zephyros {

TCHAR* g_szAppName;
TCHAR* g_szAppVersion;
TCHAR* g_szRegistryKey;
TCHAR* g_szUpdaterURL;
TCHAR* g_szLicenseInfoFilename;

LicenseManager* g_pLicenseManager;
NativeExtensions* g_pNativeExtensions;
    
    
    
void Init(int argc, const char* argv[], const TCHAR* appName, const TCHAR* appVersion)
{
    size_t lenAppName = _tcslen(appName);
    g_szAppName = new TCHAR[lenAppName + 1];
    g_szAppVersion = new TCHAR[_tcslen(appVersion) + 1];
    
    _tcscpy(g_szAppName, appName);
    _tcscpy(g_szAppVersion, appVersion);
    
    
    // initialize with default values
    
    // - set the registry key to "Software\<AppName>"
    g_szRegistryKey = new TCHAR[9 + lenAppName + 1];
    _tcscpy(g_szRegistryKey, TEXT("Software\\"));
    _tcscat(g_szRegistryKey, g_szAppName);
    
    // - set the updater URL to the empty string
    g_szUpdaterURL = new TCHAR[1];
    g_szUpdaterURL[0] = TEXT('\0');
    
    // - set the name of the file containing the license information to "lic.data"
    g_szLicenseInfoFilename = new TCHAR[9];
    _tcscpy(g_szLicenseInfoFilename, TEXT("lic.data"));
    
    g_pLicenseManager = NULL;
    g_pNativeExtensions = new DefaultNativeExtensions();
    
    
#ifdef USE_CEF
    // initialize and start CEF
    InitCEFApplication(argc, argv);
#endif
}
    
void Shutdown()
{
    delete[] g_szAppName;
    delete[] g_szAppVersion;
    delete[] g_szRegistryKey;
    delete[] g_szUpdaterURL;
    delete[] g_szLicenseInfoFilename;
}
    
const TCHAR* GetAppName()
{
    return g_szAppName;
}
    
const TCHAR* GetAppVersion()
{
    return g_szAppVersion;
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
    
const TCHAR* GetLicenseInfoFilename()
{
    return g_szLicenseInfoFilename;
}
    
void SetLicenseInfoFilename(const TCHAR* licenseInfoFilename)
{
    if (g_szLicenseInfoFilename)
        delete[] g_szLicenseInfoFilename;
    g_szLicenseInfoFilename = new TCHAR[_tcslen(licenseInfoFilename) + 1];
    _tcscpy(g_szLicenseInfoFilename, licenseInfoFilename);
}

LicenseManager* GetLicenseManager()
{
    return g_pLicenseManager;
}
    
void SetLicenseManager(LicenseManager* pLicenseManager)
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

} // namespace Zephyros
