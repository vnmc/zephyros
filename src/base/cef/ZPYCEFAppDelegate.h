//
//  ZephyrosAppDelegate.h
//  Zephyros
//
//  Created by Matthias Christen on 08.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#ifndef Zephyros_ZPYCEFAppDelegate_h
#define Zephyros_ZPYCEFAppDelegate_h

#import <Cocoa/Cocoa.h>
#import "base/ZPYAppDelegate.h"


@interface ZPYCEFAppDelegate : ZPYAppDelegate

- (void) createApp: (id) object;
- (void) createMainWindow;
- (void) tryToTerminateApplication: (NSApplication*) app;

@end

#endif
