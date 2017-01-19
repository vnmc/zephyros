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


#include <vector>
#include <map>

#include <ShlObj.h>

#include "base/app.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "native_extensions/os_util.h"
#include "native_extensions/image_util_win.h"
#include "native_extensions/process_manager.h"

#include <strsafe.h>


#define FILENAME TEXT("Kernel32.dll")
#define MENU_BUF_SIZE 4096
#define MAX_ACCELS 256


extern HINSTANCE g_hInst;
extern CefRefPtr<Zephyros::ClientHandler> g_handler;

extern int g_nMinWindowWidth;
extern int g_nMinWindowHeight;


namespace Zephyros {
namespace OSUtil {

/**
 * Get the Windows version.
 *
 * According to the MSDN:
 * To obtain the full version number for the operating system, call the GetFileVersionInfo
 * function on one of the system DLLs, such as Kernel32.dll, then call VerQueryValue to obtain
 * the \\StringFileInfo\\<lang><codepage>\\ProductVersion subblock of the file version information.
 */
String GetOSVersion()
{
    StringStream ssVersion;
    ssVersion << TEXT("Win-");

    DWORD dwHandle;
    DWORD dwInfoSize = GetFileVersionInfoSize(FILENAME, &dwHandle);
    if (dwInfoSize > 0)
    {
        BYTE* pData = new BYTE[dwInfoSize];
        if (GetFileVersionInfo(FILENAME, 0, dwInfoSize, pData))
        {
            struct {
                WORD wLanguage;
                WORD wCodePage;
            } * lpTranslate = NULL;
            UINT cbTranslate = 0;

            if (VerQueryValue(pData, TEXT("\\VarFileInfo\\Translation"), (LPVOID*) &lpTranslate, &cbTranslate))
            {
                TCHAR lpszSubBlock[50];
                if (StringCchPrintf(lpszSubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage) == S_OK)
                {
                    LPVOID lpVersion;
                    UINT cbVersion = 0;
                    if (VerQueryValue(pData, lpszSubBlock, &lpVersion, &cbVersion))
                        ssVersion << String((LPCTSTR) lpVersion, cbVersion);
                }
            }
        }

        delete[] pData;
    }

    return ssVersion.str();
}

/**
 * Cf. http://msdn.microsoft.com/en-us/library/windows/desktop/bb773352(v=vs.85).aspx
 */
BOOL IsWin7OrLater()
{
    return IsWinVersionOrLater(6, 1);
}

BOOL IsWin8OrLater()
{
    return IsWinVersionOrLater(6, 2);
}

BOOL IsWinVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
    // Initialize the OSVERSIONINFOEX structure
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = dwMajorVersion;
    osvi.dwMinorVersion = dwMinorVersion;

    // Initialize the condition mask
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    // Perform the test
    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

String GetUserName()
{
    // TODO: implement
    return TEXT("");
}

String GetHomeDirectory()
{
    TCHAR szProfileFolder[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szProfileFolder);
    return szProfileFolder;
}

String GetComputerName()
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwLen = MAX_COMPUTERNAME_LENGTH;
    ::GetComputerName(szComputerName, &dwLen);
    return szComputerName;
}

void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
    // create and start a new process
    // the process manager deletes itself once the process has terminated
    ProcessManager* pMgr = new ProcessManager(callback, executableFileName, arguments, cwd);
    pMgr->Start();
}

String GetKeyName(WORD wVkCode)
{
    WORD nFlags = 2;
    if (0x20 <= wVkCode && wVkCode <= 0x2f)
        nFlags |= 1;

    // get the key name
    TCHAR szName[128];
    int nRet = GetKeyNameText(MapVirtualKey(wVkCode, MAPVK_VK_TO_VSC) << 16 | (nFlags << 24), szName, 127);
    szName[nRet] = TEXT('\0');

    // capitalize the name
    bool bPrevWasAlnum = false;
    for (int i = 0; i < nRet; ++i)
    {
        TCHAR c = szName[i];

        if (bPrevWasAlnum)
            szName[i] = tolower(c);
        if (i > 0 && c == TEXT('-'))
            szName[i] = TEXT(' ');

        bPrevWasAlnum = isalnum(c) != 0;
    }

    return szName;
}


