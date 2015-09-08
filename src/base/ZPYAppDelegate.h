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


#ifndef Zephyros_ZPYAppDelegate_h
#define Zephyros_ZPYAppDelegate_h
#pragma once


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

#endif // Zephyros_ZPYAggDelegate_h
