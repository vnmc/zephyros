//
// Copyright (C) 2013-2014 Vanamco AG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#import <JavaScriptCore/JavaScriptCore.h>
#import "AppDelegate.h"

#import "GLMenuItem.h"

#import "native_extensions.h"


// if this is not an appstore build, use the private WebScriptCallFrame API
// to get more information about JavaScript exceptions
#if APPSTORE == 0

@interface WebScriptCallFrame
- (id) exception;
- (id) caller;
- (NSString*) functionName;
@end

#endif


// The global JavaScript Core context
JSContextRef g_ctx = NULL;


@implementation AppDelegate

- (id) init
{
    self = [super init];
    
    m_extension = new ClientExtensionHandler();
    
    return self;
}

- (void) dealloc
{
    delete m_extension;
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) sender
{
    return NSTerminateNow;
}

//
// Don't quit the application when the window is closed.
//
- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) sender
{
    return NO;
}

//
// Restore the window when the dock icon is clicked.
//
- (BOOL) applicationShouldHandleReopen: (NSApplication*) sender hasVisibleWindows: (BOOL) flag
{
    if (flag)
        [self.window orderFront: self];
    else
        [self.window makeKeyAndOrderFront: self];
    
    return YES;
}

- (void) applicationDidFinishLaunching: (NSNotification*) aNotification
{
    NSBundle* bundle = [NSBundle mainBundle];
    
    _view.extension = m_extension;
    
    // set this class as the web view's frame load delegate
    // we will then be notified when the scripting environment
    // becomes available in the page
    _view.frameLoadDelegate = self;
    _view.UIDelegate = self;
#if APPSTORE == 0
    [_view setScriptDebugDelegate: self];
#endif

    // set the GUI URL
    NSMutableString *url = [NSMutableString stringWithString: [bundle resourcePath]];
    [url appendString: @"/app/index.html"];
    _view.mainFrameURL = url;

    _window.isVisible = YES;
}

//
// This is called as soon as the script environment is ready in the webview
//
- (void) webView: (WebView*) sender didClearWindowObject: (WebScriptObject*) windowScriptObject forFrame: (WebFrame*) frame
{
    g_ctx = _view.mainFrame.globalContext;
    AddNativeExtensions(m_extension);

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


#if APPSTORE == 0

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
    JavaScript::Array params = JavaScript::CreateArray();
    
    for (int i = 0; i < [args count]; ++i)
    {
        id arg = args[i];
        
        if (arg == nil || [arg isKindOfClass: [WebUndefined class]])
            ; // undefined
        else if ([arg isKindOfClass: [NSNumber class]])
            params->SetDouble(i, [arg doubleValue]);
        else if ([arg isKindOfClass: [NSString class]])
            params->SetString(i, String([arg UTF8String]));
        else if ([arg isKindOfClass: [NSNull class]])
            params->SetNull(i);
        else if ([arg isKindOfClass: [WebScriptObject class]])
        {
            JSObjectRef argObj = [(WebScriptObject*) arg JSObject];
            
            if (JavaScript::IsArray(argObj))
            {
                // arg is an array
                params->SetList(i, JavaScript::CreateArray(argObj));
            }
            else if (JSObjectIsFunction(g_ctx, argObj))
                params->SetFunction(i, argObj);
            else
                params->SetDictionary(i, JavaScript::CreateObject(argObj));
        }
    }
    
    // invoke the function
    m_extension->InvokeFunction(String([name UTF8String]), params);
    
    return nil;
}

//
// Invoked when a menu has been clicked.
//
- (IBAction) performClick: (id) sender
{
    NSString *commandId = [sender valueForKey: @"commandId"];
    if (commandId == nil || [commandId isEqualToString: @""])
        return;
    
    JavaScript::Array args = JavaScript::CreateArray();
    args->SetString(0, String([commandId UTF8String]));
    m_extension->InvokeCallbacks("onMenuCommand", args);
}

@end
