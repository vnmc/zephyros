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


#import "native_extensions/image_util_mac.h"
#import "util/NSData+Base64.h"


namespace Zephyros {
namespace ImageUtil {

String NSImageToBase64EncodedPNG(NSImage* image)
{
    [image lockFocus];
    NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, image.size.width, image.size.height)];
    [image unlockFocus];
    
    NSData* data = [bitmapRep representationUsingType: NSPNGFileType properties: [[NSDictionary alloc] init]];
    if (data == nil)
        return "";
    
    String imgDataUrl = "data:image/png;base64,";
    imgDataUrl.append([[data base64EncodedString] UTF8String]);
    
    //[bitmapRep release];
    
    return imgDataUrl;
}
    
NSImage* Base64EncodedPNGToNSImage(String imageDataUrl, NSSize size)
{
    String mime = "data:image/png;base64,";
    if (imageDataUrl.substr(0, mime.length()) != mime)
        return nil;
    
    NSData* data = [NSData dataFromBase64String: [NSString stringWithUTF8String: imageDataUrl.substr(mime.length()).c_str()]];
    if (data == nil)
        return nil;
        
    NSImage* image = [[NSImage alloc] initWithData: data];
    
    // resize the image if needed
    NSImage* ret = image;
    NSSize imgSize = image.size;
    if (size.width > 0 && size.height > 0 && size.width != imgSize.width && size.height != imgSize.height)
    {
        ret = [[NSImage alloc] initWithSize: size];
        [ret lockFocus];
        [image setSize: size];
        [NSGraphicsContext currentContext].imageInterpolation = NSImageInterpolationHigh;
        [image drawAtPoint: NSZeroPoint fromRect: NSMakeRect(0, 0, imgSize.width, imgSize.height) operation: NSCompositeCopy fraction: 1];
        [ret unlockFocus];
    }
    
    return ret;
}
    
String ConvertImageToBase64EncodedPNG(String base64EncodedImage)
{
    NSData* data = [NSData dataFromBase64String: [NSString stringWithUTF8String: base64EncodedImage.c_str()]];
    NSImage* image = [[NSImage alloc] initWithData: data];
    return NSImageToBase64EncodedPNG(image);
}
    
} // namespace ImageUtil
} // namespace Zephyros
