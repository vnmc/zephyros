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

} // namespace UpdaterUtil
} // namespace Zephyros
