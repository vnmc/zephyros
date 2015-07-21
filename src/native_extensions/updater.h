//
//  updater.h
//  Ghostlab
//
//  Created by Matthias Christen on 7.10.13.
//
//

#include "base/types.h"


#ifndef APPSTORE

namespace Zephyros {
namespace UpdaterUtil {

JavaScript::Object GetSettings();
void SetSettings(JavaScript::Object settings);

} // namespace UpdaterUtil
} // namespace Zephyros

#endif