//
//  os_util_mac.mm
//  Ghostlab
//
//  Created by Matthias Christen on 05.12.13.
//
//

#import "base/types.h"


namespace Zephyros {
namespace OSUtil {
    
String GetOSVersion()
{
    return TEXT("");
}

String GetUserName()
{
    return TEXT("");
}
    
String GetHomeDirectory()
{
    return TEXT("");
}
    
String GetComputerName()
{
    return TEXT("");
}
 
void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
}
 
void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
}
    
    
void SetMinimumWindowSize(int width, int height)
{
}
    
    
void DisplayNotification(String title, String details)
{
}
    
void RequestUserAttention()
{
}
    
long CreateContextMenu(JavaScript::Array menuItems)
{
    return 0;
}
    
String ShowContextMenu(long nMenuHandle, int x, int y)
{
    return TEXT("");
}
    
void CopyToClipboard(String text)
{
}

void CleanUp()
{
}

} // namespace OSUtil
} // namespace Zephyros

