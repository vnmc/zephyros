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


#import "base/app.h"
#import "base/ZPYAppDelegate.h"


#ifdef USE_CEF
extern CefRefPtr<Zephyros::ClientHandler> g_handler;
#endif


namespace Zephyros {
namespace App {
    
    
String GetUserAgent()
{
    // construct a user agent string
    // format:
    // <app-name> <version>; <os>[/<os-supplement>]; <language>

    NSBundle* bundle = [NSBundle mainBundle];
    NSString* bundleId = [bundle bundleIdentifier];
    
    NSString* userAgent = [NSString stringWithFormat: @"%s %@; Mac OS X/%@; %@",
        Zephyros::GetAppName(),
        [bundle objectForInfoDictionaryKey: @"CFBundleShortVersionString"],
        [bundleId substringFromIndex: [bundleId rangeOfString: @"." options: NSBackwardsSearch].location + 1],
        [[NSLocale preferredLanguages] objectAtIndex: 0]];
    
    return String([userAgent UTF8String]);
}
    
void CloseWindow()
{
#ifdef USE_CEF
    // initiate closing the browser windows
    g_handler->CloseAllBrowsers(false);

    // kill the process if it still running after one second
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        kill([NSProcessInfo processInfo].processIdentifier, SIGTERM);
    });
#endif
}

void Quit()
{
    [NSApp terminate: [NSApp delegate]];
}
    
void QuitMessageLoop()
{
    App::Quit();
}
    
void Alert(String title, String msg, AlertStyle style)
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithUTF8String: title.c_str()];
    alert.informativeText = [NSString stringWithUTF8String: msg.c_str()];

    switch (style)
    {
    case App::AlertStyle::AlertInfo:
        alert.alertStyle = NSInformationalAlertStyle;
        break;
    case App::AlertStyle::AlertWarning:
        alert.alertStyle = NSWarningAlertStyle;
        break;
    case App::AlertStyle::AlertError:
        alert.alertStyle = NSCriticalAlertStyle;
        break;
    }
        
    [alert runModal];
}
    
void BeginWait()
{
}
    
void EndWait()
{
}

void Log(String msg)
{
    NSLog(@"%@", [NSString stringWithUTF8String: msg.c_str()]);
}
    
void SetMenuItemStatuses(MenuItemStatuses& items)
{
    [(ZPYAppDelegate*) [NSApp delegate] setMenuItemStatuses: items];
}

} // namespace App
} // namespace Zephyros
