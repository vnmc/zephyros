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


#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>

#include "lib/cef/include/cef_app.h"

#include "zephyros.h"
#include "base/app.h"
#include "base/cef/client_handler.h"
#include "base/cef/zephyros_cef_win.h"
#include "native_extensions/os_util.h"

#include "res/windows/resource.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;
extern HINSTANCE g_hInst;
extern bool g_isMessageLoopRunning;
extern bool g_isMultithreadedMessageLoop;
extern HWND g_hMessageWnd;

HANDLE g_hndLogFile = NULL;
TCHAR g_szLogFileName[MAX_PATH];


namespace Zephyros {
namespace App {

String GetUserAgent()
{
    String strOS = Zephyros::OSUtil::GetOSVersion();
    StringStream ssUserAgent;
    ssUserAgent << Zephyros::GetAppName() << TEXT(" ") << Zephyros::GetAppVersion() << TEXT("; Windows NT/") << strOS << TEXT("; ");

    bool isLangAdded = false;

    ULONG numLangs = 0;
    ULONG cchBuf = 0;
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLangs, NULL, &cchBuf))
    {
        TCHAR* pszLanguages = new TCHAR[cchBuf];
        if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLangs, pszLanguages, &cchBuf))
        {
            if (numLangs > 0)
            {
                ssUserAgent << pszLanguages[0] << pszLanguages[1];
                isLangAdded = true;
            }
        }

        delete[] pszLanguages;
    }

    if (!isLangAdded)
    {
        // default language if no languages are found
        ssUserAgent << TEXT("en");
    }

    return ssUserAgent.str();
}

void CloseWindow()
{
    // save the window position
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(App::GetMainHwnd(), &wpl);
    Rect r;
    r.x = wpl.rcNormalPosition.left;
    r.y = wpl.rcNormalPosition.top;
    r.w = wpl.rcNormalPosition.right - wpl.rcNormalPosition.left;
    r.h = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top;
    SaveWindowPlacement(&r, wpl.showCmd);

    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser.get())
    {
        // notify the browser window that we would like to close it
        // this will result in a call to ClientHandler::DoClose() if the
        // JavaScript 'onbeforeunload' event handler allows it
        browser->GetHost()->CloseBrowser(false);
    }
}

void Quit()
{
    // close the app's main window
    DestroyWindow(App::GetMainHwnd());

	if (g_hndLogFile)
	{
		CloseHandle(g_hndLogFile);
		g_hndLogFile = NULL;
	}
}

void QuitMessageLoop()
{
    if (!g_isMessageLoopRunning)
        return;

    if (g_isMultithreadedMessageLoop)
    {
        // running in multi-threaded message loop mode
        // need to execute PostQuitMessage on the main application thread
        DCHECK(g_hMessageWnd);
        PostMessage(g_hMessageWnd, WM_COMMAND, ID_QUIT, 0);
    }
    else
        CefQuitMessageLoop();

	if (g_hndLogFile)
	{
		CloseHandle(g_hndLogFile);
		g_hndLogFile = NULL;
	}
}

void BeginWait()
{
    HWND hwnd = GetActiveWindow();
    HCURSOR hCursor = LoadCursor(NULL, IDC_WAIT);
    SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) hCursor);
}

void EndWait()
{
    HWND hwnd = GetActiveWindow();
    HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
    SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) hCursor);
}

void Alert(String title, String msg, AlertStyle style)
{
    HWND hWnd = NULL;
    if (g_handler.get())
        hWnd = g_handler->GetMainHwnd();

    UINT type = MB_ICONINFORMATION;
    switch (style)
    {
    case AlertWarning:
        type = MB_ICONWARNING;
        break;
    case AlertError:
        type = MB_ICONERROR;
        break;
    }

    MessageBox(hWnd, msg.c_str(), title.c_str(), type);
}

HANDLE OpenLogFile()
{
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, g_szLogFileName);

    const TCHAR* szCompanyName = Zephyros::GetCompanyName();
    if (szCompanyName != NULL && szCompanyName[0] != TCHAR('\0'))
    {
        PathAddBackslash(g_szLogFileName);
        PathAppend(g_szLogFileName, szCompanyName);
        CreateDirectory(g_szLogFileName, NULL);
    }

    PathAddBackslash(g_szLogFileName);
    PathAppend(g_szLogFileName, Zephyros::GetAppName());
    CreateDirectory(g_szLogFileName, NULL);

    PathAppend(g_szLogFileName, TEXT("\\debug.log"));
    HANDLE hndLogFile = CreateFile(g_szLogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hndLogFile == INVALID_HANDLE_VALUE)
    {
        //AppShowErrorMessage();
        hndLogFile = CreateFile(g_szLogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    // write marker bytes to mark the file as UTF-8 encoded
    BYTE marker[] = { 0xef, 0xbb, 0xbf };
    DWORD dwBytesWritten = 0;
    WriteFile(hndLogFile, marker, 3, &dwBytesWritten, NULL);

    return hndLogFile;
}

void Log(String msg)
{
    if (!g_hndLogFile || msg.length() == 0)
        return;

    int mbLen = WideCharToMultiByte(CP_UTF8, 0, msg.c_str(), (int) msg.length(), NULL, 0, NULL, NULL);
    if (mbLen == 0)
    {
        App::ShowErrorMessage();
        return;
    }

    char bufTime[30];
    SYSTEMTIME time;
    GetSystemTime(&time);
    sprintf(bufTime, "%d-%d-%d %d:%d:%d.%d ", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    DWORD dwBytesWritten = 0;
    WriteFile(g_hndLogFile, bufTime, (DWORD) strlen(bufTime), &dwBytesWritten, NULL);

    BYTE* buf = new BYTE[mbLen + 3];
    WideCharToMultiByte(CP_UTF8, 0, msg.c_str(), (int) msg.length(), (LPSTR) buf, mbLen, NULL, NULL);
    buf[mbLen] = '\r';
    buf[mbLen + 1] = '\n';
    buf[mbLen + 2] = 0;
    dwBytesWritten = 0;
    WriteFile(g_hndLogFile, buf, mbLen + 2, &dwBytesWritten, NULL);
    FlushFileBuffers(g_hndLogFile);
    delete[] buf;
}

String ShowErrorMessage()
{
    TCHAR* msg = NULL;
    String strMsg;

    DWORD errCode = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, errCode, 0, (LPWSTR) &msg, 0, NULL);

    if (msg != NULL)
    {
        MessageBox(NULL, msg, TEXT("Error"), MB_ICONERROR);
        strMsg = String(msg);
        LocalFree(msg);
    }
    else
    {
        msg = new TCHAR[100];

#ifdef _UNICODE
        wsprintf(msg, L"Error code %ld", errCode);
#else
        sprintf(msg, "Error code %ld", errCode);
#endif

        MessageBox(NULL, msg, TEXT("Error"), MB_ICONERROR);
        strMsg = String(msg);
        delete[] msg;
    }

    return strMsg;
}

void SetMenuItemStatuses(MenuItemStatuses& statuses)
{
    if (!g_handler.get())
        return;

    HMENU hMenu = GetMenu(g_handler->GetMainHwnd());
    if (!hMenu)
        return;

    for (MenuItemStatuses::iterator it = statuses.begin(); it != statuses.end(); ++it)
    {
        int nID = Zephyros::GetMenuIDForCommand(it->first.c_str());
        if (nID)
        {
            int status = it->second;
            EnableMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x01) ? MF_ENABLED : MF_DISABLED));
            CheckMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x02) ? MF_CHECKED : MF_UNCHECKED));
        }
    }
}

} // namespace App
} // namespace Zephyros
