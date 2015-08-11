#ifndef APPSTORE

#include <Foundation/Foundation.h>
#include <Sparkle/SUUpdater.h>

#include "native_extensions/updater.h"

namespace Zephyros {
namespace UpdaterUtil {

JavaScript::Object GetSettings()
{
    return JavaScript::CreateObject();
}
    
void SetSettings(JavaScript::Object settings)
{
}
    
} // namespace UpdaterUtil
} // namespace Zephyros

#endif