// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <sstream>

#include "include/cef_app.h"
#include "include/cef_application_mac.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"

#include "app.h"
#include "client_handler.h"
#include "resource_util.h"
#include "string_util.h"
#include "extension_handler.h"
#include "GLMenuItem.h"


// The global ClientHandler reference.
extern CefRefPtr<ClientHandler> g_handler;

bool g_isMessageLoopRunning = false;
bool g_isWindowLoaded = false;
bool g_isWindowBeingLoaded = true;

// Default content area size for newly created windows.
const int kDefaultWindowWidth = 800;
const int kDefaultWindowHeight = 600;


// Provide the CefAppProtocol implementation required by CEF.
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

@end


// Receives notifications from controls and the browser window.
// Will delete itself when done.
@interface ClientWindowDelegate : NSObject <NSWindowDelegate>

- (void) alert: (NSString*) title withMessage: (NSString*) message;

@end


@implementation ClientWindowDelegate

- (void) alert: (NSString*) title withMessage: (NSString*) message
{
    NSAlert *alert = [NSAlert alertWithMessageText: title
                                     defaultButton: @"OK"
                                   alternateButton: nil
                                       otherButton: nil
                         informativeTextWithFormat: @"%@", message];
    [alert runModal];
}

- (void) windowDidBecomeKey: (NSNotification*) notification
{
    if (g_handler.get() && g_handler->GetBrowser().get() && g_handler->GetBrowserId())
    {
        // Give focus to the browser window.
        g_handler->GetBrowser()->GetHost()->SetFocus(true);
    }
}

//
// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the window. By returning YES, we allow the window
// to be removed from the screen.
//
- (BOOL) windowShouldClose: (id) window
{
    // save the window geometry
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSRect rectWindow = [(NSWindow*) window frame];
    NSRect rectView = [[(NSWindow*) window contentView] frame];
    [defaults setObject: [NSKeyedArchiver archivedDataWithRootObject: @{
                          @"x": [NSNumber numberWithDouble: rectWindow.origin.x],
                          @"y": [NSNumber numberWithDouble: rectWindow.origin.y],
                          @"w": [NSNumber numberWithDouble: rectView.size.width],
                          @"h": [NSNumber numberWithDouble: rectView.size.height]
                          }]
                 forKey: @"wnd2"];
    [defaults synchronize];

    if (g_handler.get() && !g_handler->IsClosing())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
        {
            // notify the browser window that we would like to close it. This
            // will result in a call to ClientHandler::DoClose() if the
            // JavaScript 'onbeforeunload' event handler allows it
            browser->GetHost()->CloseBrowser(false);

            // cancel the close
            return NO;
        }
    }

    // try to make the window go away.
    [window autorelease];
    g_isWindowLoaded = false;

    // clean ourselves up after clearing the stack of anything that might have the window on it.
    [self performSelectorOnMainThread: @selector(cleanup:)
                           withObject: window
                        waitUntilDone: NO];
    
    // Allow the close
    return YES;
}

//
// Deletes itself.
//
- (void) cleanup: (id) window
{
    [self release];
}

@end


//
// Receives notifications from the application. Will delete itself when done.
//
@interface ClientAppDelegate : NSObject

@property(retain) NSWindow *window;

- (void) createApp: (id) object;

@end


@implementation ClientAppDelegate

//
// Create the application on the UI thread.
//
- (void) createApp: (id) object
{
    [NSApplication sharedApplication];
    [NSBundle loadNibNamed: @"MainMenu" owner: NSApp];

    // set the delegate for application events
    [NSApp setDelegate: self];
    
    [self createMainWindow];
}

