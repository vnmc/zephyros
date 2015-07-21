//
//  ImageHelper.m
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#include "native_extensions/image_util_mac.h"
#include "util/NSData+Base64.h"


namespace Zephyros {
namespace ImageUtil {

String NSImageToBase64EncodedPNG(NSImage* image)
{
    [image lockFocus];
    NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, image.size.width, image.size.height)];
    [image unlockFocus];
    
    NSData* data = [bitmapRep representationUsingType: NSPNGFileType properties: nil];
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
