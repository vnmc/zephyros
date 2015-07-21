// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <sstream>

#include "lib/cef/include/cef_app.h"
#include "lib/cef/include/cef_application_mac.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/cef_runnable.h"

#include "base/zephyros_impl.h"
#include "base/app.h"

#include "base/cef/client_app.h"
//#include "client_handler.h"
//#include "resource_util.h"
//#include "string_util.h"
//#include "extension_handler.h"
//#include "licensing.h"

#include "base/cef/ZPYCEFAppDelegate.h"


// the global ClientHandler reference
extern CefRefPtr<Zephyros::ClientHandler> g_handler;

bool g_isMessageLoopRunning = false;
bool g_isWindowLoaded = false;
bool g_isWindowBeingLoaded = true;


/**
 * Provide the CefAppProtocol implementation required by CEF.
 */
@interface ClientApplication : NSApplication<CefAppProtocol>
{
@private
    BOOL m_handlingSendEvent;
}

@end


@implementation ClientApplication

- (BOOL) isHandlingSendEvent
{
    return m_handlingSendEvent;
}

- (void) setHandlingSendEvent: (BOOL) handlingSendEvent
{
    m_handlingSendEvent = handlingSendEvent;
}

- (void) sendEvent: (NSEvent*) event
{
    CefScopedSendingEvent sendingEventScoper;
    [super sendEvent: event];
}

- (void) terminate: (id) sender
{
    ZPYCEFAppDelegate* delegate = static_cast<ZPYCEFAppDelegate*>([NSApp delegate]);
    [delegate tryToTerminateApplication: self];
    
    // return, don't exit; the application is responsible for exiting on its own
}

@end


ZPYCEFAppDelegate *g_appDelegate = nil;

namespace Zephyros {

int InitCEFApplication(int argc, const char* argv[])
{
    CefMainArgs main_args(argc, (char**) argv);
    CefRefPtr<Zephyros::ClientApp> app(new Zephyros::ClientApp);

    // execute the secondary process, if any
    int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
    if (exit_code >= 0)
        return exit_code;

    // initialize the AutoRelease pool
//    NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

    // initialize the ClientApplication instance
    [ClientApplication sharedApplication];

    // parse command line arguments
    Zephyros::App::InitCommandLine(argc, argv);

    CefSettings settings;

    // populate the settings based on command line arguments
    Zephyros::App::GetSettings(settings);

    // initialize CEF
    CefInitialize(main_args, settings, app.get(), NULL);

    Zephyros::LicenseManager* pLicenseMgr = Zephyros::GetLicenseManager();
    g_appDelegate = [[ZPYCEFAppDelegate alloc] init];

    if (pLicenseMgr != NULL)
        pLicenseMgr->Start();

    if (pLicenseMgr == NULL || pLicenseMgr->CanStartApp())
    {
        // create the application window
        [g_appDelegate performSelectorOnMainThread: @selector(createApp:) withObject: nil waitUntilDone: NO];

        // run the application message loop
        g_isMessageLoopRunning = true;
        CefRunMessageLoop();
        g_isMessageLoopRunning = false;
    }

    // shut down CEF
    if (g_handler != NULL)
        g_handler->ReleaseCefObjects();
    CefShutdown();

    // release the handler
    g_handler = NULL;

    // release the delegate
//    if (g_appDelegate != nil)
//        [g_appDelegate release];
    
    // release the AutoRelease pool
//    [autopool release];
    
    return 0;
}
    
} // namespace Zephyros
