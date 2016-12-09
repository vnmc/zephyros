/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#import <objc/runtime.h>

#import <JavaScriptCore/JavaScriptCore.h>
#import <SystemConfiguration/SystemConfiguration.h>

#import "base/types.h"
#import "base/logging.h"
#import "base/webview/ZPYWebViewAppDelegate.h"

#import "components/ZPYMenuItem.h"

#import "native_extensions/custom_url_manager.h"
#import "native_extensions/path.h"

#import "zephyros.h"
#import "native_extensions.h"


#ifndef APPSTORE

@interface WebScriptCallFrame
- (id) exception;
- (id) caller;
- (NSString*) functionName;
@end

#endif


JSContextRef g_ctx = NULL;


@implementation ZPYWebViewAppDelegate

- (id) init
{
    self = [super init];
    m_extension = new Zephyros::ClientExtensionHandler();    
    return self;
}

- (void) dealloc
{
    delete m_extension;
}

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
    [super applicationDidFinishLaunching: notification];

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
    _view.customUserAgent = [NSString stringWithUTF8String: Zephyros::App::GetUserAgent().c_str()];
    
    // set the GUI URL
    NSMutableString *url = [NSMutableString stringWithString: [[NSBundle mainBundle] resourcePath]];
    [url appendString: @"/"];
    [url appendString: [NSString stringWithUTF8String: Zephyros::GetAppURL()]];
    _view.mainFrameURL = url;
    
#ifndef APPSTORE
    if (Zephyros::GetLicenseManager())
    {
        Zephyros::GetLicenseManager()->Start();
    
        if (!Zephyros::GetLicenseManager()->CanStartApp())
            [NSApp terminate: self];
    }
    
    // create and initialize the updater
    if (Zephyros::GetUpdaterURL() && _tcslen(Zephyros::GetUpdaterURL()) > 0)
    {
        self.updater = [[SUUpdater alloc] init];
        self.updater.feedURL = [NSURL URLWithString: [NSString stringWithUTF8String: Zephyros::GetUpdaterURL()]];
    }
#endif
    
    self.window.isVisible = YES;
    
    // register ourselves as delegate for the notification center
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate: self];
}

- (void) awakeFromNib
{
    if (self.window != nil)
    {
        self.window.title = [NSString stringWithUTF8String: Zephyros::GetAppName()];
        self.window.frameAutosaveName = [NSString stringWithFormat: @"%s:MainWindow", Zephyros::GetAppName()];
    }
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) sender
{
    return m_extension->InvokeCallbacks(TEXT("onAppTerminating"), Zephyros::JavaScript::CreateArray()) ?
        NSTerminateNow :
        NSTerminateCancel;
}

//
// This is called as soon as the script environment is ready in the webview
//
- (void) webView: (WebView*) sender didClearWindowObject: (WebScriptObject*) windowScriptObject forFrame: (WebFrame*) frame
{
    if (!Zephyros::GetNativeExtensions()->GetNativeExtensionsAdded())
    {
        g_ctx = _view.mainFrame.globalContext;
        if (Zephyros::GetNativeExtensions())
            Zephyros::GetNativeExtensions()->AddNativeExtensions(m_extension);

        Zephyros::GetNativeExtensions()->SetNativeExtensionsAdded();
        
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
    for (unsigned int i = 0; i < [args count]; ++i)
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
                if (Zephyros::UseLogging())
                {
                    NSLog(@"<object>");
                    Zephyros::JavaScript::Object o = Zephyros::JavaScript::CreateObject(argObj);
                    Zephyros::JavaScript::KeyList keys;
                    o->GetKeys(keys);
                    for (String k : keys)
                        NSLog(@"* %s", k.c_str());
                
                    params->SetDictionary(i, o);
                }
                else
                    params->SetDictionary(i, Zephyros::JavaScript::CreateObject(argObj));
            }
        }
    }
    
    // invoke the function
    m_extension->InvokeFunction(String([name UTF8String]), params);
    
    return nil;
}

@end
