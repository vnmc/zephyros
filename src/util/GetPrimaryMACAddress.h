//
//  GetPrimaryMACAddress.h
//  Ghostlab
//
//  Created by Matthias Christen on 08.03.13.
//  Copyright (c) 2013 Vanamco GmbH. All rights reserved.
//

#ifndef Ghostlab_GetPrimaryMACAddress_h
#define Ghostlab_GetPrimaryMACAddress_h

#include <vector>
#include "base/types.h"

std::vector<String> getMACAddresses(bool primaryOnly);

#endif
