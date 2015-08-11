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


void StartApplication(int argc, const char* argv[])
{
    Zephyros::LicenseManagerInfo info;
    memset(&info, 0, sizeof(Zephyros::LicenseManagerInfo));
    
    info.currentLicenseInfo.productId = 1002;
    info.currentLicenseInfo.pubkey = "";
    info.numDemoDays = 7;
    
    /*
    Zephyros::LicenseManager* pLicenseManager = new Zephyros::LicenseManager();
    pLicenseManager->SetLicenseInfo(1002, "");
    pLicenseManager->AddObsoleteLicenseInfo(1001, "");
    pLicenseManager->SetNumberOfDemoDays(7);
     */
    
    //Zephyros::SetLicenseManager(new Zephyros::LicenseManager(&info));
    
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


    Zephyros::Init(argc, argv, TEXT("ZephyrosSampleApp"), TEXT("1.0.0"));
}
