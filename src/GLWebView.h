//
//  GLWebView.h
//  GhostlabMac
//
//  Created by Matthias Christen on 14.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#import <WebKit/WebKit.h>
#import "webview_extension.h"


@interface GLWebView : WebView

@property ClientExtensionHandlerPtr extension;

#if APPSTORE == 0
- (void) setScriptDebugDelegate: (id) delegate;
#endif

@end
