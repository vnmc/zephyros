//
//  os.h
//  Ghostlab
//
//  Created by Matthias Christen on 5.12.13.
//
//

#ifndef Ghostlab_os_util_h
#define Ghostlab_os_util_h


#include "base/types.h"


#define CONTEXT_MENU_STARTID 90000


namespace Zephyros {
namespace OSUtil {

String GetOSVersion();
String GetComputerName();
String GetUserName();
String GetHomeDirectory();
    
void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd);

long CreateContextMenu(JavaScript::Array menuItems);
String ShowContextMenu(long nMenuHandle, int x, int y);
    
#ifdef OS_WIN
void GetWindowBorderSize(POINT* pPtBorder);
#endif

void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight);
void SetMinimumWindowSize(int width, int height);
    
void DisplayNotification(String title, String details);
    
void RequestUserAttention();
    
void CopyToClipboard(String text);

void CleanUp();

#ifdef OS_WIN
BOOL IsWin7OrLater();
BOOL IsWin8OrLater();
BOOL IsWinVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion);
#endif

} // namespace OSUtil
} // namespace Zephyros

#endif // Ghostlab_os_util_h