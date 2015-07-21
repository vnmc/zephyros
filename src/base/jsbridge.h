//
//  jsbridge.h
//  GhostlabMac
//
//  Created by Matthias Christen on 12.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#ifndef __Zephyros__jsbridge__
#define __Zephyros__jsbridge__


#include "base/types.h"


namespace Zephyros {
namespace JavaScript {
    
String GetStringFromDictionary(Object dict, KeyType key);

String JSONEscape(String s);

bool HasType(int type, int expectedType);

} // namespace JavaScript
} // namespace Zephyros


#endif /* defined(__Zephyros__jsbridge__) */
