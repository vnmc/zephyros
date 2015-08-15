//
//  ZPYWindowDelegate.m
//  Zephyros
//
//  Created by Matthias Christen on 20.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "lib/cef/include/cef_browser.h"

#import "base/cef/client_handler.h"
#import "base/cef/ZPYWindowDelegate.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;
extern bool g_isWindowLoaded;


@implementation ZPYWindowDelegate

- (id) initWithWindow: (NSWindow*) window
{
    if (self = [super init])
    {
        m_window = window;
        [m_window setDelegate:self];
        
        // Register for application hide/unhide notifications.
        
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationDidHide:)
                                                     name: NSApplicationDidHideNotification
                                                   object: nil];
        
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationDidUnhide:)
                                                     name: NSApplicationDidUnhideNotification
                                                   object: nil];
    }
    
    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) alert: (NSString*) title withMessage: (NSString*) message
{
    NSAlert *alert = [NSAlert alertWithMessageText: title
                                     defaultButton: @"OK"
                                   alternateButton: nil
                                       otherButton: nil
                         informativeTextWithFormat: @"%@", message];
    [alert runModal];
}

/**
 * Called when we are activated (when we gain focus).
 */
- (void) windowDidBecomeKey: (NSNotification*) notification
{
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetFocus(true);
    }
}

/**
 * Called when we are deactivated (when we lose focus).
 */
- (void) windowDidResignKey: (NSNotification*) notification
{
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetFocus(false);
    }
}

/**
 * Called when we have been minimized.
 */
- (void) windowDidMiniaturize: (NSNotification*) notification
{
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetWindowVisibility(false);
    }
}

/**
 * Called when we have been unminimized.
 */
- (void) windowDidDeminiaturize: (NSNotification*) notification
{
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetWindowVisibility(true);
    }
}

/**
 * Called when the application has been hidden.
 */
- (void) applicationDidHide: (NSNotification*) notification
{
    // if the window is miniaturized then nothing has really changed
    if (![m_window isMiniaturized])
    {
        if (g_handler.get())
        {
            CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
            if (browser.get())
                browser->GetHost()->SetWindowVisibility(false);
        }
    }
}

/**
 * Called when the application has been unhidden.
 */
- (void) applicationDidUnhide: (NSNotification*) notification
{
    // if the window is miniaturized then nothing has really changed.
    if (![m_window isMiniaturized])
    {
        if (g_handler.get())
        {
            CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
            if (browser.get())
                browser->GetHost()->SetWindowVisibility(true);
        }
    }
}

/**
 * Called when the window is about to close. Perform the self-destruction
 * sequence by getting rid of the window. By returning YES, we allow the window
 * to be removed from the screen.
 */
- (BOOL) windowShouldClose: (id) window
{
    // save the window geometry
    NSRect rectWindow = [(NSWindow*) window frame];
    NSRect rectView = [[(NSWindow*) window contentView] frame];
    
    NSDictionary* data = @{
        @"x": [NSNumber numberWithDouble: rectWindow.origin.x],
        @"y": [NSNumber numberWithDouble: rectWindow.origin.y],
        @"w": [NSNumber numberWithDouble: rectView.size.width],
        @"h": [NSNumber numberWithDouble: rectView.size.height]
    };
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject: [NSKeyedArchiver archivedDataWithRootObject: data] forKey: @"wnd2"];
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
    
    // try to make the window go away
//    [window autorelease];
    g_isWindowLoaded = false;

    /*
    // clean ourselves up after clearing the stack of anything that might have the window on it
    [self performSelectorOnMainThread: @selector(cleanup:)
                           withObject: window
                        waitUntilDone: NO];
    */
    
    // allow closing
    return YES;
}

//
// Deletes itself.
//
- (void) cleanup: (id) window
{
//    [self release];
}

@end