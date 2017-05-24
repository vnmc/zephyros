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


#ifndef Zephyros_ZPYAppDelegate_h
#define Zephyros_ZPYAppDelegate_h
#pragma once


#import <Cocoa/Cocoa.h>

#ifndef APPSTORE
#import <Sparkle/Sparkle.h>
#endif

#import "base/app.h"
#import "base/ZPYTouchBarHandler.h"

#import "native_extensions/path.h"

#import "zephyros.h"
#import "jsbridge.h"
#import "native_extensions.h"


@interface ZPYAppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate, NSTouchBarProvider, NSTouchBarDelegate>
{
@protected
    std::vector<Zephyros::Path*> m_launchPaths;

    NSString* m_previousDropURL;
    NSTimeInterval m_previousDropURLTime;
}


@property (retain) IBOutlet NSWindow *window;

#ifndef APPSTORE
@property (retain) SUUpdater *updater;
#endif

@property ZPYTouchBarHandler *touchBarHandler;
@property NSTouchBar *touchBar;


- (void) addLaunchPaths;

- (void) setMenuItemStatuses: (Zephyros::App::MenuItemStatuses&) statuses;

@end

#endif // Zephyros_ZPYAggDelegate_h
