/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "lib/winsparkle/winsparkle.h"
#include "native_extensions/updater.h"


namespace Zephyros {
namespace UpdaterUtil {

JavaScript::Object GetSettings()
{
    JavaScript::Object settings = JavaScript::CreateObject();

    const TCHAR* szUpdaterURL = Zephyros::GetUpdaterURL();
    if (szUpdaterURL != NULL && szUpdaterURL[0] != TCHAR('\0'))
    {
        settings->SetBool("autoCheck", win_sparkle_get_automatic_check_for_updates() != 0);
        settings->SetInt("frequency", win_sparkle_get_update_check_interval());
    }
    else
    {
        settings->SetBool("autoCheck", false);
        settings->SetInt("frequency", 0);
    }
    
    return settings;
}
    
void SetSettings(JavaScript::Object settings)
{
    const TCHAR* szUpdaterURL = Zephyros::GetUpdaterURL();
    if (szUpdaterURL == NULL || szUpdaterURL[0] == TCHAR('\0'))
        return;

    bool autoCheck = settings->HasKey("autoCheck") ? settings->GetBool("autoCheck") : true;
    int frequency = settings->HasKey("frequency") ? settings->GetInt("frequency") : 24 * 3600;

    win_sparkle_set_automatic_check_for_updates(autoCheck ? 1 : 0);
    win_sparkle_set_update_check_interval(frequency);
}

void CheckForUpdates()
{
	win_sparkle_check_update_with_ui();
}

} // namespace UpdaterUtil
} // namespace Zephyros
