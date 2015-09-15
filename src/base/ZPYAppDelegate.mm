/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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
    
    m_nativeExtensionsAdded = false;
    m_extension = new Zephyros::ClientExtensionHandler();
    
    return self;
}

- (void) dealloc
{
    delete m_extension;
}

- (void) awakeFromNib
{
    Zephyros::AbstractLicenseManager* pMgr = Zephyros::GetLicenseManager();
    if (pMgr && pMgr->GetReceiptChecker())
        pMgr->GetReceiptChecker()->CopyAppStoreReceipt();
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
    // register URL drops to the dock
    // (we need to do this in applicationWillFinishLaunching so it picks up link clicks when the app isn't started yet)
    // http://stackoverflow.com/questions/3115657/nsapplicationdelegate-application-active-because-of-url-protocol

    [[NSAppleEventManager sharedAppleEventManager] setEventHandler: self
                                                       andSelector: @selector(handleGetURLEvent:withReplyEvent:)
                                                     forEventClass: kInternetEventClass
                                                        andEventID: kAEGetURL];
}

/**
 * Called when an URL is dropped on the dock icon.
 */
- (void) handleGetURLEvent: (NSAppleEventDescriptor*) event withReplyEvent: (NSAppleEventDescriptor*) replyEvent
{
    NSString *url = [[event paramDescriptorForKeyword: keyDirectObject] stringValue];
    
    if ([url hasPrefix: @"http://"] || [url hasPrefix: @"https://"])
    {
        m_launchPaths.push_back(new Zephyros::Path([url UTF8String]));
        [self addLaunchPaths];
    }
    else
    {
        Zephyros::CustomURLManager* pMgr = Zephyros::GetNativeExtensions()->GetCustomURLManager();
        
        pMgr->AddURL([url UTF8String]);
        
        if (m_nativeExtensionsAdded)
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
    if (!m_nativeExtensionsAdded || m_launchPaths.size() == 0)
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
    m_extension->InvokeCallbacks("onAddURLs", args);
    
    m_launchPaths.clear();
}

/**
 * Create extra menu items.
 * Subclasses should override if desired
 */
- (void) createMenuItems
{
#ifndef APPSTORE
    if (Zephyros::GetUpdaterURL() && _tcslen(Zephyros::GetUpdaterURL()) > 0)
    {
        NSMenu *appMenu = [[[NSApp mainMenu] itemAtIndex: 0] submenu];
    
        // create the "check for updates" menu item
        NSMenuItem *checkUpdatesMenuItem = [[NSMenuItem alloc] initWithTitle: [NSString stringWithUTF8String: Zephyros::GetString(ZS_UPDATES_CHECK).c_str()]
                                                                      action: @selector(checkForUpdates:)
                                                               keyEquivalent: @"u"];
        checkUpdatesMenuItem.keyEquivalentModifierMask = NSShiftKeyMask | NSCommandKeyMask;
        checkUpdatesMenuItem.target = self.updater;
        [appMenu insertItem: checkUpdatesMenuItem atIndex: 1];
    }
#endif
}

/**
 * Enables/disables menu items.
 * NOTE: for this to work, the menu containing the menu item must have autoEnablesMenuItems=NO.
 */
- (void) setMenuItemStatuses: (NSDictionary*) statuses
{
    [self setMenuItemStatusesRecursive: statuses forMenu: [NSApp mainMenu]];
}

- (void) setMenuItemStatusesRecursive: (NSDictionary*) statuses forMenu: (NSMenu*) menu
{
    for (NSMenuItem *item in menu.itemArray)
    {
        if ([item isKindOfClass: ZPYMenuItem.class])
        {
            NSNumber *newStatus = [statuses objectForKey: [(ZPYMenuItem*) item commandId]];
            if (newStatus != nil)
            {
                int status = [newStatus intValue];
                
                [item setEnabled: (status & 0x01) ? YES : NO];
                item.state = (status & 0x02) ? NSOnState : NSOffState;
            }
        }
        
        if (item.hasSubmenu)
            [self setMenuItemStatusesRecursive: statuses forMenu: item.submenu];
    }
}

- (BOOL) userNotificationCenter: (NSUserNotificationCenter*) center shouldPresentNotification: (NSUserNotification*) notification
{
    return YES;
}

- (ClientExtensionHandlerPtr) extensionHandler
{
    return m_extension;
}

@end
