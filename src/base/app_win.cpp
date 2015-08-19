// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <direct.h>
#include <Shlwapi.h>
#include <ShlObj.h>

#include <tchar.h>
#include <sstream>
#include <string>
#include <map>

#include <minmax.h>
#include <gdiplus.h>

#include "include\cef_app.h"
#include "include\cef_browser.h"
#include "include\cef_frame.h"
#include "include\cef_runnable.h"

#include "lib/winsparkle\winsparkle.h"
#include "lib/CrashRpt\CrashRpt.h"

#include "base/zephyros_impl.h"
#include "base/app.h"
#include "base/licensing.h"
#include "base/cef/client_handler.h"
#include "base/cef/resource_util.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "components/dialog_win.h"

#include "native_extensions/custom_url_manager.h"
#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"


#define MAX_LOADSTRING 100
#define MAX_URL_LENGTH 255

#define TIMER_SHOW_WINDW 1


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

// the log file handle
TCHAR g_szLogFileName[MAX_PATH];
HANDLE g_hndLogFile;

// menu commands
std::map<int, String> g_mapMenuCommands;
std::map<String, int> g_mapMenuIDs;

// localized strings (for demo/licensing box)
std::map<String, UINT> g_mapLocalizedStrings;

String g_strCustomUrlToAdd(TEXT(""));


// forward declarations of functions included in this code module
void InstallCrashReporting();
void SetUserAgentString(CefSettings& settings);
int CreateMainWindow();
ATOM MyRegisterClass(HINSTANCE hInstance, TCHAR* szWindowClass);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InitMenuCommands();
void InitLocalizedStrings();
bool HandleOpenCustomURL(LPTSTR lpCommandLine, bool bOtherInstanceRunning);

// used for processing messages on the main application thread while running
// in multi-threaded message loop mode.
HWND g_hMessageWnd = NULL;
HWND CreateMessageWindow(HINSTANCE hInstance);
LRESULT CALLBACK MessageWndProc(HWND, UINT, WPARAM, LPARAM);


typedef struct
{
	int x;
	int y;
	int w;
	int h;
} Rect;

void LoadWindowPlacement(Rect* pRectNormal, UINT* pCmdShow);
void SaveWindowPlacement(Rect* pRectNormal, UINT cmdShow);
void AdjustWindowPlacementToMonitor(Rect* pRect);


namespace Zephyros {
namespace App {

HANDLE OpenLogFile();

} // namespace App
} // namespace Zephyros


#if defined(OS_WIN)
// Add Common Controls to the application manifest because it's required to
// support the default tooltip implementation.
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")  // NOLINT(whitespace/line_length)
#endif


/////////////////////////////////////////////////////////////////////////////////////////////
// Implementation

//
// Program entry point function.
//
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);


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

#if 0
	if (HasSpecialCommandLine(lpCmdLine))
		return 0;
#endif

	// open the log file for writing
	g_hndLogFile = Zephyros::App::OpenLogFile();
	InstallCrashReporting();

	g_hInst = hInstance;

	InitLocalizedStrings();

	CefMainArgs main_args(hInstance);
	CefRefPtr<Zephyros::ClientApp> app(new Zephyros::ClientApp());

	// execute the secondary process, if any
	int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
	if (exit_code >= 0)
	{
		if (hMutex != NULL)
			ReleaseMutex(hMutex);
		return exit_code;
	}

	// parse command line arguments
	// the passed in values are ignored on Windows
	Zephyros::App::InitCommandLine(0, NULL);

	// populate the settings based on command line arguments
	CefSettings settings;
	settings.no_sandbox = true;
	Zephyros::App::GetSettings(settings);

	// set the user agent
	SetUserAgentString(settings);
	
	g_isMultithreadedMessageLoop = settings.multi_threaded_message_loop != 0;

	// initialize CEF
	CefInitialize(main_args, settings, app.get(), NULL);

	// register cookieable schemes with the global cookie manager
	std::vector<CefString> schemes;
	schemes.push_back("http");
	schemes.push_back("https");
	CefCookieManager::GetGlobalManager()->SetSupportedSchemes(schemes);

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

	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// create the main window and run
	int result = 0;
	if (pMgr == NULL || pMgr->CanStartApp())
	{
		InitMenuCommands();
		result = CreateMainWindow();
	}
	
	// shut down CEF
	CefShutdown();
