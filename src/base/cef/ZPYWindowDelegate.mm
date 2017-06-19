/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
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
        
        //*
        // register for application hide/unhide notifications
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationDidHide:)
                                                     name: NSApplicationDidHideNotification
                                                   object: nil];
        //*/

        /*
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationDidUnhide:)
                                                     name: NSApplicationDidUnhideNotification
                                                   object: nil];
        */
    }

    return self;
}

- (void) applicationDidHide: (NSNotification*) notification
{
    NSLog(@"!!! window hidden");
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) alert: (NSString*) title withMessage: (NSString*) message
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = title;
    alert.informativeText = message;
    [alert runModal];
}

/**
 * Called when we are activated (when we gain focus).
 */
//- (void) windowDidBecomeKey: (NSNotification*) notification
//{
    /*
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetFocus(true);
    }*/
//}

/**
 * Called when we are deactivated (when we lose focus).
 */
//- (void) windowDidResignKey: (NSNotification*) notification
//{
    /*
    if (g_handler.get())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
            browser->GetHost()->SetFocus(false);
    }*/
//}

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
    
    return YES;
}

@end
