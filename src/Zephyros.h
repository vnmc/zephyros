//
//  Zephyros.h
//  Zephyros
//
//  Created by Matthias Christen on 08.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#import <Cocoa/Cocoa.h>

//! Project version number for Zephyros.
FOUNDATION_EXPORT double ZephyrosVersionNumber;

//! Project version string for Zephyros.
FOUNDATION_EXPORT const unsigned char ZephyrosVersionString[];


// import all the public headers

#ifdef __OBJC__

#import "base/ZPYAppDelegate.h"
#import "base/webview/ZPYWebViewAppDelegate.h"
#import "base/cef/ZPYCEFAppDelegate.h"

#endif
