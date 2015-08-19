#include "lib/winsparkle/winsparkle.h"
#include "native_extensions/updater.h"


namespace Zephyros {
namespace UpdaterUtil {

JavaScript::Object GetSettings()
{
    
    JavaScript::Object settings = JavaScript::CreateObject();
    settings->SetBool("autoCheck", win_sparkle_get_automatic_check_for_updates() != 0);
    settings->SetInt("frequency", win_sparkle_get_update_check_interval());
    
    return settings;
}
    
void SetSettings(JavaScript::Object settings)
{
    bool autoCheck = settings->HasKey("autoCheck") ? settings->GetBool("autoCheck") : true;
    int frequency = settings->HasKey("frequency") ? settings->GetInt("frequency") : 24 * 3600;

	win_sparkle_set_automatic_check_for_updates(autoCheck ? 1 : 0);
	win_sparkle_set_update_check_interval(frequency);
}

} // namespace UpdaterUtil
} // namespace Zephyros