//	delete g_pLicenseManager;

	Gdiplus::GdiplusShutdown(gdiplusToken);

	// Uninitialize CrashRpt before exiting the main function
	crUninstall();

	if (hMutex != NULL)
		ReleaseMutex(hMutex);

	return result;
}

void SetUserAgentString(CefSettings& settings)
{
	std::stringstream ssUserAgent;

	String strOSWide = Zephyros::OSUtil::GetOSVersion();
	std::string strOS(strOSWide.begin(), strOSWide.end());
	if (strOS[strOS.length() - 1] == '\0')
		strOS = strOS.substr(0, strOS.length() - 1);

	ssUserAgent << Zephyros::GetAppName() << " " << Zephyros::GetAppVersion() << "; Windows NT/" << strOS << "; ";

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
				ssUserAgent << (char) pszLanguages[0] << (char) pszLanguages[1];
				isLangAdded = true;
			}
		}

		delete[] pszLanguages;
	}

	if (!isLangAdded)
	{
		// default language if no languages are found
		ssUserAgent << "en";
	}

	CefString(&settings.user_agent).FromASCII(ssUserAgent.str().c_str());
}

// Define the callback function that will be called on crash
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{  
	// The application has crashed!

	// Close the log file here to ensure CrashRpt is able to include it into error report
	if (g_hndLogFile)
	{
		CloseHandle(g_hndLogFile);
		g_hndLogFile = NULL;
	}

	// Return CR_CB_DODEFAULT to generate error report
	return CR_CB_DODEFAULT;
}

void InstallCrashReporting()
{
	// define CrashRpt configuration parameters
	CR_INSTALL_INFO info;  
	memset(&info, 0, sizeof(CR_INSTALL_INFO));  
	info.cb = sizeof(CR_INSTALL_INFO);
	
	info.pszAppName = Zephyros::GetAppName();
	info.pszAppVersion = Zephyros::GetAppVersion();
	info.pszUrl = Zephyros::GetCrashReportingURL();
	info.pszPrivacyPolicyURL = Zephyros::GetPrivacyPolicyURL();

	// send report only over HTTP
	info.uPriorities[CR_HTTP] = 1;
	info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;
	info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY;

	// Install all available exception handlers and restart the app after a crash
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS | CR_INST_APP_RESTART | CR_INST_SEND_QUEUED_REPORTS;
	
	// Install crash reporting
	int nResult = crInstall(&info);    
	if(nResult == 0)
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

int CreateMainWindow()
{
	// initialize title bar text and the main window class name
	TCHAR szTitle[MAX_LOADSTRING];
	TCHAR szWindowClass[MAX_LOADSTRING];
	LoadString(g_hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(g_hInst, IDC_CEFCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(g_hInst, szWindowClass);

	// create the main window
	Rect rectWindow;
	LoadWindowPlacement(&rectWindow, &g_nCmdShow);
	AdjustWindowPlacementToMonitor(&rectWindow);

	HWND hWndMain = CreateWindow(
		szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		rectWindow.x, rectWindow.y, rectWindow.w, rectWindow.h,
		NULL, NULL, g_hInst, NULL
	);

	if (hWndMain == NULL)
	{
		Zephyros::App::ShowErrorMessage();
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDC_CEFCLIENT));
	g_handler->SetAccelTable(hAccelTable);

	// initialize winsparkle
	win_sparkle_set_appcast_url(Zephyros::GetUpdaterURL());
	win_sparkle_init();

	g_isMessageLoopRunning = true;
	int result = 0;
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
		ASSERT(g_hMessageWnd);

		MSG msg;

		// run the application message loop
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DestroyWindow(g_hMessageWnd);
		g_hMessageWnd = NULL;

		result = static_cast<int>(msg.wParam);
	}

	g_isMessageLoopRunning = false;
	g_handler->ReleaseCefObjects();
	win_sparkle_cleanup();
	FreeResources();
	Zephyros::OSUtil::CleanUp();

	return result;
}

/**
 * This function and its usage are only necessary if you want this code
 * to be compatible with Win32 systems prior to the 'RegisterClassEx'
 * function that was added to Windows 95. It is important to call this
 * function so that the application will get 'well formed' small icons
 * associated with it.
 */
ATOM MyRegisterClass(HINSTANCE hInstance, TCHAR* szWindowClass)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_CEFCLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));

	return RegisterClassEx(&wcex);
}