static std::vector<HBITMAP> g_vecCreatedBitmaps;

void CreateMenuRecursive(
    BYTE* pBufMenu, DWORD& dwOffsetMenu, ACCEL* pBufAccel, DWORD& dwNumAccels, JavaScript::Array menuItems, WORD& wCurrentID,
    SIZE sizeIcon, std::map<WORD, HBITMAP>& mapImageItems, bool bIsInDemoMode)
{
    bool bPrevItemWasSeparator = false;
    int nNumItems = (int) menuItems->GetSize();

    for (int i = 0; i < nNumItems; ++i)
    {
        JavaScript::Object item = menuItems->GetDictionary(i);
        MENUITEMTEMPLATE* pTemplate = (MENUITEMTEMPLATE*) (pBufMenu + dwOffsetMenu);

        String strCaption = item->GetString(TEXT("caption"));
        if (strCaption == TEXT("-"))
        {
            // this menu item is a separator

            if (bPrevItemWasSeparator)
                continue;

            pTemplate->mtOption = (i == nNumItems - 1 ? MF_END : 0);
            // ID and mtString are 0x0000

            dwOffsetMenu += sizeof(MENUITEMTEMPLATE);
            bPrevItemWasSeparator = true;
        }
        else
        {
            bool bHasSubMenuItems = item->HasKey(TEXT("subMenuItems"));
            wchar_t* pCaption = pTemplate->mtString;
            pTemplate->mtOption = (bHasSubMenuItems ? MF_POPUP : 0) | (i == nNumItems - 1 ? MF_END : 0);

            // menu ID
            if (!bHasSubMenuItems)
            {
                WORD wCmd = 0;
                bool bHasCommand = false;

                if (item->HasKey(TEXT("systemCommandId")))
                {
                    pTemplate->mtID = wCmd = item->GetInt(TEXT("systemCommandId"));
                    bHasCommand = true;
                }
                else if (item->HasKey(TEXT("menuCommandId")))
                {
                    String strCommandId = item->GetString(TEXT("menuCommandId"));

                    // don't add licensing related item if not in demo mode
                    if (!bIsInDemoMode && (strCommandId == TEXT(MENUCOMMAND_ENTER_LICENSE) || strCommandId == TEXT(MENUCOMMAND_PURCHASE_LICENSE)))
                        continue;

                    Zephyros::SetMenuIDForCommand(strCommandId.c_str(), wCurrentID);
                    pTemplate->mtID = wCmd = wCurrentID;
                    ++wCurrentID;
                    bHasCommand = true;
                }

                // create the menu item icon
                if (bHasCommand && item->HasKey(TEXT("image")))
                {
                    String strImage = item->GetString(TEXT("image"));
                    if (strImage.length() > 0)
                    {
                        // create a bitmap from the image, which is interpreted as a base64-encoded PNG
                        HBITMAP hBmp = ImageUtil::Base64PNGDataToBitmap(strImage, sizeIcon);
                        if (hBmp)
                        {
                            g_vecCreatedBitmaps.push_back(hBmp);
                            mapImageItems[wCmd] = hBmp;
                        }
                    }
                }

                // add the accelerator key
                if (pBufAccel && bHasCommand && item->HasKey(TEXT("key")))
                {
                    int nModifiers = 0;
                    if (item->HasKey(TEXT("keyModifiers")))
                        nModifiers = item->GetInt(TEXT("keyModifiers"));

                    String strKey = item->GetString("key");
                    String strKeyLc = ToLower(strKey);

                    pBufAccel[dwNumAccels].fVirt = FVIRTKEY | ((nModifiers & 1) ? FSHIFT : 0) | ((nModifiers & 2) ? FCONTROL : 0) | ((nModifiers & 4) ? FALT : 0);
                    
                    WORD wVkCode = 0;
                    if (strKeyLc == TEXT("left"))
                        wVkCode = VK_LEFT;
                    else if (strKeyLc == TEXT("right"))
                        wVkCode = VK_RIGHT;
                    else if (strKeyLc == TEXT("up"))
                        wVkCode = VK_UP;
                    else if (strKeyLc == TEXT("down"))
                        wVkCode = VK_DOWN;
                    else if (strKeyLc == TEXT("page up") || strKeyLc == TEXT("pageup"))
                        wVkCode = VK_PRIOR;
                    else if (strKeyLc == TEXT("page down") || strKeyLc == TEXT("pagedown"))
                        wVkCode = VK_NEXT;
                    else if (strKeyLc == TEXT("home"))
                        wVkCode = VK_HOME;
                    else if (strKeyLc == TEXT("end"))
                        wVkCode = VK_END;
                    else if (strKeyLc == TEXT("insert"))
                        wVkCode = VK_INSERT;
                    else if (strKeyLc == TEXT("delete"))
                        wVkCode = VK_DELETE;
                    else if (strKeyLc == TEXT("backspace"))
                        wVkCode = VK_BACK;
                    else if (strKeyLc == TEXT("enter"))
                        wVkCode = VK_RETURN;
                    else if (strKeyLc.length() > 1 && strKeyLc.at(0) == 'f' && isdigit(strKeyLc.at(1)))
                        wVkCode = VK_F1 + _ttoi(strKey.substr(1).c_str()) - 1;
                    else
                        wVkCode = (WORD) toupper(strKey.at(0));

                    pBufAccel[dwNumAccels].key = wVkCode;
                    pBufAccel[dwNumAccels].cmd = wCmd;
                    ++dwNumAccels;

                    // add the keyboard shortcut to the menu item caption
                    strCaption.append(TEXT("\t"));
                    if (nModifiers & 1)
                        strCaption.append(GetKeyName(VK_SHIFT) + TEXT("+"));
                    if (nModifiers & 2)
                        strCaption.append(GetKeyName(VK_CONTROL) + TEXT("+"));
                    if (nModifiers & 4)
                        strCaption.append(GetKeyName(VK_MENU) + TEXT("+"));
                    strCaption.append(GetKeyName(wVkCode));
                }

                dwOffsetMenu += 2 * sizeof(WORD);
            }
            else
            {
                // popup menus don't have an ID member
                pCaption = (wchar_t*) (((BYTE*) pCaption) - sizeof(WORD));
                dwOffsetMenu += sizeof(WORD);
            }

            // caption (Windows expects a "&" before the underlined character)
            strCaption = StringReplace(StringReplace(strCaption, TEXT("&"), TEXT("&&")), TEXT("_"), TEXT("&"));
            dwOffsetMenu += (DWORD) ((strCaption.length() + 1) * sizeof(wchar_t));
            if (dwOffsetMenu >= MENU_BUF_SIZE)
                return;
            wcscpy(pCaption, strCaption.c_str());
            bPrevItemWasSeparator = false;

            if (bHasSubMenuItems)
                CreateMenuRecursive(pBufMenu, dwOffsetMenu, pBufAccel, dwNumAccels, item->GetList("subMenuItems"), wCurrentID, sizeIcon, mapImageItems, bIsInDemoMode);
        }
    }
}

