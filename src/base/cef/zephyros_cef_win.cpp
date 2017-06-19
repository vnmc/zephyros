/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
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

#include <tchar.h>
#include <sstream>
#include <string>
#include <map>

#include <minmax.h>
#include <objidl.h>
#include <gdiplus.h>

#include "lib/cef/include/base/cef_bind.h"
#include "lib/cef/include/cef_app.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/wrapper/cef_closure_task.h"

#include "lib/winsparkle/winsparkle.h"
#include "lib/CrashRpt/CrashRpt.h"

#include "zephyros.h"
#include "base/app.h"
#include "base/cef/client_app.h"
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#include "base/cef/local_scheme_handler.h"
#include "base/cef/zephyros_cef_win.h"

#include "util/string_util.h"

#include "native_extensions/custom_url_manager.h"
#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"

#include "res/windows/resource.h"


#define TIMER_SHOW_WINDW 1


// Add Common Controls to the application manifest because it's required to
// support the default tooltip implementation.
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")  // NOLINT(whitespace/line_length)


/////////////////////////////////////////////////////////////////////////////////////////////
// Global Variables

// current instance
HINSTANCE g_hInst;

// the global ClientHandler reference.
extern CefRefPtr<Zephyros::ClientHandler> g_handler;

// is the message loop running?
bool g_isMessageLoopRunning = false;

// do we use a multi-threaded message loop?
bool g_isMultithreadedMessageLoop;

// how the window is shown
UINT g_nCmdShow;
int g_nMinWindowWidth = 0;
int g_nMinWindowHeight = 0;

String g_strCustomUrlToAdd(TEXT(""));

// used for processing messages on the main application thread while running in multi-threaded message loop mode.
HWND g_hMessageWnd = NULL;

extern HANDLE g_hndLogFile;
extern TCHAR g_szLogFileName[MAX_PATH];


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CreateMainWindow();
void InstallCrashReporting();

void SetUserAgentString(CefSettings& settings);
bool HandleOpenCustomURL(LPTSTR lpCommandLine, bool bOtherInstanceRunning);

HWND CreateMessageWindow(HINSTANCE hInstance);
LRESULT CALLBACK MessageWndProc(HWND, UINT, WPARAM, LPARAM);

namespace Zephyros {
namespace App {

HANDLE OpenLogFile();

} // namespace App
} // namespace Zephyros


/////////////////////////////////////////////////////////////////////////////////////////////
// Implementation

namespace Zephyros {

int RunApplication(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_hInst = hInstance;

    // determine if another instance of the application is already running
    bool bOtherInstanceRunning = false;
    HANDLE hMutex = CreateMutex(NULL, FALSE, Zephyros::GetAppName());
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_ALREADY_EXISTS)
        bOtherInstanceRunning = true;
    else if (dwErr == ERROR_ACCESS_DENIED)
    {
        hMutex = OpenMutex(SYNCHRONIZE, FALSE, Zephyros::GetAppName());
        if (hMutex != NULL)
            bOtherInstanceRunning = true;
    }

    // handle "openurl" command line arguments (when a custom URL scheme link was clicked)
    if (HandleOpenCustomURL(lpCmdLine, bOtherInstanceRunning))
    {
        if (hMutex)
            ReleaseMutex(hMutex);
        return 0;
    }

	// initialize OLE
	OleInitialize(0);

    // open the log file for writing
    g_hndLogFile = Zephyros::App::OpenLogFile();
    InstallCrashReporting();

    CefMainArgs main_args(hInstance);
    CefRefPtr<ClientApp> app(new ClientApp());

    // execute the secondary process, if any
    int nExitCode = CefExecuteProcess(main_args, app.get(), NULL);
    if (nExitCode >= 0)
    {
        if (hMutex != NULL)
            ReleaseMutex(hMutex);
        Zephyros::Shutdown();
        return nExitCode;
    }

