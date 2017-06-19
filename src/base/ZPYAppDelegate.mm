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

#import "base/ZPYAppDelegate.h"

#import "components/ZPYMenuItem.h"

#import "native_extensions/custom_url_manager.h"
#import "native_extensions/path.h"

#import "zephyros.h"
#import "zephyros_strings.h"
#import "native_extensions.h"


@implementation ZPYAppDelegate

- (id) init
{
    self = [super init];
    
    m_previousDropURL = nil;
    m_previousDropURLTime = 0;
    m_eventsRegistered = false;

    return self;
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) sender
{
    return NSTerminateNow;
}

/**
 * Don't quit the application when the window is closed.
 */
- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) sender
{
#ifndef APPSTORE
    return Zephyros::GetLicenseManager() != nil && !Zephyros::GetLicenseManager()->CanStartApp() ? YES : NO;
#else
    return NO;
#endif
}

/**
 * Restore the window when the dock icon is clicked.
 */
- (BOOL) applicationShouldHandleReopen: (NSApplication*) sender hasVisibleWindows: (BOOL) flag
{
#ifndef APPSTORE
    if (Zephyros::GetLicenseManager() != nil && !Zephyros::GetLicenseManager()->CanStartApp())
        return NO;
#endif
    
    if (flag)
        [self.window orderFront: self];
    else if (self.window != nil)
        [self.window makeKeyAndOrderFront: self];
    
    return YES;
}

- (void) applicationWillFinishLaunching: (NSNotification*) notification
{
    if (m_eventsRegistered)
        return;

    // register URL drops to the dock
    // (we need to do this in applicationWillFinishLaunching so it picks up link clicks when the app isn't started yet)
    // http://stackoverflow.com/questions/3115657/nsapplicationdelegate-application-active-because-of-url-protocol

    [[NSAppleEventManager sharedAppleEventManager] setEventHandler: self
                                                       andSelector: @selector(handleGetURLEvent:withReplyEvent:)
                                                     forEventClass: kInternetEventClass
                                                        andEventID: kAEGetURL];

    Zephyros::AbstractLicenseManager* pMgr = Zephyros::GetLicenseManager();
    if (pMgr && pMgr->GetReceiptChecker())
        pMgr->GetReceiptChecker()->CopyAppStoreReceipt();
    
    m_eventsRegistered = true;
}

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
    if ([[NSApplication sharedApplication] respondsToSelector: @selector(isAutomaticCustomizeTouchBarMenuItemEnabled)])
    {
        [NSApplication sharedApplication].automaticCustomizeTouchBarMenuItemEnabled = YES;
        self.touchBarHandler = [[ZPYTouchBarHandler alloc] init];
    }

#ifndef APPSTORE
    // initialize the license manager (if available)
    if (Zephyros::GetLicenseManager())
    {
        Zephyros::GetLicenseManager()->Start();
        if (!Zephyros::GetLicenseManager()->CanStartApp())
            [NSApp terminate: self];
    }

    // create and initialize the updater
    if (Zephyros::GetUpdaterURL() && _tcslen(Zephyros::GetUpdaterURL()) > 0)
    {
        self.updater = [[SUUpdater alloc] init];
        self.updater.feedURL = [NSURL URLWithString: [NSString stringWithUTF8String: Zephyros::GetUpdaterURL()]];
        [self.updater resetUpdateCycle];
    }
#endif
}

/**
 * Called when an URL is dropped on the dock icon.
 */
- (void) handleGetURLEvent: (NSAppleEventDescriptor*) event withReplyEvent: (NSAppleEventDescriptor*) replyEvent
{
    NSString *url = [[event paramDescriptorForKeyword: keyDirectObject] stringValue];
    
    bool isSameURL = false;
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    if ([url isEqualToString: m_previousDropURL] && now - m_previousDropURLTime < 1)
        isSameURL = true;

    m_previousDropURL = url;
    m_previousDropURLTime = now;

    if (isSameURL)
        return;
    
    if ([url hasPrefix: @"http://"] || [url hasPrefix: @"https://"])
    {
        m_launchPaths.push_back(new Zephyros::Path([url UTF8String]));
        [self addLaunchPaths];
    }
    else
    {
        Zephyros::CustomURLManager* pMgr = Zephyros::GetNativeExtensions()->GetCustomURLManager();
        
        pMgr->AddURL([url UTF8String]);

        if (Zephyros::GetNativeExtensions()->GetNativeExtensionsAdded())
            pMgr->FireCustomURLs();
    }
}