bool CreateMenuInternal(JavaScript::Array menuItems, WORD wStartCommandID, bool bIsPopupMenu, HMENU* phMenu, HACCEL* phAccel = NULL)
{
    *phMenu = NULL;
    if (phAccel)
        *phAccel = NULL;

    BYTE* pBufMenu = new BYTE[MENU_BUF_SIZE];
    ZeroMemory(pBufMenu, MENU_BUF_SIZE);
    ACCEL* pBufAccel = phAccel ? new ACCEL[MAX_ACCELS] : NULL;

    ((MENUITEMTEMPLATEHEADER*) pBufMenu)->versionNumber = 0;
    ((MENUITEMTEMPLATEHEADER*) pBufMenu)->offset = 0;

    DWORD dwOffsetMenu = sizeof(MENUITEMTEMPLATEHEADER);
    DWORD dwNumAccels = 0;
    WORD wCurrentMenuID = wStartCommandID;

    // add a dummy top level menu if this is a popup menu
    if (bIsPopupMenu)
    {
        // mtOption
        *((WORD*) (pBufMenu + dwOffsetMenu)) = MF_POPUP | MF_END;
        dwOffsetMenu += sizeof(WORD);

        // mtString
        wcscpy((wchar_t*) (pBufMenu + dwOffsetMenu), TEXT("_"));
        dwOffsetMenu += 2 * sizeof(wchar_t);
    }

    // get the size for the menu icons
    SIZE sizeIcon;
    sizeIcon.cx = GetSystemMetrics(SM_CXSMICON);
    sizeIcon.cy = GetSystemMetrics(SM_CYSMICON);

    std::map<WORD, HBITMAP> mapImages;

    // create the menu structure in memory
    CreateMenuRecursive(
        pBufMenu, dwOffsetMenu, pBufAccel, dwNumAccels, menuItems, wCurrentMenuID, sizeIcon, mapImages,
        !bIsPopupMenu && Zephyros::GetLicenseManager() != NULL && Zephyros::GetLicenseManager()->IsInDemoMode()
    );

    // create the menu
    *phMenu = LoadMenuIndirect(static_cast<MENUTEMPLATE*>(pBufMenu));

    // set the menu bitmaps
    for (std::map<WORD, HBITMAP>::iterator it = mapImages.begin(); it != mapImages.end(); ++it)
        SetMenuItemBitmaps(*phMenu, it->first, MF_BYCOMMAND, it->second, it->second);

    // create the accelerator table
    if (phAccel)
        *phAccel = CreateAcceleratorTable(pBufAccel, dwNumAccels);

    // clean up
    delete[] pBufMenu;
    if (pBufAccel)
        delete[] pBufAccel;

    return true;
}

