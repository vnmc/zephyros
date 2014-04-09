//
//  AppDelegate.h
//  Zephyros
//
//  Created by Matthias Christen on 24.11.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#import "GLWebView.h"
#import "webview_extension.h"


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    ClientExtensionHandlerPtr m_extension;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet GLWebView *view;

@end
