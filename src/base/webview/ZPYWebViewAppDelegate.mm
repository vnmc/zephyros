//
//  ZephyrosAppDelegate.m
//  Zephyros
//
//  Created by Matthias Christen on 08.07.15.
//  Copyright (c) 2015 Vanamco AG. All rights reserved.
//

#import <objc/runtime.h>

#import <JavaScriptCore/JavaScriptCore.h>
#import <SystemConfiguration/SystemConfiguration.h>

#import "base/zephyros_impl.h"
#import "base/types.h"
#import "base/webview/ZPYWebViewAppDelegate.h"

#import "components/ZPYMenuItem.h"

#import "native_extensions/custom_url_manager.h"
#import "native_extensions/path.h"

#import "native_extensions/native_extensions.h"


#ifndef APPSTORE

@interface WebScriptCallFrame
- (id) exception;
- (id) caller;
- (NSString*) functionName;
@end

#endif


JSContextRef g_ctx = NULL;


@implementation ZPYWebViewAppDelegate

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
    NSBundle* bundle = [NSBundle mainBundle];
    
    _view.extension = m_extension;
    
    // set this class as the web view's frame load delegate
    // we will then be notified when the scripting environment
    // becomes available in the page
    _view.frameLoadDelegate = self;
    _view.UIDelegate = self;
    
#ifndef APPSTORE
    [_view setScriptDebugDelegate: self];
#endif
    
    // construct a user agent string
    // format:
    // <app-name> <version>; <os>[/<os-supplement>]; <language>
    NSString* bundleId = [bundle bundleIdentifier];
    _view.customUserAgent = [NSString stringWithFormat: @"%s %@; Mac OS X/%@; %@",
        Zephyros::GetAppName(),
        [bundle objectForInfoDictionaryKey: @"CFBundleShortVersionString"],
        [bundleId substringFromIndex: [bundleId rangeOfString: @"." options: NSBackwardsSearch].location + 1],
        [[NSLocale preferredLanguages] objectAtIndex: 0]];
    
    // set the GUI URL
    BOOL useLocalhost = NO;
    if (useLocalhost)
        _view.mainFrameURL = @"http://localhost:8005/";
    else
    {
        NSMutableString *url = [NSMutableString stringWithString: [bundle resourcePath]];
        [url appendString: @"/app/index.html"];
        _view.mainFrameURL = url;
    }
    
#ifndef APPSTORE
    if (Zephyros::GetLicenseManager())
    {
        Zephyros::GetLicenseManager()->Start();
    
        if (!Zephyros::GetLicenseManager()->CanStartApp())
            [NSApp terminate: self];
    }
    
    // create and initialize the updater
    self.updater = [[SUUpdater alloc] init];
    self.updater.feedURL = [NSURL URLWithString: [NSString stringWithUTF8String: Zephyros::GetUpdaterURL()]];
#endif
    
    [self createMenuItems];
    self.window.isVisible = YES;
    
    // register ourselves as delegate for the notification center
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate: self];
}

//
// This is called as soon as the script environment is ready in the webview
//
- (void) webView: (WebView*) sender didClearWindowObject: (WebScriptObject*) windowScriptObject forFrame: (WebFrame*) frame
{
    if (!m_nativeExtensionsAdded)
    {
        g_ctx = _view.mainFrame.globalContext;
        if (Zephyros::GetNativeExtensions())
            Zephyros::GetNativeExtensions()->AddNativeExtensions(m_extension);
        
        m_nativeExtensionsAdded = true;
        
        // add paths/URLs that have been dragged onto the dock icon
        ZPYAppDelegate* me = self;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(),
        ^{
            [me addLaunchPaths];
        });
    }
    
    // add the controller to the script environment
    // the "app" object will now be available to JavaScript
    [windowScriptObject setValue: self forKey: @"app"];
}

- (void) webView: (WebView*) sender runJavaScriptAlertPanelWithMessage: (NSString*) message initiatedByFrame: (WebFrame*) frame
{
    NSLog(@"%@", message);
}

- (NSArray*) webView: (WebView*) sender contextMenuItemsForElement: (NSDictionary*) element defaultMenuItems: (NSArray*) defaultMenuItems
{
    // disable right-click context menu
    return nil;
}


#ifndef APPSTORE

- (void) webView: (WebView*) webView exceptionWasRaised: (WebScriptCallFrame*) frame sourceId: (int) sid line: (int) line forWebFrame: (WebFrame*) webFrame
{
    [[webFrame windowObject] setValue: [frame exception] forKey: @"____frame_exception____"];
    NSLog(@"JS Exception: %@:%d %@\n%@",
          [frame functionName],
          line,
          [[webFrame windowObject] evaluateWebScript: @"____frame_exception____.toString()"],
          [[webFrame windowObject] evaluateWebScript: @"____frame_exception____.stack"]
          );
    
    [[webFrame windowObject] setValue: nil forKey:@"____frame_exception____"];
}

#endif


- (id) invokeUndefinedMethodFromWebScript: (NSString*) name withArguments: (NSArray*) args
{
    DEBUG_LOG(@"Invoking Native Function %@ (%ld args)", name, [args count]);
    
    Zephyros::JavaScript::Array params = Zephyros::JavaScript::CreateArray();
    for (int i = 0; i < [args count]; ++i)
    {
        id arg = args[i];
        
        if (arg == nil || [arg isKindOfClass: [WebUndefined class]])
        {
            // undefined
            DEBUG_LOG(@"arg %d -> undefined", i);
        }
        else if ([arg isKindOfClass: [NSNumber class]])
        {
            DEBUG_LOG(@"arg %d -> (double) %f", i, [arg doubleValue]);
            params->SetDouble(i, [arg doubleValue]);
        }
        else if ([arg isKindOfClass: [NSString class]])
        {
            DEBUG_LOG(@"arg %d -> (string) %s", i, [arg UTF8String]);
            params->SetString(i, String([arg UTF8String]));
        }
        else if ([arg isKindOfClass: [NSNull class]])
        {
            DEBUG_LOG(@"arg %d -> null", i);
            params->SetNull(i);
        }
        else if ([arg isKindOfClass: [WebScriptObject class]])
        {
            DEBUG_LOG(@"arg %d -> object", i);
            JSObjectRef argObj = [(WebScriptObject*) arg JSObject];
            
            if (Zephyros::JavaScript::IsArray(argObj))
            {
                // arg is an array
                DEBUG_LOG(@"<array>");
                params->SetList(i, Zephyros::JavaScript::CreateArray(argObj));
            }
            else if (JSObjectIsFunction(g_ctx, argObj))
            {
                DEBUG_LOG(@"<function>");
                params->SetFunction(i, argObj);
            }
            else
            {
#ifdef DEBUG_LOGGING_ON
                NSLog(@"<object>");
                JavaScript::Object o =JavaScript::CreateObject(argObj);
                JavaScript::KeyList keys;
                o->GetKeys(keys);
                for (String k : keys)
                    NSLog(@"* %s", k.c_str());
                
                params->SetDictionary(i, o);
#else
                params->SetDictionary(i, Zephyros::JavaScript::CreateObject(argObj));
#endif
            }
        }
    }
    
    // invoke the function
    m_extension->InvokeFunction(String([name UTF8String]), params);
    
    return nil;
}

@end
