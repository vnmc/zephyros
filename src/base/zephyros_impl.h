//
//  zephyros_impl.h
//  Zephyros
//
//  Created by Matthias Christen on 12.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#ifndef Zephyros_zephyros_h
#define Zephyros_zephyros_h

#include "base/types.h"
#include "base/licensing.h"

#include "native_extensions/path.h"
#include "native_extensions/native_extensions.h"

#ifdef USE_CEF
#include "base/cef/extension_handler.h"
#endif


namespace Zephyros {
    
    
//////////////////////////////////////////////////////////////////////////
// Global Variables

extern TCHAR* g_szAppName;
extern TCHAR* g_szAppVersion;
extern TCHAR* g_szRegistryKey;
extern TCHAR* g_szUpdaterURL;
extern TCHAR* g_szLicenseInfoFilename;

extern LicenseManager* g_pLicenseManager;   
extern NativeExtensions* g_pNativeExtensions;
    

//////////////////////////////////////////////////////////////////////////
// Zephyros Functions

void Init(int argc, const char* argv[], const TCHAR* appName, const TCHAR* appVersion);
void Shutdown();
    
const TCHAR* GetAppName();
const TCHAR* GetAppVersion();
    
const TCHAR* GetRegistryKey();
void SetRegistryKey(const TCHAR* registryKey);
    
const TCHAR* GetUpdaterURL();
void SetUpdaterURL(const TCHAR* url);
    
const TCHAR* GetLicenseInfoFilename();
void SetLicenseInfoFilename(const TCHAR* licenseInfoFilename);

LicenseManager* GetLicenseManager();
void SetLicenseManager(LicenseManager* pLicenseManager);
    
NativeExtensions* GetNativeExtensions();
void SetNativeExtensions(NativeExtensions* pNativeExtensions);

} // namespace Zephyros


#endif