void CreateMenu(JavaScript::Array menuItems)
{
    // create and set the menu
    HMENU hMenu;
    HACCEL hAccel;
    if (!CreateMenuInternal(menuItems, 1000, false, &hMenu, &hAccel))
        return;

    // remove the old menu and set the new one
    if (hMenu)
    {
        HWND hWnd = g_handler->GetMainHwnd();
        HMENU hMenuOld = GetMenu(hWnd);
        if (hMenuOld)
            DestroyMenu(hMenuOld);

        SetMenu(hWnd, hMenu);
    }

    // create and set the accelerator table
    if (hAccel)
    {
        DestroyAcceleratorTable(g_handler->GetAccelTable());
        g_handler->SetAccelTable(hAccel);
    }
}

void RemoveMenuItem(String strCommandId)
{
    int nMenuID = Zephyros::GetMenuIDForCommand(strCommandId.c_str());
    if (nMenuID != 0)
        RemoveMenu(GetMenu(g_handler->GetMainHwnd()), nMenuID, MF_BYCOMMAND);
}


static std::vector<HMENU> g_vecCreatedMenus;
static WORD g_wCurrentPopupMenuID = CONTEXT_MENU_STARTID;

MenuHandle CreateContextMenu(JavaScript::Array menuItems)
{
    HMENU hMenu;
    if (!CreateMenuInternal(menuItems, g_wCurrentPopupMenuID, true, &hMenu))
        return (MenuHandle) NULL;

    g_vecCreatedMenus.push_back(hMenu);
    return (MenuHandle) hMenu;
}

