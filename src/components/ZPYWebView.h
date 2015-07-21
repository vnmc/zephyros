//
//  GLWebView.h
//  GhostlabMac
//
//  Created by Matthias Christen on 14.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import <WebKit/WebKit.h>
#import "base/webview/webview_extension.h"


@interface ZPYWebView : WebView

@property ClientExtensionHandlerPtr extension;

#ifndef APPSTORE
- (void) setScriptDebugDelegate: (id) delegate;
#endif

@end
