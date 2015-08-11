#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>

#include "native_extensions/image_util_mac.h"
#include "native_extensions/browser.h"


namespace Zephyros {
namespace BrowserUtil {

//
// Returns an array of all browsers available on the system.
//
void FindBrowsers(std::vector<Browser*>& browsers)
{
    CFStringRef defaultBrowser = LSCopyDefaultHandlerForURLScheme(CFSTR("https"));
    NSString *ownBundleId = [[NSBundle mainBundle] bundleIdentifier];
    
#if __has_feature(objc_arc)
    NSString *strDefaultBrowser = (__bridge NSString*) defaultBrowser;
#else
    NSString *strDefaultBrowser = (NSString*) defaultBrowser;
#endif
    
    CFArrayRef handlerApps = LSCopyAllHandlersForURLScheme(CFSTR("https"));
    CFIndex numBrowsers = CFArrayGetCount(handlerApps);
        
    for (int i = 0; i < numBrowsers; i++)
    {
        NSString *bundleID = (NSString*) CFArrayGetValueAtIndex(handlerApps, i);
            
        // exclude Ghostlab
        if ([ownBundleId isEqualToString: bundleID])
            continue;
            
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
        
        browsers.push_back(new Browser(
            [appName UTF8String],
            appVersion != nil ? [appVersion UTF8String] : TEXT(""),
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
    if (browser == NULL)
        return false;
    
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
    NSString *bundleId = [NSString stringWithUTF8String: browser->GetIdentifier().c_str()];
    if (bundleId == nil)
        return false;
    
    return [[NSWorkspace sharedWorkspace] openURLs: @[ url ]
                           withAppBundleIdentifier: bundleId
                                           options: NSWorkspaceLaunchDefault
                    additionalEventParamDescriptor: NULL
                                 launchIdentifiers: NULL];
}

void CleanUp(std::vector<Browser*>& browsers)
{
    // nothing to do...
}

} // namespace BrowserUtil
} // namespace Zephyros