String ShowContextMenu(MenuHandle nMenuHandle, int x, int y)
{
    HWND hWnd = App::GetMainHwnd();

    POINT pt;
    pt.x = x;
    pt.y = y;
    ClientToScreen(hWnd, &pt);

    int nRet = TrackPopupMenu(GetSubMenu((HMENU) nMenuHandle, 0), TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
    if (nRet == FALSE)
        return TEXT("");

    return Zephyros::GetMenuCommandForID(nRet);
}

void CreateTouchBar(JavaScript::Array touchBarItems)
{
    // not applicable
}

void GetWindowBorderSize(POINT* pPtBorder)
{
    RECT rectWnd, rectClient;
    HWND hWnd = App::GetMainHwnd();

    GetWindowRect(hWnd, &rectWnd);
    GetClientRect(hWnd, &rectClient);

    pPtBorder->x = rectWnd.right - rectWnd.left - rectClient.right;
    pPtBorder->y = rectWnd.bottom - rectWnd.top - rectClient.bottom;
}


int lastOriginX = INT_MIN;
int lastOriginY = INT_MIN;
int lastX = -1;
int lastY = -1;
int lastWidth = -1;
int lastHeight = -1;

void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
    HWND hWnd = App::GetMainHwnd();

    // get the window position
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hWnd, &wpl);

    // get the size of the monitor containing the window
    POINT pt;
    pt.x = wpl.rcNormalPosition.left;
    pt.y = wpl.rcNormalPosition.top;
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMonitor, &info))
    {
        // set some arbitrary values if there was an error retrieving the monitor info
        info.rcWork.right = 10000;
        info.rcWork.bottom = 10000;
    }

    // determine whether the window has moved since it was last resized
    bool windowMoved = wpl.rcNormalPosition.left != lastX || wpl.rcNormalPosition.top != lastY;

    // find the window's border sizes
    POINT ptBorder;
    GetWindowBorderSize(&ptBorder);

    if (hasWidth)
    {
        // account for the window's borders
        width += ptBorder.x;

        // update the window frame rectangle
        wpl.rcNormalPosition.right = wpl.rcNormalPosition.left + width;

        // do we need to restore the lastOriginX?
        if (lastWidth > width && lastOriginX != INT_MIN)
        {
            if (!windowMoved)
            {
                int w = wpl.rcNormalPosition.right - wpl.rcNormalPosition.left;
                wpl.rcNormalPosition.left = lastOriginX;
                wpl.rcNormalPosition.right = lastOriginX + w;
            }

            lastOriginX = INT_MIN;
        }

        // adjust the window's top-left corner if, after resizing, the window would fall off the screen
        if (wpl.rcNormalPosition.right > info.rcWork.right)
        {
            int left = info.rcWork.right - width;
            if (left < 0)
                left = 0;

            if (left != wpl.rcNormalPosition.left)
            {
                lastOriginX = wpl.rcNormalPosition.left;
                wpl.rcNormalPosition.left = left;
                wpl.rcNormalPosition.right = info.rcWork.right;
            }
        }

        // save the last width
        lastWidth = width;
    }
    else
        lastWidth = -1;

    if (hasHeight)
    {
        // account for the window's borders
        height += ptBorder.y;

        // update the window frame rectangle
        wpl.rcNormalPosition.bottom = wpl.rcNormalPosition.top + height;

        // do we need to restore the lastOriginX?
        if (lastHeight > height && lastOriginY != INT_MIN)
        {
            if (!windowMoved)
            {
                int h = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top;
                wpl.rcNormalPosition.top = lastOriginY;
                wpl.rcNormalPosition.bottom = lastOriginY + h;
            }

            lastOriginY = INT_MIN;
        }

        // adjust the window's top-left corner if, after resizing, the window would fall off the screen
        if (wpl.rcNormalPosition.bottom > info.rcWork.bottom)
        {
            int top = info.rcWork.bottom - height;
            if (top < 0)
                top = 0;

            if (top != wpl.rcNormalPosition.top)
            {
                lastOriginY = wpl.rcNormalPosition.top;
                wpl.rcNormalPosition.top = top;
                wpl.rcNormalPosition.bottom = info.rcWork.bottom;
            }
        }

        // save the last height
        lastHeight = height;
    }
    else
        lastHeight = -1;

    lastX = wpl.rcNormalPosition.left;
    lastY = wpl.rcNormalPosition.top;

    *pNewWidth = wpl.rcNormalPosition.right - wpl.rcNormalPosition.left - ptBorder.x;
    *pNewHeight = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top - ptBorder.y;

    if (hasWidth || hasHeight)
        SetWindowPlacement(hWnd, &wpl);

    // TODO: animate resizing the window
    // invoke the callback (needed when we animate; adjust the return value in extension_handler.cpp)
    // g_handler->GetClientExtensionHandler()->InvokeCallback(callback, JavaScript::CreateArray());
}


