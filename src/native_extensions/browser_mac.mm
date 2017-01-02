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


#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "native_extensions/image_util_mac.h"
#import "native_extensions/browser.h"


namespace Zephyros {
namespace BrowserUtil {

/**
 * Returns an array of all browsers available on the system.
 */
void FindBrowsers(std::vector<Browser*>** ppBrowsers)
{
    if (*ppBrowsers != NULL)
        return;
    
    *ppBrowsers = new std::vector<Browser*>();
    
    CFStringRef defaultBrowser = LSCopyDefaultHandlerForURLScheme(CFSTR("https"));
    NSString *ownBundleId = [[NSBundle mainBundle] bundleIdentifier];
    NSString *strDefaultBrowser = (__bridge NSString*) defaultBrowser;
    
    CFArrayRef handlerApps = LSCopyAllHandlersForURLScheme(CFSTR("https"));
    CFIndex numBrowsers = CFArrayGetCount(handlerApps);
        
    for (int i = 0; i < numBrowsers; i++)
    {
        NSString *bundleID = (NSString*) CFArrayGetValueAtIndex(handlerApps, i);
            
        // exclude this app, the PBLinkHandler, Mobile Safari
        if (bundleID == nil ||
            [ownBundleId isEqualToString: bundleID] ||
            [bundleID isEqualToString: @"com.apple.PBLinkHandler"] ||
            [bundleID isEqualToString: @"com.apple.mobilesafari"])
        {
            continue;
        }

        NSString *bundlePath = [[NSWorkspace sharedWorkspace] absolutePathForAppBundleWithIdentifier: bundleID];
        if (bundlePath == nil)
            continue;
        
        NSBundle *bundle = [NSBundle bundleWithPath: bundlePath];
        if (bundle == nil)
            continue;
            
        NSString *appName = [[bundle infoDictionary] objectForKey: @"CFBundleDisplayName"];
        if (appName == nil)
            appName = [[bundle infoDictionary] objectForKey: @"CFBundleName"];
        if (appName == nil)
        {
            NSRange range = [bundleID rangeOfString: @"." options: NSBackwardsSearch];
            appName = range.location == NSNotFound ? bundleID : [bundleID substringFromIndex: range.location + 1];
        }
        
        NSString *appVersion = [[bundle infoDictionary] objectForKey: @"CFBundleShortVersionString"];

        // if "Parallels" is contained in the path, check whether the bundle identifier contains the name of a known browser
        if ([bundleID rangeOfString: @"parallels" options: NSCaseInsensitiveSearch].location != NSNotFound)
        {
            if (!BrowserUtil::IsKnownBrowser([appName UTF8String]))
                continue;
        }
        
        NSImage *image = [[NSWorkspace sharedWorkspace] iconForFile: bundlePath] ;
        
        (*ppBrowsers)->push_back(new Browser(
            [appName UTF8String],
            appVersion != nil ? [appVersion UTF8String] : "",
            [bundleID UTF8String],
            ImageUtil::NSImageToBase64EncodedPNG(image),
            [bundleID isEqualToString: strDefaultBrowser]
        ));
    }
        
    CFRelease(defaultBrowser);
    CFRelease(handlerApps);
}

bool OpenURLInBrowser(std::string strUrl, Browser* browser)
{
    // parse the URL
    NSString *urlStr = [NSString stringWithUTF8String: strUrl.c_str()];
    NSURL *url = [NSURL URLWithString: urlStr];
    if (url == nil)
    {
        // malformed URL; try to fix
        
        // maybe there are multiple hashtags?
        // create a new URL with only the last hashtag
        NSArray *parts = [urlStr componentsSeparatedByString: @"#"];
        if (parts.count > 2)
        {
            url = [NSURL URLWithString: [NSString stringWithFormat: @"%@#%@", parts[0], parts[parts.count - 1]]];
            if (url == nil)
                return false;
        }
    }
    
    // get the browser's bundle id
    NSString *bundleId = nil;
    if (browser != NULL)
        bundleId = [NSString stringWithUTF8String: browser->GetIdentifier().c_str()];
    
    return [[NSWorkspace sharedWorkspace] openURLs: @[ url ]
                           withAppBundleIdentifier: bundleId
                                           options: NSWorkspaceLaunchDefault
                    additionalEventParamDescriptor: NULL
                                 launchIdentifiers: NULL];
}

void CleanUp(std::vector<Browser*>* pBrowsers)
{
    // nothing to do...
}

} // namespace BrowserUtil
} // namespace Zephyros