    // parse command line arguments
    // the passed in values are ignored on Windows
    Zephyros::App::InitCommandLine(0, NULL);

    // populate the settings based on command line arguments
    CefSettings settings;
    Zephyros::App::GetSettings(settings);

    // set the user agent
    SetUserAgentString(settings);

    g_isMultithreadedMessageLoop = settings.multi_threaded_message_loop != 0;

    // initialize CEF
    CefInitialize(main_args, settings, app.get(), NULL);

    // register the "local" scheme (for loading resources from the local file system)
    CefRegisterSchemeHandlerFactory("local", "", new LocalSchemeHandlerFactory());
    
    std::vector<CefString> schemes;
    schemes.push_back("local");
    CefCookieManager::GetGlobalManager(NULL)->SetSupportedSchemes(schemes, NULL);

    // start GDI+
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // check the license
    Zephyros::AbstractLicenseManager* pMgr = Zephyros::GetLicenseManager();
    if (pMgr)
    {
        if (pMgr->IsLicensingLink(g_strCustomUrlToAdd))
        {
            pMgr->ActivateFromURL(g_strCustomUrlToAdd);
            g_strCustomUrlToAdd = TEXT("");
        }
        pMgr->Start();
    }

    // create the main window and run
    int nResult = 0;
    if (pMgr == NULL || pMgr->CanStartApp())
        nResult = CreateMainWindow();

    // shut down CEF
    CefShutdown();

    Gdiplus::GdiplusShutdown(gdiplusToken);

	OleUninitialize();

    // Uninitialize CrashRpt before exiting the main function
    const TCHAR* szCrashReportingURL = Zephyros::GetCrashReportingURL();
    if (szCrashReportingURL != NULL && szCrashReportingURL[0] != TCHAR('\0'))
        crUninstall();

    if (hMutex != NULL)
        ReleaseMutex(hMutex);

    Zephyros::Shutdown();

    return nResult;
}

void LoadWindowPlacement(Rect* pRectNormal, UINT* pShowCmd)
{
    pRectNormal->x = CW_USEDEFAULT;
    pRectNormal->y = CW_USEDEFAULT;
    pRectNormal->w = Zephyros::GetDefaultWindowSize().nWidth;
    pRectNormal->h = Zephyros::GetDefaultWindowSize().nHeight;
    *pShowCmd = SW_SHOWDEFAULT;

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, Zephyros::GetWindowsInfo().szRegistryKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD type = 0;
        DWORD len = 0;
        if (RegQueryValueEx(hKey, TEXT("window"), NULL, &type, NULL, &len) == ERROR_SUCCESS)
        {
            // read the data from the registry
            BYTE* buf = new BYTE[len];
            RegQueryValueEx(hKey, TEXT("window"), NULL, &type, buf, &len);

            // parse the string
            StringStream ss(ToString(buf, len));
            TCHAR delim;
            ss >> pRectNormal->x;
            ss >> delim;
            ss >> pRectNormal->y;
            ss >> delim;
            ss >> pRectNormal->w;
            ss >> delim;
            ss >> pRectNormal->h;
            ss >> delim;
            ss >> *pShowCmd;

            delete[] buf;
        }

        RegCloseKey(hKey);
    }
}

void SaveWindowPlacement(Rect* pRectNormal, UINT showCmd)
{
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, Zephyros::GetWindowsInfo().szRegistryKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        StringStream ss;
        ss << pRectNormal->x << TEXT(',') << pRectNormal->y << TEXT(',') << pRectNormal->w << TEXT(',') << pRectNormal->h << TEXT(',') << showCmd;
        String data = ss.str();

        RegSetValueEx(hKey, TEXT("window"), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (DWORD) ((data.length() + 1) * sizeof(TCHAR)));
        RegCloseKey(hKey);
    }
}

