//
//  jsbridge.h
//  GhostlabMac
//
//  Created by Matthias Christen on 12.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#ifndef __GhostlabMac__jsbridge__
#define __GhostlabMac__jsbridge__


#include "types.h"


namespace JavaScript {
    
String GetStringFromDictionary(Object dict, KeyType key);

String JSONEscape(String s);

bool HasType(int type, int expectedType);
    
} // namespace JavaScript


#endif /* defined(__GhostlabMac__jsbridge__) */