- (void) createMainWindow
{
    // create the handler
    if (g_handler != NULL)
    {
        g_handler->ReleaseCefObjects();
        g_handler = NULL;
    }
    g_handler = new ClientHandler();

    // Create the delegate for control and browser window events.
    ClientWindowDelegate* delegate = [[ClientWindowDelegate alloc] init];
    
    // Create the main application window.
    NSRect rectScreen = [[NSScreen mainScreen] visibleFrame];
    NSRect rectWindow;
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSData *dataDefaults = [defaults objectForKey: @"wnd2"];
    if (dataDefaults != nil)
    {
        NSDictionary *dictWndRect = [NSKeyedUnarchiver unarchiveObjectWithData: dataDefaults];
        rectWindow = NSMakeRect(
            [(NSNumber*)[dictWndRect objectForKey: @"x"] doubleValue],
            [(NSNumber*)[dictWndRect objectForKey: @"y"] doubleValue],
            [(NSNumber*)[dictWndRect objectForKey: @"w"] doubleValue],
            [(NSNumber*)[dictWndRect objectForKey: @"h"] doubleValue]
        );
            
        if (rectWindow.origin.x + rectWindow.size.width > rectScreen.size.width)
            rectWindow.origin.x = rectScreen.size.width - rectWindow.size.width;
        if (rectWindow.origin.x < 0)
            rectWindow.origin.x = 0;
            
        if (rectWindow.origin.y + rectWindow.size.height > rectScreen.size.height)
            rectWindow.origin.y = rectScreen.size.height - rectWindow.size.height;
        if (rectWindow.origin.y < 0)
            rectWindow.origin.y = 0;
    }
    else
        rectWindow = NSMakeRect(0, rectScreen.size.height - kDefaultWindowHeight, kDefaultWindowWidth, kDefaultWindowHeight);
        
    self.window = [[UnderlayOpenGLHostingWindow alloc] initWithContentRect: rectWindow
                                                                 styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask )
                                                                   backing: NSBackingStoreBuffered
                                                                     defer: NO];
    self.window.title = @"Zephyros";
    self.window.delegate = delegate;
    self.window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

    // Rely on the window delegate to clean us up rather than immediately
    // releasing when the window gets closed. We use the delegate to do
    // everything from the autorelease pool so the window isn't on the stack
    // during cleanup (ie, a window close from javascript).
    [self.window setReleasedWhenClosed: NO];
    
    NSView* contentView = self.window.contentView;
    g_handler->SetMainHwnd(contentView);
    
    // create the browser view
    CefWindowInfo window_info;
    CefBrowserSettings settings;
    settings.web_security = STATE_DISABLED;
    
    // initialize window info to the defaults for a child window
    window_info.SetAsChild(contentView, 0, 0, rectWindow.size.width, rectWindow.size.height);
    
    CefBrowserHost::CreateBrowser(window_info, g_handler.get(), g_handler->GetStartupURL(), settings, NULL);
    
    // size the window
    NSRect r = [self.window contentRectForFrameRect: self.window.frame];
    r.size.width = rectWindow.size.width;
    r.size.height = rectWindow.size.height;
    [self.window setFrame: [self.window frameRectForContentRect: r] display: YES];
}

//
// Don't quit the application when the window is closed.
//
- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) sender
{
    return NO;
}

//
// Restore the window when the dock icon is clicked.
//
- (BOOL) applicationShouldHandleReopen: (NSApplication*) sender hasVisibleWindows: (BOOL) flag
{
    if (flag)
        [self.window orderFront: self];
    else
    {
        g_isWindowBeingLoaded = true;
        [self createMainWindow];
    }
    
    return YES;
}

//
// Called when the application's Quit menu item is selected.
//
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) sender
{
    // Request that all browser windows close.
    if (g_handler.get())
    {
        g_handler->CloseAllBrowsers(false);
        if (!g_handler->GetClientExtensionHandler()->InvokeCallbacks("onAppTerminating", CefListValue::Create()))
            return NSTerminateNow;
        
        // cancel the termination: the app will quit once all onAppTerminating callbacks have completed
        return NSTerminateCancel;
    }

    return NSTerminateNow;
}

