//
//  app.cpp
//  Zephyros
//
//  Created by Matthias Christen on 20.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#include "zephyros_impl.h"
#include "app.h"

#include "path.h"

// TODO remove this somehow...
#if USE_CEF
#include "client_handler.h"
#include "extension_handler.h"
#endif


int main(int argc, const char * argv[])
{
    Zephyros::LicenseManager* pLicenseManager = new Zephyros::LicenseManager();
    pLicenseManager->SetLicenseInfo(1002, TEXT(""));
    pLicenseManager->AddObsoleteLicenseInfo(1001, TEXT(""));
    pLicenseManager->SetNumberOfDemoDays(7);
    
    Zephyros::SetLicenseManager(pLicenseManager);
    
#ifdef OS_MACOSX
    Zephyros::SetUpdaterURL("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast.xml");
#endif
#ifdef OS_WIN
#ifdef _WIN64
    Zephyros::SetUpdaterURL(TEXT("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast-win64.xml"));
#else
    Zephyros::SetUpdaterURL(TEXT("http://awesome.vanamco.com/Ghostlab2/update/ghostlab2-cast-win32.xml"));
#endif
#endif
    
    //    Zephyros::SetNativeExtension();
    
    
    return Zephyros::Init(argc, argv, TEXT("ZephyrosSampleApp"), TEXT("1.0.0"), TEXT("app/index.html"));
}