void AdjustWindowPlacementToMonitor(Rect* pRect)
{
    POINT pt;
    pt.x = pRect->x;
    pt.y = pRect->y;
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    // get the monitor info
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMonitor, &info))
        return;

    // adjust for work area
    pRect->x += info.rcWork.left - info.rcMonitor.left;
    pRect->y += info.rcWork.top - info.rcMonitor.top;

    // adjust width and height
    if (pRect->w > info.rcWork.right - info.rcWork.left)
        pRect->w = info.rcWork.right - info.rcWork.left;
    if (pRect->h > info.rcWork.bottom - info.rcWork.top)
        pRect->h = info.rcWork.bottom - info.rcWork.top;

    // adjust position
    if (pRect->x < info.rcWork.left)
        pRect->x = info.rcWork.left;
    if (pRect->x + pRect->w > info.rcWork.right)
        pRect->x = info.rcWork.right - pRect->w;
    if (pRect->y < info.rcWork.top)
        pRect->y = info.rcWork.top;
    if (pRect->y + pRect->h > info.rcWork.bottom)
        pRect->y = info.rcWork.bottom - pRect->h;
}

} // namespace Zephyros


// Set focus to |browser| on the UI thread.
static void SetFocusToBrowser(CefRefPtr<CefBrowser> browser)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // execute on the UI thread
        CefPostTask(TID_UI, base::Bind(&SetFocusToBrowser, browser));
        return;
    }

    if (!g_handler)
        return;

    // give focus to the browser
    browser->GetHost()->SetFocus(true);
}

