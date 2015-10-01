/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 *******************************************************************************/


#ifndef APPSTORE

#import <Foundation/Foundation.h>
#import <Sparkle/SUUpdater.h>

#import "base/types.h"
#import "native_extensions/updater.h"


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
