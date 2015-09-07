//
//  ZephyrosAppDelegate.h
//  Zephyros
//
//  Created by Matthias Christen on 08.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#ifndef Zephyros_ZPYAppDelegate_h
#define Zephyros_ZPYAppDelegate_h

#import <Cocoa/Cocoa.h>

#ifndef APPSTORE
#import <Sparkle/Sparkle.h>
#endif

#import "native_extensions/path.h"

#import "zephyros.h"
#import "native_extensions.h"


@interface ZPYAppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
@protected
    std::vector<Zephyros::Path*> m_launchPaths;
    
    bool m_nativeExtensionsAdded;
    ClientExtensionHandlerPtr m_extension;
}


@property (retain) IBOutlet NSWindow *window;

#ifndef APPSTORE
@property (retain) SUUpdater *updater;
#endif


- (void) addLaunchPaths;

- (void) createMenuItems;
- (void) setMenuItemStatuses: (NSDictionary*) statuses;


@end

#endif