int CreateMainWindow()
{
    Zephyros::WindowsInfo info = Zephyros::GetWindowsInfo();

    // create the window class name
    TCHAR szWindowClass[256];
    _tcscpy(szWindowClass, TEXT("MainWnd_"));
    _tcsncat(szWindowClass, Zephyros::GetAppName(), min(_tcslen(Zephyros::GetAppName()), 256 - 9));

    // register the window class of the main window
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = g_hInst;
    wcex.hIcon = info.nIconID ? LoadIcon(g_hInst, MAKEINTRESOURCE(info.nIconID)) : NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = info.nMenuID ? MAKEINTRESOURCE(info.nMenuID) : NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = info.nIconID ? LoadIcon(g_hInst, MAKEINTRESOURCE(info.nIconID)) : NULL;
    ::RegisterClassEx(&wcex);

    // create the main window
    Rect rectWindow;
    Zephyros::LoadWindowPlacement(&rectWindow, &g_nCmdShow);
    Zephyros::AdjustWindowPlacementToMonitor(&rectWindow);

    HWND hWndMain = CreateWindow(
        szWindowClass, Zephyros::GetAppName(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        rectWindow.x, rectWindow.y, rectWindow.w, rectWindow.h,
        NULL, NULL, g_hInst, NULL
    );

    if (hWndMain == NULL)
    {
        Zephyros::App::ShowErrorMessage();
        return FALSE;
    }

    if (info.nAccelID)
    {
        HACCEL hAccelTable = LoadAccelerators(g_hInst, MAKEINTRESOURCE(info.nAccelID));
        g_handler->SetAccelTable(hAccelTable);
    }

    // initialize winsparkle
    const TCHAR* szUpdaterURL = Zephyros::GetUpdaterURL();
    if (szUpdaterURL != NULL && szUpdaterURL[0] != TCHAR('\0'))
    {
        String strUpdaterURL(szUpdaterURL);
        win_sparkle_set_appcast_url(std::string(strUpdaterURL.begin(), strUpdaterURL.end()).c_str());
        win_sparkle_init();
    }

    g_isMessageLoopRunning = true;
    int nResult = 0;
    if (!g_isMultithreadedMessageLoop)
    {
        // run the CEF message loop
        // this function will block until the application recieves a WM_QUIT message
        CefRunMessageLoop();
    }
    else
    {
        // create a hidden window for message processing
        g_hMessageWnd = CreateMessageWindow(g_hInst);
        DCHECK(g_hMessageWnd);

        MSG msg;

        // run the application message loop
        while (GetMessage(&msg, NULL, 0, 0))
        {
            HACCEL hAccel = g_handler->GetAccelTable();
            if (hAccel && !TranslateAccelerator(msg.hwnd, hAccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        DestroyWindow(g_hMessageWnd);
        g_hMessageWnd = NULL;

        nResult = static_cast<int>(msg.wParam);
    }

    g_isMessageLoopRunning = false;
    g_handler->ReleaseCefObjects();

    if (szUpdaterURL != NULL && szUpdaterURL[0] != TCHAR('\0'))
        win_sparkle_cleanup();

    Zephyros::OSUtil::CleanUp();

    return nResult;
}

// Define the callback function that will be called on crash
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
    // the application has crashed!

    // close the log file here to ensure CrashRpt is able to include it into error report
    if (g_hndLogFile)
    {
        CloseHandle(g_hndLogFile);
        g_hndLogFile = NULL;
    }

    // return CR_CB_DODEFAULT to generate error report
    return CR_CB_DODEFAULT;
}

void InstallCrashReporting()
{
    const TCHAR* szCrashReportingURL = Zephyros::GetCrashReportingURL();
    if (szCrashReportingURL == NULL || szCrashReportingURL[0] == TCHAR('\0'))
        return;

    // define CrashRpt configuration parameters
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);

    info.pszAppName = Zephyros::GetAppName();
    info.pszAppVersion = Zephyros::GetAppVersion();
    info.pszUrl = szCrashReportingURL;
    info.pszPrivacyPolicyURL = Zephyros::GetCrashReportingPrivacyPolicyURL();

    // send report only over HTTP
    info.uPriorities[CR_HTTP] = 1;
    info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;
    info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY;

    // Install all available exception handlers and restart the app after a crash
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS | CR_INST_APP_RESTART | CR_INST_SEND_QUEUED_REPORTS;

    // Install crash reporting
    if (crInstall(&info) == 0)
    {
        Zephyros::App::Log(TEXT("Crash reporting installed successfully"));

        // Set crash callback function
        crSetCrashCallback(CrashCallback, NULL);

        // Add our log file to the error report
        crAddFile2(g_szLogFileName, NULL, TEXT("Log File"), CR_AF_MAKE_FILE_COPY);
    }
    else
    {
        // something went wrong. Get error message.
        TCHAR szErrorMsg[512] = TEXT("");
        crGetLastErrorMsg(szErrorMsg, 512);
        Zephyros::App::Log(TEXT("Failed to install crash reporting:"));
        Zephyros::App::Log(szErrorMsg);
    }
}

void SetUserAgentString(CefSettings& settings)
{
    String userAgent = Zephyros::App::GetUserAgent();
    std::string strUserAgent(userAgent.begin(), userAgent.end());
    CefString(&settings.user_agent).FromASCII(strUserAgent.c_str());
}

bool HandleOpenCustomURL(LPTSTR lpCommandLine, bool bOtherInstanceRunning)
{
    if (_tcsnccmp(lpCommandLine, TEXT("openurl"), 7) == 0)
    {
        // extract the URL (remove the "openurl " and the quotation marks)
        String strUrl(lpCommandLine);
        strUrl = strUrl.substr(9 /* strlen("openurl \"") */, strUrl.length() - 10);

        if (!bOtherInstanceRunning)
        {
            g_strCustomUrlToAdd = strUrl;
            return false;
        }

        // create the window class name
        TCHAR szWindowClass[256];
        _tcscpy(szWindowClass, TEXT("MainWnd_"));
        _tcsncat(szWindowClass, Zephyros::GetAppName(), min(_tcslen(Zephyros::GetAppName()), 256 - 9));

        HWND hWnd = FindWindow(szWindowClass, NULL);
        if (!hWnd)
        {
            // no window could be found; start a new instance
            g_strCustomUrlToAdd = strUrl;
            return false;
        }

        COPYDATASTRUCT cs;
        cs.cbData = (DWORD) ((strUrl.length() + 1) * sizeof(TCHAR));
        cs.lpData = (void*) strUrl.c_str();
        SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM) &cs);

        return true;
    }

    return false;
}