/**
 * Called when a file or a folder is dropped on the dock icon.
 */
- (void) application: (NSApplication*) sender openFiles: (NSArray*) filenames
{
    NSFileManager* mgr = [NSFileManager defaultManager];
    
    for (NSString* filename in filenames)
    {
        NSString* dir = filename;
        NSURL* url = nil;
        
        // check if the filename is a directory, otherwise take the parent directory
        BOOL isDirectory = NO;
        if (![mgr fileExistsAtPath: filename isDirectory: &isDirectory])
            continue;
        
        if (isDirectory)
            dir = [filename stringByDeletingLastPathComponent];
#ifdef APPSTORE
        else
        {
            // can't get security-scope bookmark to the containing directory; discard
            continue;
        }
#endif
        
        // create a security-scope bookmkark
        url = [NSURL fileURLWithPath: dir isDirectory: YES];
        
        NSError *error = nil;
        NSData *bookmarkData = [url bookmarkDataWithOptions: NSURLBookmarkCreationWithSecurityScope
                             includingResourceValuesForKeys: nil
                                              relativeToURL: nil
                                                      error: &error];
        if (bookmarkData != nil)
        {
            BOOL bookmarkDataIsStale;
            url = [NSURL URLByResolvingBookmarkData: bookmarkData
                                            options: NSURLBookmarkResolutionWithSecurityScope
                                      relativeToURL: nil
                                bookmarkDataIsStale: &bookmarkDataIsStale
                                              error: &error];
        }
        else
            url = nil;
        
        m_launchPaths.push_back(new Zephyros::Path([filename UTF8String], url == nil ? "" : [[url absoluteString] UTF8String], url != nil));
    }
    
    [self addLaunchPaths];
}

- (void) addLaunchPaths
{
    if (!Zephyros::GetNativeExtensions()->GetNativeExtensionsAdded() || m_launchPaths.size() == 0)
        return;
    
    Zephyros::JavaScript::Array listFilenames = Zephyros::JavaScript::CreateArray();
    
    int i = 0;
    for (Zephyros::Path* pPath: m_launchPaths)
    {
        listFilenames->SetDictionary(i++, pPath->CreateJSRepresentation());
        delete pPath;
    }
    
    Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
    args->SetList(0, listFilenames);
    Zephyros::GetNativeExtensions()->GetClientExtensionHandler()->InvokeCallbacks("onAddURLs", args);
    
    m_launchPaths.clear();
}

/**
 * Enables/disables menu items.
 * NOTE: for this to work, the menu containing the menu item must have autoEnablesMenuItems=NO.
 */
- (void) setMenuItemStatuses: (Zephyros::App::MenuItemStatuses&) statuses
{
    [self setMenuItemStatusesRecursive: statuses forMenu: [NSApp mainMenu]];
}

- (void) setMenuItemStatusesRecursive: (Zephyros::App::MenuItemStatuses&) statuses forMenu: (NSMenu*) menu
{
    for (NSMenuItem *item in menu.itemArray)
    {
        if ([item isKindOfClass: ZPYMenuItem.class])
        {
            NSString *commandId = [(ZPYMenuItem*) item commandId];
            if (commandId)
            {
                String strCommandId([commandId UTF8String]);
                if (statuses.find(strCommandId) != statuses.end())
                {
                    int status = statuses.at(strCommandId);

                    [item setEnabled: (status & 0x01) ? YES : NO];
                    [item setState: (status & 0x02) ? NSOnState : NSOffState];
                }
            }
        }
        
        if (item.hasSubmenu)
            [self setMenuItemStatusesRecursive: statuses forMenu: item.submenu];
    }
}

- (nullable NSTouchBarItem*) touchBar: (NSTouchBar*) touchBar makeItemForIdentifier: (NSTouchBarItemIdentifier) identifier
{
    return [self.touchBarHandler itemForId: identifier];
}

- (BOOL) userNotificationCenter: (NSUserNotificationCenter*) center shouldPresentNotification: (NSUserNotification*) notification
{
    return YES;
}

@end
