#ifndef APPSTORE

#include <Foundation/Foundation.h>
#include <Sparkle/SUUpdater.h>

#include "native_extensions/updater.h"

namespace Zephyros {
namespace UpdaterUtil {

JavaScript::Object GetSettings()
{
    SUUpdater *updater = [SUUpdater sharedUpdater];
    
    JavaScript::Object settings = JavaScript::CreateObject();
    settings->SetBool("autoCheck", [updater automaticallyChecksForUpdates] == YES);
    settings->SetInt("frequency", (int) [updater updateCheckInterval]);
    settings->SetBool("autoDownload", [updater automaticallyDownloadsUpdates] == YES);
    
    return settings;
}
    
void SetSettings(JavaScript::Object settings)
{
    SUUpdater *updater = [SUUpdater sharedUpdater];

    bool autoCheck = settings->HasKey("autoCheck") ? settings->GetBool("autoCheck") : true;
    int frequency = settings->HasKey("frequency") ? settings->GetInt("frequency") : 24 * 3600;
    bool autoDownload = settings->HasKey("autoDownload") ? settings->GetBool("autoDownload") : false;
    
    [updater setAutomaticallyChecksForUpdates: autoCheck];
    [updater setUpdateCheckInterval: (NSTimeInterval) frequency];
    [updater setAutomaticallyDownloadsUpdates: autoDownload];
}
    
} // namespace UpdaterUtil
} // namespace Zephyros

#endif