/**
 * Processes messages for the main window.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    // callback for the main window
    switch (message)
    {
    case WM_CREATE:
        {
            // create the single static handler class instance
            g_handler = new Zephyros::ClientHandler();
            g_handler->SetMainHwnd(hWnd);

            // initialize the custom URL manager, and add the URL if there is one
            if (g_strCustomUrlToAdd.length() > 0)
                Zephyros::GetNativeExtensions()->GetCustomURLManager()->AddURL(g_strCustomUrlToAdd);

            // create the child windows used for navigation
            RECT rect;
            GetClientRect(hWnd, &rect);

            CefWindowInfo info;
            CefBrowserSettings settings;
            settings.web_security = STATE_DISABLED;

            // initialize window info to the defaults for a child window.
            info.SetAsChild(hWnd, rect);

            // create the new child browser window
            CefBrowserHost::CreateBrowser(info, g_handler.get(), Zephyros::GetAppURL(), settings, NULL);

            // show the window after 5 seconds, even if the content hasn't completed loading
            SetTimer(hWnd, TIMER_SHOW_WINDW, 5000, NULL);

            return 0;
        }

    case WM_TIMER:
        if (wParam == TIMER_SHOW_WINDW)
        {
            HWND hWndMain = g_handler->GetMainHwnd();
            if (!IsWindowVisible(hWndMain))
            {
                ShowWindow(hWndMain, g_nCmdShow);
                UpdateWindow(hWndMain);
            }
            KillTimer(hWndMain, TIMER_SHOW_WINDW);
            return 0;
        }
        return 1;

    case WM_COMMAND:
        {
            CefRefPtr<CefBrowser> browser;
            if (g_handler.get())
                browser = g_handler->GetBrowser();

            int wmId = LOWORD(wParam);

            // parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_CHECKFORUPDATES:
                win_sparkle_check_update_with_ui();
                return 0;

            case ID_HELP_PURCHASELICENSE:
                if (Zephyros::GetLicenseManager())
                    Zephyros::GetLicenseManager()->OpenPurchaseLicenseURL();
                return 0;

            case ID_HELP_ENTERLICENSEKEY:
                if (Zephyros::GetLicenseManager())
                    Zephyros::GetLicenseManager()->ShowEnterLicenseDialog();
                return 0;

            case IDM_EXIT:
                if (g_handler.get())
                    g_handler->CloseAllBrowsers(false);
                return 0;

            default:
                {
                    const TCHAR* szCommand = Zephyros::GetMenuCommandForID(wmId);
                    if (szCommand)
                    {
                        if (_tcscmp(szCommand, TEXT(MENUCOMMAND_CHECK_UPDATE)) == 0)
                            win_sparkle_check_update_with_ui();
                        else if (_tcscmp(szCommand, TEXT(MENUCOMMAND_PURCHASE_LICENSE)) == 0)
                        {
                            if (Zephyros::GetLicenseManager())
                                Zephyros::GetLicenseManager()->OpenPurchaseLicenseURL();
                        }
                        else if (_tcscmp(szCommand, TEXT(MENUCOMMAND_ENTER_LICENSE)) == 0)
                        {
                            if (Zephyros::GetLicenseManager())
                                Zephyros::GetLicenseManager()->ShowEnterLicenseDialog();
                        }
                        else if (_tcscmp(szCommand, TEXT(MENUCOMMAND_TERMINATE)) == 0)
                        {
                            if (g_handler.get())
                                g_handler->CloseAllBrowsers(false);
                        }
                        else if (g_handler.get())
                        {
                            CefRefPtr<CefListValue> args = CefListValue::Create();
                            args->SetString(0, szCommand);
                            g_handler->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onMenuCommand"), args);
                        }

                        return 0;
                    }
                }
                break;
            }
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_SETFOCUS:
        if (g_handler.get())
        {
            CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
            if (browser)
                SetFocusToBrowser(browser);
        }
        return 0;

    case WM_SIZE:
        {
            if (!g_handler.get())
                break;

            if (g_handler->GetBrowser())
            {
                // retrieve the window handle (parent window with off-screen rendering)
                CefWindowHandle hwnd = g_handler->GetBrowser()->GetHost()->GetWindowHandle();
                if (hwnd)
                {
                    if (wParam == SIZE_MINIMIZED)
                    {
                        // for windowed browsers when the frame window is minimized set the
                        // browser window size to 0x0 to reduce resource usage
                        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
                    }
                    else
                    {
                        // resize the window to match the new frame size
                        RECT rect;
                        GetClientRect(hWnd, &rect);

                        HDWP hdwp = BeginDeferWindowPos(1);
                        hdwp = DeferWindowPos(hdwp, hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
                        EndDeferWindowPos(hdwp);
                    }
                }
            }
        }
        break;

    case WM_GETMINMAXINFO:
        {
            POINT ptBorder;
            Zephyros::OSUtil::GetWindowBorderSize(&ptBorder);

            if (g_nMinWindowWidth > 0)
                (reinterpret_cast<MINMAXINFO*> (lParam))->ptMinTrackSize.x = g_nMinWindowWidth + ptBorder.x;
            if (g_nMinWindowHeight > 0)
                (reinterpret_cast<MINMAXINFO*> (lParam))->ptMinTrackSize.y = g_nMinWindowHeight + ptBorder.y;
        }
        break;

    case WM_MOVING:
    case WM_MOVE:
        // notify the browser of move events so that popup windows are displayed
        // in the correct location and dismissed when the window moves
        if (g_handler.get() && g_handler->GetBrowser())
            g_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
        return 0;

    case WM_ERASEBKGND:
        if (g_handler.get() && g_handler->GetBrowser())
        {
            // don't erase the background if the browser window has been loaded to avoid flickering
            CefWindowHandle hwnd = g_handler->GetBrowser()->GetHost()->GetWindowHandle();
            if (hwnd)
                return 0;
        }
        break;

    case WM_ENTERMENULOOP:
        // entering the menu loop for the application menu
        if (!wParam)
            CefSetOSModalLoop(true);
        break;

    case WM_EXITMENULOOP:
        // exiting the menu loop for the application menu
        if (!wParam)
            CefSetOSModalLoop(false);
        break;

    case WM_TIMECHANGE:
        if (Zephyros::GetLicenseManager() != NULL)
            Zephyros::GetLicenseManager()->CheckDemoValidity();
        return 0;

    case WM_COPYDATA:
        if (g_handler.get())
        {
            Zephyros::CustomURLManager* pMgr = Zephyros::GetNativeExtensions()->GetCustomURLManager();
            if (pMgr)
            {
                pMgr->AddURL((TCHAR*) ((COPYDATASTRUCT*) lParam)->lpData);
                pMgr->FireCustomURLs();
            }
        }
        break;

    case WM_CLOSE:
        if (g_handler.get() && !g_handler->IsClosing())
        {
            // invoke the "onAppTerminating" callbacks and cancel the close
            g_handler->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onAppTerminating"), CefListValue::Create());
            return 0;
        }

        // allow the close
        break;

    case WM_DESTROY:
        // quitting CEF is handled in ClientHandler::OnBeforeClose()
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateMessageWindow(HINSTANCE hInstance)
{
    static const TCHAR kWndClass[] = L"ClientMessageWindow";

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = MessageWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kWndClass;
    RegisterClassEx(&wc);

    return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
}

LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case ID_QUIT:
                PostQuitMessage(0);
                return 0;
            }
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
