//
//  jsbridge.h
//  Zephyros
//
//  Created by Matthias Christen on 12.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#ifndef __Zephyros__jsbridge__
#define __Zephyros__jsbridge__


#include "types.h"


namespace JavaScript {
    
String GetStringFromDictionary(Object dict, KeyType key);

String JSONEscape(String s);

bool HasType(int type, int expectedType);
    
} // namespace JavaScript


#endif /* defined(__Zephyros__jsbridge__) */
