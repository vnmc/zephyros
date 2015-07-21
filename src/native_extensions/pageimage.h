//
//  pageimage_mac.h
//  GhostlabMac
//
//  Created by Matthias Christen on 29.06.14.
//  Copyright (c) 2014 Vanamco AG. All rights reserved.
//

#include "base/types.h"


namespace Zephyros {
namespace PageImage {
    
static const int ImageWidth = 1024;
static const int ImageHeight = 768;

void GetPageImageForURL(CallbackId callback, String url, int width);

} // namespace PageImage
} // namespace Zephyros