/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#import <WebKit/WebKit.h>

#ifdef USE_CEF
#import "base/cef/client_handler.h"
#import "base/cef/extension_handler.h"
#endif

#import "zephyros.h"
#import "native_extensions.h"

#import "native_extensions/pageimage.h"
#import "native_extensions/image_util_mac.h"
#import "native_extensions/path.h"


#ifdef USE_CEF
extern CefRefPtr<Zephyros::ClientHandler> g_handler;
#endif

#ifdef USE_WEBVIEW
extern JSContextRef g_ctx;
#endif


@interface ScreenshotQueueItem : NSObject

@property CallbackId callback;
@property NSString *url;
@property int width;

- (id) initWithCallback: (CallbackId) callback url: (NSString*) url width: (int) width;

@end


@interface ScreenshotWebViewFrameLoadDelegate : NSObject <WebFrameLoadDelegate>

@property NSMutableArray* queue;
- (void) addToQueue: (CallbackId) callback url: (String) url width: (int) width;

@end


static WebView *g_screenshotWebView = nil;
static ScreenshotWebViewFrameLoadDelegate *g_loadDelegate = nil;


@implementation ScreenshotQueueItem

- (id) initWithCallback: (CallbackId) callback url: (NSString*) url width: (int) width
{
    self = [super init];
    
    self.callback = callback;
    self.url = url;
    self.width = width;
    
    return self;
}

@end


@implementation ScreenshotWebViewFrameLoadDelegate

- (id) init
{
    self = [super init];
    self.queue = [[NSMutableArray alloc] init];
    return self;
}

- (void) addToQueue: (CallbackId) callback url: (String) url width: (int) width
{
    [self.queue addObject: [[ScreenshotQueueItem alloc] initWithCallback: callback
                                                                     url: [NSString stringWithUTF8String: url.c_str()]
                                                                   width: width]];
#ifdef USE_WEBVIEW
    JSValueProtect(g_ctx, callback);
#endif
    
    // start loading if the queue contains only one item
    if (self.queue.count == 1)
    g_screenshotWebView.mainFrameURL = [NSString stringWithUTF8String: url.c_str()];
}

- (void) webView: (WebView*) view didFinishLoadForFrame: (WebFrame*) frame
{
    ScreenshotWebViewFrameLoadDelegate *me = self;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(),
    ^{
        if (me.queue.count <= 0)
            return;
                       
        ScreenshotQueueItem *item = [me.queue objectAtIndex: 0];
        [me.queue removeObjectAtIndex: 0];
                       
        // get the view's contents as an image
        NSBitmapImageRep *imageRep = [view bitmapImageRepForCachingDisplayInRect: [view frame]];
        [view cacheDisplayInRect: [view frame] toBitmapImageRep: imageRep];
                       
        // create an NSImage
        NSImage *image = [[NSImage alloc] initWithSize: imageRep.size];
        [image addRepresentation: imageRep];
                       
        // resize the image
        int w = item.width;
        if (w > Zephyros::PageImage::ImageWidth)
            w = Zephyros::PageImage::ImageWidth;
        
        NSSize size = NSMakeSize(w, (int) (((long) w * Zephyros::PageImage::ImageHeight) / Zephyros::PageImage::ImageWidth));
        NSImage *imageResized = image;
        if (w < Zephyros::PageImage::ImageWidth)
        {
            imageResized = [[NSImage alloc] initWithSize: size];
            [imageResized lockFocus];
            [image setSize: size];
            [NSGraphicsContext currentContext].imageInterpolation = NSImageInterpolationHigh;
            [image drawAtPoint: NSZeroPoint fromRect: NSMakeRect(0, 0, imageRep.size.width, imageRep.size.height) operation: NSCompositeCopy fraction: 1];
            [imageResized unlockFocus];
        }
        
        // create a data URL from the resized image
        String imgData = Zephyros::ImageUtil::NSImageToBase64EncodedPNG(imageResized);
                       
        // invoke the callback with the result data URL
#ifdef USE_CEF
        Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
        args->SetString(0, imgData);
        g_handler->GetClientExtensionHandler()->InvokeCallback(item.callback, args);
#endif
        
#ifdef USE_WEBVIEW
        JSStringRef retData = JSStringCreateWithUTF8CString(imgData.c_str());
        JSValueRef arg = JSValueMakeString(g_ctx, retData);
        JSObjectCallAsFunction(g_ctx, item.callback, NULL, 1, &arg, NULL);
        JSStringRelease(retData);
                       
        JSValueUnprotect(g_ctx, item.callback);
#endif
        
        if (me.queue.count > 0)
        {
            ScreenshotQueueItem *nextItem = [me.queue objectAtIndex: 0];
            g_screenshotWebView.mainFrameURL = nextItem.url;
        }
        else
            g_screenshotWebView.mainFrameURL = @"about:blank";
    });
}

@end


namespace Zephyros {
namespace PageImage {

void GetPageImageForURL(CallbackId callback, String url, int width)
{
    if (g_loadDelegate == nil)
    g_loadDelegate = [[ScreenshotWebViewFrameLoadDelegate alloc] init];
    
    if (g_screenshotWebView == nil)
    {
        g_screenshotWebView = [[WebView alloc] initWithFrame: NSMakeRect(0, 0, ImageWidth, ImageHeight)];
        g_screenshotWebView.shouldUpdateWhileOffscreen = YES;
        g_screenshotWebView.frameLoadDelegate = g_loadDelegate;
    }
    
    [g_loadDelegate addToQueue: callback url: url width: width];
}
    
} // namespace PageImage
} // namespace Zephyros

