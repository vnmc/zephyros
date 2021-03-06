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


#import <objc/runtime.h>

#import <JavaScriptCore/JavaScriptCore.h>
#import <SystemConfiguration/SystemConfiguration.h>

#import "lib/cef/include/cef_application_mac.h"
#import "lib/cef/include/cef_browser.h"
#import "lib/cef/include/cef_frame.h"

#import "base/types.h"

#import "base/cef/client_handler.h"
#import "base/cef/ZPYCEFAppDelegate.h"

#import "components/ZPYMenuItem.h"

#import "native_extensions/custom_url_manager.h"
#import "native_extensions/path.h"

#import "zephyros.h"
#import "native_extensions.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;
extern bool g_isWindowBeingLoaded;
extern bool g_isWindowLoaded;


@implementation ZPYCEFAppDelegate

- (id) init
{
    self = [super init];
    
    self.windowDelegate = nil;

    /*
    if (Zephyros::GetUpdaterURL() && _tcslen(Zephyros::GetUpdaterURL()) > 0)
    {
        self.updater = [[SUUpdater alloc] init];
        self.updater.feedURL = [NSURL URLWithString: [NSString stringWithUTF8String: Zephyros::GetUpdaterURL()]];
    }
     */

    // TODO: check if this works (not too early)
    // register ourselves as delegate for the notification center
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate: self];
    
    return self;
}

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
    [super applicationDidFinishLaunching: notification];

    if ((!Zephyros::GetLicenseManager() || Zephyros::GetLicenseManager()->CanStartApp()) && g_isWindowLoaded)
    {
        self.window.isVisible = YES;
        [self.window makeKeyAndOrderFront: self];
    }
}

- (void) createApp: (id) object
{
    NSApplication* app = [NSApplication sharedApplication];

    if (Zephyros::GetOSXInfo().szMainNibName)
    {
        NSArray* tl;
        [[NSBundle mainBundle] loadNibNamed: [NSString stringWithUTF8String: Zephyros::GetOSXInfo().szMainNibName] owner: app topLevelObjects: &tl];
    }

    // set the delegate for application events
    app.delegate = self;
    
    [self applicationWillFinishLaunching: nil];
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
    g_handler = new Zephyros::ClientHandler();
    
    // create the main application window
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
    {
        rectWindow = NSMakeRect(
            0,
            rectScreen.size.height - Zephyros::GetDefaultWindowSize().nHeight,
            Zephyros::GetDefaultWindowSize().nWidth,
            Zephyros::GetDefaultWindowSize().nHeight
        );
    }
    
    self.window = [[UnderlayOpenGLHostingWindow alloc] initWithContentRect: rectWindow
                                                                 styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask | NSUnifiedTitleAndToolbarWindowMask
                                                                   backing: NSBackingStoreBuffered
                                                                     defer: NO];
    
    // create the delegate for control and browser window events
    if (self.windowDelegate == nil)
        self.windowDelegate = [[ZPYWindowDelegate alloc] initWithWindow: self.window];
    
    self.window.delegate = self.windowDelegate;
    self.window.title = [NSString stringWithUTF8String: Zephyros::GetAppName()];
    self.window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
    self.window.hidesOnDeactivate = NO;

    // rely on the window delegate to clean us up rather than immediately
    // releasing when the window gets closed. We use the delegate to do
    // everything from the autorelease pool so the window isn't on the stack
    // during cleanup (ie, a window close from javascript)
    self.window.releasedWhenClosed = NO;
    
    NSView* contentView = self.window.contentView;
    contentView.wantsLayer = YES;
    g_handler->SetMainHwnd(contentView);
    
    // create the browser view
    CefWindowInfo windowInfo;
    CefBrowserSettings settings;
    settings.web_security = STATE_DISABLED;
    
    // initialize window info to the defaults for a child window
    windowInfo.SetAsChild(contentView, 0, 0, rectWindow.size.width, rectWindow.size.height);
    
    CefBrowserHost::CreateBrowser(windowInfo, g_handler.get(), Zephyros::GetAppURL(), settings, NULL);
    
    // size the window
    NSRect r = [self.window contentRectForFrameRect: self.window.frame];
    r.size.width = rectWindow.size.width;
    r.size.height = rectWindow.size.height;
    [self.window setFrame: [self.window frameRectForContentRect: r] display: YES];
}

@end
