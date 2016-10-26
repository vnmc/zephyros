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


#import <Cocoa/Cocoa.h>
#import <sstream>

#import "lib/cef/include/cef_app.h"
#import "lib/cef/include/cef_application_mac.h"
#import "lib/cef/include/cef_browser.h"
#import "lib/cef/include/cef_frame.h"
#import "lib/cef/include/cef_runnable.h"

#import "base/app.h"
#import "base/cef/client_app.h"
#import "base/cef/local_scheme_handler.h"
#import "base/cef/ZPYCEFAppDelegate.h"


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
    // invoke the "onAppTerminating" callbacks and cancel the close
    if (g_handler.get() && !g_handler->IsClosing())
        g_handler->GetClientExtensionHandler()->InvokeCallbacks("onAppTerminating", CefListValue::Create());
}

@end


ZPYCEFAppDelegate *g_appDelegate = nil;

namespace Zephyros {
    
int RunApplication(int argc, char* argv[])
{
    CefMainArgs main_args(argc, (char**) argv);
    CefRefPtr<Zephyros::ClientApp> app(new Zephyros::ClientApp);

    // execute the secondary process, if any
    int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
    if (exit_code >= 0)
        return exit_code;

    // initialize the ClientApplication instance
    [ClientApplication sharedApplication];

    // parse command line arguments
    Zephyros::App::InitCommandLine(argc, argv);

    CefSettings settings;

    // populate the settings based on command line arguments
    Zephyros::App::GetSettings(settings);

    // set the user agent
    CefString(&settings.user_agent).FromASCII(Zephyros::App::GetUserAgent().c_str());

    // initialize CEF
    CefInitialize(main_args, settings, app.get(), NULL);
    CefRegisterSchemeHandlerFactory("local", "", new LocalSchemeHandlerFactory());

    Zephyros::AbstractLicenseManager* pLicenseMgr = Zephyros::GetLicenseManager();
    g_appDelegate = [[ZPYCEFAppDelegate alloc] init];

    if (pLicenseMgr != NULL)
        pLicenseMgr->Start();

    if (pLicenseMgr == NULL || pLicenseMgr->CanStartApp())
    {
        // create the application window
        [g_appDelegate performSelectorOnMainThread: @selector(createApp:) withObject: nil waitUntilDone: YES];
        [g_appDelegate.window orderFrontRegardless];

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

    Zephyros::Shutdown();

    return 0;
}
    
} // namespace Zephyros
