//
//  ImageHelper.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#endif

#include "base/types.h"


namespace Zephyros {
namespace ImageUtil {

#ifdef __OBJC__
String NSImageToBase64EncodedPNG(NSImage* image);
NSImage* Base64EncodedPNGToNSImage(String imageDataUrl, NSSize size);
#endif
    
String ConvertImageToBase64EncodedPNG(String base64EncodedImage);

} // namespace ImageUtil
} // namespace Zephyros