void InitMenuCommands()
{
	// create the map menu item ID -> command ID
	g_mapMenuCommands[ID_FILE_PREFERENCES] = TEXT("show_preferences");
	g_mapMenuCommands[ID_FILE_RESETPREFERENCES] = TEXT("reset_preferences");
	g_mapMenuCommands[ID_FILE_RESETSITEDEFAULTS] = TEXT("reset_site_defaults");
	g_mapMenuCommands[ID_VIEW_TOGGLETRAY] = TEXT("toggle_tray");
	g_mapMenuCommands[ID_SITES_ADDSITE] = TEXT("add_site");
	g_mapMenuCommands[ID_SITES_REMOVESITES] = TEXT("remove_sites");
	g_mapMenuCommands[ID_SITES_SEARCH] = TEXT("search_sites");
	g_mapMenuCommands[ID_SERVER_REFRESH] = TEXT("refresh_devices");
	g_mapMenuCommands[ID_SERVER_LAUNCHINDEFAULTBROWSER] = TEXT("server_launch");
	g_mapMenuCommands[ID_SYNCMODES_DEFAULT] = TEXT("syncmode_default");
	g_mapMenuCommands[ID_SYNCMODES_PRESENTATION] = TEXT("syncmode_presentation");
	g_mapMenuCommands[ID_SYNCMODES_SELECTIVESYNC] = TEXT("syncmode_selective");
	g_mapMenuCommands[ID_SERVER_STOPSERVER] = TEXT("stop_server");
	g_mapMenuCommands[ID_SERVER_SHOWSERVERURLQRCODE] = TEXT("show_server_qr");
	g_mapMenuCommands[ID_WORKSPACE_OPENWORKSPACE] = TEXT("open_workspace");
	g_mapMenuCommands[ID_WORKSPACE_SETASWORKSPACE] = TEXT("set_workspace");
	g_mapMenuCommands[ID_WORKSPACE_RESETWORKSPACETODEFAULT] = TEXT("reset_workspace");
	g_mapMenuCommands[ID_WORKSPACE_CLOSEWORKSPACEWINDOWS] = TEXT("close_workspace");
	g_mapMenuCommands[ID_WORKSPACE_WORKSPACEINFO] = TEXT("workspace_info");
	g_mapMenuCommands[IDM_ABOUT] = TEXT("show_about");
	g_mapMenuCommands[ID_HELP_HELP] = TEXT("show_help");

	// create the reverse map
	for (std::map<int, String>::iterator it = g_mapMenuCommands.begin(); it != g_mapMenuCommands.end(); ++it)
		g_mapMenuIDs[it->second] = it->first;
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

		// send the URL to add to the other instance of Ghostlab
		TCHAR szWindowClass[MAX_LOADSTRING];
		LoadString(g_hInst, IDC_CEFCLIENT, szWindowClass, MAX_LOADSTRING);

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
			Zephyros::CustomUrlManager* pMgr = g_handler->GetClientExtensionHandler()->GetState()->m_customUrlManager;
			if (g_strCustomUrlToAdd.length() > 0)
				pMgr->AddUrl(g_strCustomUrlToAdd);

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

			// Todo: find a better solution for applying this e.g. with timeout in case showing of windows initially fails
			//ShowWindow(hWnd, SW_SHOWDEFAULT);
			//UpdateWindow(hWnd);
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
			int wmEvent = HIWORD(wParam);

			// parse the menu selections
			if (g_mapMenuCommands.find(wmId) != g_mapMenuCommands.end())
			{
				String commandId = g_mapMenuCommands[wmId];
				if (g_handler.get())
				{
					CefRefPtr<CefListValue> args = CefListValue::Create();
					args->SetString(0, commandId);
					g_handler->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onMenuCommand"), args);
				}

				return 0;
			}
			else
			{
				switch (wmId)
				{
				case ID_FILE_CHECKFORUPDATES:
					win_sparkle_check_update_with_ui();
					return 0;

				case ID_TOOLS_ENABLELOOPBACKFORINTERNETEXPLORER:
					Zephyros::NetworkUtil::SetIELoopbackExemption(true);
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
				}
			}

			break;
		}

    case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;

    case WM_SETFOCUS:
		if (g_handler.get() && g_handler->GetBrowser())
		{
			// pass focus to the browser window
			CefWindowHandle hwnd = g_handler->GetBrowser()->GetHost()->GetWindowHandle();
			if (hwnd)
				PostMessage(hwnd, WM_SETFOCUS, wParam, NULL);
		}
		return 0;

    case WM_SIZE:
		// minimizing resizes the window to 0x0 which causes our layout to go all
		// screwy, so we just ignore it.
		if (wParam != SIZE_MINIMIZED && g_handler.get() && g_handler->GetBrowser())
		{
			CefWindowHandle hwnd = g_handler->GetBrowser()->GetHost()->GetWindowHandle();
			if (hwnd)
			{
				// resize the browser window and address bar to match the new frame window size
				RECT rect;
				GetClientRect(hWnd, &rect);

				HDWP hdwp = BeginDeferWindowPos(1);
				hdwp = DeferWindowPos(hdwp, hwnd, NULL,
				rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
				EndDeferWindowPos(hdwp);
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
		// Notify the browser of move events so that popup windows are displayed
		// in the correct location and dismissed when the window moves.
		if (g_handler.get() && g_handler->GetBrowser())
			g_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
		return 0;

    case WM_ERASEBKGND:
		if (g_handler.get() && g_handler->GetBrowser())
		{
			CefWindowHandle hwnd = g_handler->GetBrowser()->GetHost()->GetWindowHandle();
			if (hwnd)
			{
				// don't erase the background if the browser window has been loaded (this avoids flashing)
				return 1;
			}
		}
		break;

    case WM_ENTERMENULOOP:
		if (!wParam)
		{
			// entering the menu loop for the application menu
			CefSetOSModalLoop(true);
		}
		break;

    case WM_EXITMENULOOP:
		if (!wParam)
		{
			// Exiting the menu loop for the application menu.
			CefSetOSModalLoop(false);
		}
		break;

	case WM_TIMECHANGE:
		if (Zephyros::GetLicenseManager() != NULL)
			Zephyros::GetLicenseManager()->CheckDemoValidity();
		return 0;

	case WM_COPYDATA:
		if (g_handler.get())
		{
			Zephyros::CustomUrlManager* pMgr = g_handler->GetClientExtensionHandler()->GetState()->m_customUrlManager;
			if (pMgr)
			{
				pMgr->AddUrl((TCHAR*) ((COPYDATASTRUCT*) lParam)->lpData);
				pMgr->FireCustomUrls();
			}
		}
		break;

    case WM_CLOSE:
		if (g_handler.get() && !g_handler->IsClosing())
		{
			WINDOWPLACEMENT wpl;
			wpl.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(Zephyros::App::GetMainHwnd(), &wpl);
			Rect r;
			r.x = wpl.rcNormalPosition.left;
			r.y = wpl.rcNormalPosition.top;
			r.w = wpl.rcNormalPosition.right - wpl.rcNormalPosition.left;
			r.h = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top;
			SaveWindowPlacement(&r, wpl.showCmd);

			g_handler->GetClientExtensionHandler()->InvokeCallbacks(TEXT("onAppTerminating"), CefListValue::Create());

			CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
			if (browser.get())
			{
				// Notify the browser window that we would like to close it. This
				// will result in a call to ClientHandler::DoClose() if the
				// JavaScript 'onbeforeunload' event handler allows it.
				browser->GetHost()->CloseBrowser(false);

				// cancel the close
				return 0;
			}
		}

		// allow the close
		break;

    case WM_DESTROY:
		// quitting CEF is handled in ClientHandler::OnBeforeClose().
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateMessageWindow(HINSTANCE hInstance)
{
	static const TCHAR kWndClass[] = TEXT("ClientMessageWindow");

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


/////////////////////////////////////////////////////////////////////////////////////////////
// Global Functions

void LoadWindowPlacement(Rect* pRectNormal, UINT* pShowCmd)
{
	pRectNormal->x = CW_USEDEFAULT;
	pRectNormal->y = CW_USEDEFAULT;
	pRectNormal->w = 400;
	pRectNormal->h = 700;
	*pShowCmd = SW_SHOWDEFAULT;

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, Zephyros::GetRegistryKey(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD type = 0;
		DWORD len = 0;
		if (RegQueryValueEx(hKey, TEXT("window"), NULL, &type, NULL, &len) == ERROR_SUCCESS)
		{
			// read the data from the registry
			BYTE* buf = new BYTE[len];
			RegQueryValueEx(hKey, TEXT("window"), NULL, &type, buf, &len);

			// parse the string
			StringStream ss(String(reinterpret_cast<TCHAR*>(buf), reinterpret_cast<TCHAR*>(buf + len - sizeof(TCHAR))));
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
	if (RegCreateKeyEx(HKEY_CURRENT_USER, Zephyros::GetRegistryKey(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
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


namespace Zephyros {
namespace App {

void Quit()
{
	// close the app's main window
	DestroyWindow(App::GetMainHwnd());
}

void QuitMessageLoop()
{
	if (!g_isMessageLoopRunning)
		return;

	if (g_isMultithreadedMessageLoop)
	{
		// running in multi-threaded message loop mode
		// need to execute PostQuitMessage on the main application thread
		ASSERT(g_hMessageWnd);
		PostMessage(g_hMessageWnd, WM_COMMAND, ID_QUIT, 0);
	}
	else
		CefQuitMessageLoop();

	CloseHandle(g_hndLogFile);
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

	PathAppend(g_szLogFileName, TEXT("\\Vanamco"));
	CreateDirectory(g_szLogFileName, NULL);

	PathAppend(g_szLogFileName, TEXT("\\"));
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
	if (msg.length() == 0)
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

String LocalizeString(String src)
{
	TCHAR retVal[MAX_LOADSTRING];

	if (g_mapLocalizedStrings.find(src) == g_mapLocalizedStrings.end())
		return src;
	
	LoadString(g_hInst, g_mapLocalizedStrings[src], retVal, MAX_LOADSTRING);
	return String(retVal);
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
		String strMsg = String(msg);
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

void SetMenuItemStatuses(JavaScript::Object items)
{
	if (!g_handler.get())
		return;

	HMENU hMenu = GetMenu(g_handler->GetMainHwnd());
	if (!hMenu)
		return;

	JavaScript::KeyList keys;
	items->GetKeys(keys);
	for (JavaScript::KeyType commandId : keys)
	{
		String strCmdId = commandId;
		if (g_mapMenuIDs.find(strCmdId) != g_mapMenuIDs.end())
		{
			UINT uID = g_mapMenuIDs[strCmdId];
			int status = items->GetInt(commandId);

			EnableMenuItem(hMenu, uID, MF_BYCOMMAND | ((status & 0x01) ? MF_ENABLED : MF_DISABLED));
			CheckMenuItem(hMenu, uID, MF_BYCOMMAND | ((status & 0x02) ? MF_CHECKED : MF_UNCHECKED));
		}
	}
}

} // namespace App
} // namespace Zephyros