void SetMinimumWindowSize(int width, int height)
{
    g_nMinWindowWidth = width;
    g_nMinWindowHeight = height;
}


static NOTIFYICONDATA* g_pnfiNotifyIcon = NULL;
static HICON g_hIconSmall = NULL;
static HICON g_hIconLarge = NULL;

void DisplayNotification(String title, String details)
{
    bool bIconAdded = false;

    if (g_pnfiNotifyIcon == NULL)
    {
        g_pnfiNotifyIcon = new NOTIFYICONDATA;
        ZeroMemory(g_pnfiNotifyIcon, sizeof(NOTIFYICONDATA));

        int nIconID = Zephyros::GetWindowsInfo().nIconID;
        g_hIconSmall = static_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(nIconID), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
        g_hIconLarge = static_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(nIconID), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));

        g_pnfiNotifyIcon->cbSize = sizeof(NOTIFYICONDATA);
        g_pnfiNotifyIcon->hWnd = App::GetMainHwnd();
        g_pnfiNotifyIcon->uFlags = NIF_ICON | NIF_INFO | NIF_SHOWTIP;
        g_pnfiNotifyIcon->dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
        g_pnfiNotifyIcon->hIcon = g_hIconSmall;
        g_pnfiNotifyIcon->hBalloonIcon = g_hIconLarge;

        if (IsWin7OrLater())
        {
            g_pnfiNotifyIcon->uFlags |= NIF_GUID;

            // {8F4D4A24-DD64-4AC6-8EE1-296806488C73}
            static const GUID guid = { 0x8f4d4a24, 0xdd64, 0x4ac6, { 0x8e, 0xe1, 0x29, 0x68, 0x6, 0x48, 0x8c, 0x73 } };
            g_pnfiNotifyIcon->guidItem = guid;
        }
        else
            g_pnfiNotifyIcon->uID = 0;

        bIconAdded = true;
    }

    StringCchCopy(g_pnfiNotifyIcon->szInfoTitle, ARRAYSIZE(g_pnfiNotifyIcon->szInfoTitle), title.c_str());
    StringCchCopy(g_pnfiNotifyIcon->szInfo, ARRAYSIZE(g_pnfiNotifyIcon->szInfo), details.c_str());

    Shell_NotifyIcon(bIconAdded ? NIM_ADD : NIM_MODIFY, g_pnfiNotifyIcon);
}

void RequestUserAttention()
{
    FLASHWINFO flashinfo;

    flashinfo.cbSize = sizeof(FLASHWINFO);
    flashinfo.hwnd = App::GetMainHwnd();
    flashinfo.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    flashinfo.uCount = 3;
    flashinfo.dwTimeout = 0;

    FlashWindowEx(&flashinfo);
}

void CopyToClipboard(String text)
{
    size_t size = (text.length() + 1) * sizeof(TCHAR);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    HWND hWnd = App::GetMainHwnd();
    memcpy(GlobalLock(hMem), text.c_str(), size);
    GlobalUnlock(hMem);

    OpenClipboard(hWnd);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
}

void BeginDragFile(Path& path, int x, int y)
{
    // https://www.codeproject.com/Articles/840/How-to-Implement-Drag-and-Drop-Between-Your-Progra
}

void CleanUp()
{
    for (HMENU hMenu : g_vecCreatedMenus)
        DestroyMenu(hMenu);
    for (HBITMAP hBmp : g_vecCreatedBitmaps)
        DeleteObject(hBmp);

    if (g_pnfiNotifyIcon != NULL)
        Shell_NotifyIcon(NIM_DELETE, g_pnfiNotifyIcon);
    if (g_hIconSmall != NULL)
        DestroyIcon(g_hIconSmall);
    if (g_hIconLarge != NULL)
        DestroyIcon(g_hIconLarge);
}

} // namespace OSUtil
} // namespace Zephyros