//
// Sent immediately before the application terminates. This signal should not
// be called because we cancel the termination.
//
- (void) applicationWillTerminate: (NSNotification*) aNotification
{
}

//
// Invoked when a menu has been clicked.
//
- (IBAction) performClick: (id) sender
{
    NSString *commandId = [sender valueForKey: @"commandId"];
    
    if (g_handler.get())
    {
        CefRefPtr<CefListValue> args = CefListValue::Create();
        args->SetString(0, String([commandId UTF8String]));
        g_handler->GetClientExtensionHandler()->InvokeCallbacks("onMenuCommand", args);
    }
}

- (void) showAlert: (NSDictionary*) options
{
    NSString *title = [options objectForKey: @"title"];
    if (title == nil)
        title = @"";
    
    NSString *message = [options objectForKey: @"message"];
    if (message == nil)
        message = @"";
    
    App::AlertStyle style = App::AlertStyle::AlertInfo;
    NSNumber *numStyle = [options objectForKey: @"style"];
    if (numStyle != nil)
        style = (App::AlertStyle) [numStyle intValue];
    
    NSAlert *alert = [NSAlert alertWithMessageText: title
                                     defaultButton: @"OK"
                                   alternateButton: nil
                                       otherButton: nil
                         informativeTextWithFormat: @"%@", message];
    switch (style)
    {
        case App::AlertStyle::AlertInfo:
            alert.alertStyle = NSInformationalAlertStyle;
            break;
        case App::AlertStyle::AlertWarning:
            alert.alertStyle = NSWarningAlertStyle;
            break;
        case App::AlertStyle::AlertError:
            alert.alertStyle = NSCriticalAlertStyle;
            break;
    }
    
    [alert runModal];
}

@end


ClientAppDelegate *g_appDelegate = nil;

int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
    CefRefPtr<ClientApp> app(new ClientApp);

    // execute the secondary process, if any
    int exit_code = CefExecuteProcess(main_args, app.get());
    if (exit_code >= 0)
        return exit_code;

    // initialize the AutoRelease pool
    NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

    // initialize the ClientApplication instance
    [ClientApplication sharedApplication];

    // parse command line arguments
    App::InitCommandLine(argc, argv);

    CefSettings settings;

    // populate the settings based on command line arguments
    App::GetSettings(settings);

    // initialize CEF
    CefInitialize(main_args, settings, app.get());

    g_appDelegate = [[ClientAppDelegate alloc] init];

    // create the application window
    [g_appDelegate performSelectorOnMainThread: @selector(createApp:) withObject: nil waitUntilDone: NO];

    // run the application message loop
    g_isMessageLoopRunning = true;
    CefRunMessageLoop();
    g_isMessageLoopRunning = false;

    // shut down CEF
    if (g_handler != NULL)
        g_handler->ReleaseCefObjects();
    CefShutdown();

    // release the handler
    g_handler = NULL;

    // release the delegate
    if (g_appDelegate != nil)
        [g_appDelegate release];
    
    // release the AutoRelease pool
    [autopool release];

    return 0;
}


/////////////////////////////////////////////////////////////////////
// Global functions

namespace App {


void QuitMessageLoop()
{
    if (g_isMessageLoopRunning)
        CefQuitMessageLoop();
}
    
void Alert(String title, String msg, AlertStyle style)
{
    NSDictionary *options = @{
        @"title": [NSString stringWithUTF8String: title.c_str()],
        @"message": [NSString stringWithUTF8String: msg.c_str()],
        @"style": [NSNumber numberWithInt: style]
    };
    
    [g_appDelegate showAlert: options];
//    [g_appDelegate performSelectorOnMainThread: @selector(showAlert:) withObject: options waitUntilDone: NO];
}
    
void BeginWait()
{
}
    
void EndWait()
{
}

void Log(String msg)
{
    NSLog(@"%@", [NSString stringWithUTF8String: msg.c_str()]);
}

} // namespace App