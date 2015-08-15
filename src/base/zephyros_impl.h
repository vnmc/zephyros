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
// Zephyros Functions

int Init(int argc, const char* argv[], const TCHAR* appName, const TCHAR* appVersion, const TCHAR* appURL);
int InitApplication(int argc, const char** argv);
void Shutdown();
    
const TCHAR* GetAppName();
const TCHAR* GetAppVersion();
const TCHAR* GetAppURL();
    
const TCHAR* GetRegistryKey();
void SetRegistryKey(const TCHAR* registryKey);
    
const TCHAR* GetUpdaterURL();
void SetUpdaterURL(const TCHAR* url);
    
LicenseManager* GetLicenseManager();
void SetLicenseManager(LicenseManager* pLicenseManager);
    
NativeExtensions* GetNativeExtensions();
void SetNativeExtensions(NativeExtensions* pNativeExtensions);
    
String GetString(int stringId);
void SetString(int stringId, String str);

} // namespace Zephyros


#endif
