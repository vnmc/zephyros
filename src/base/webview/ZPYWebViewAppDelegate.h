//
//  ZephyrosAppDelegate.h
//  Zephyros
//
//  Created by Matthias Christen on 08.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#ifndef Zephyros_ZPYWebViewAppDelegate_h
#define Zephyros_ZPYWebViewAppDelegate_h

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#import "base/ZPYAppDelegate.h"
#import "base/webview/webview_extension.h"

#import "components/ZPYWebView.h"


@interface ZPYWebViewAppDelegate : ZPYAppDelegate

@property (assign) IBOutlet ZPYWebView *view;

@end

#endif
