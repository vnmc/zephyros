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


#include <fstream>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <X11/Xlib.h>
#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h

#include "lib/cef/include/base/cef_bind.h"
#include "lib/cef/include/cef_app.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/wrapper/cef_closure_task.h"

#include "zephyros.h"
#include "base/app.h"
#include "base/cef/client_app.h"
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "native_extensions/custom_url_manager.h"
#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"


/////////////////////////////////////////////////////////////////////////////////////////////
// Structures

typedef struct
{
	int x;
	int y;
	int w;
	int h;
} Rect;


/////////////////////////////////////////////////////////////////////////////////////////////
// Global Variables

// the global ClientHandler reference.
extern CefRefPtr<Zephyros::ClientHandler> g_handler;

// how the window is shown
int g_nMinWindowWidth = 0;
int g_nMinWindowHeight = 0;

GtkWidget* g_pMenuBar = NULL;

// height of the integrated menu bar (if any) at the top of the GTK window.
int g_nMenubarHeight = 0;

String g_strCustomUrlToAdd(TEXT(""));

extern std::ofstream* g_pLogFile;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

void CreateMainWindow(int argc, char** argv);

int XErrorHandlerImpl(Display *display, XErrorEvent *event);
int XIOErrorHandlerImpl(Display *display);

void OnDestroy(GtkWidget* widget, gpointer data);
gboolean OnDeleteEvent(GtkWidget* widget, GdkEvent* event, GtkWindow* window);
void OnTerminate(int nSignal);
void OnVboxSizeAllocated(GtkWidget* widget, GtkAllocation* allocation, void* data);
void OnMenubarSizeAllocated(GtkWidget* widget, GtkAllocation* allocation, void* data);
gboolean OnWindowFocusIn(GtkWidget* widget, GdkEventFocus* event, gpointer user_data);
gboolean OnWindowState(GtkWidget* widget, GdkEventWindowState* event, gpointer user_data);
gboolean OnWindowConfigure(GtkWindow* window, GdkEvent* event, gpointer data);

void LoadWindowPlacement(Rect* pRectNormal, bool* pbIsMaximized);
void SaveWindowPlacement(Rect* pRectNormal, bool bIsMaximized);
void AdjustWindowPlacementToMonitor(Rect* pRect);


namespace Zephyros {
namespace App {

std::ofstream* OpenLogFile();

} // namespace App
} // namespace Zephyros


/////////////////////////////////////////////////////////////////////////////////////////////
// Implementation

namespace Zephyros {

int RunApplication(int argc, char* argv[])
{
	// open the log file for writing
	g_pLogFile = Zephyros::App::OpenLogFile();

    // create a copy of "argv" on Linux because Chromium mangles the value internally (see issue #620)
    CefScopedArgArray scoped_arg_array(argc, argv);
    char** argv_copy = scoped_arg_array.array();

	CefMainArgs main_args(argc, argv);
	CefRefPtr<ClientApp> app(new ClientApp());

	// execute the secondary process, if any
	int nExitCode = CefExecuteProcess(main_args, app.get(), NULL);
	if (nExitCode >= 0)
	{
		Zephyros::Shutdown();
		return nExitCode;
	}

	// parse command line arguments
	Zephyros::App::InitCommandLine(argc, argv_copy);

	// populate the settings based on command line arguments
	CefSettings settings;
	Zephyros::App::GetSettings(settings);

	// set the user agent
    CefString(&settings.user_agent).FromASCII(Zephyros::App::GetUserAgent().c_str());

    // install xlib error handlers so that the application won't be terminated on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

	// initialize CEF
	CefInitialize(main_args, settings, app.get(), NULL);

    gtk_init(&argc, &argv);

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
	if (pMgr == NULL || pMgr->CanStartApp())
		CreateMainWindow(argc, argv_copy);

	// shut down CEF
	CefShutdown();
	Zephyros::Shutdown();

	return 0;
}

} // namespace Zephyros


void CreateMainWindow(int argc, char** argv)
{
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    g_signal_connect(window, "focus-in-event", G_CALLBACK(OnWindowFocusIn), NULL);
    g_signal_connect(window, "window-state-event", G_CALLBACK(OnWindowState), NULL);
    g_signal_connect(G_OBJECT(window), "configure-event", G_CALLBACK(OnWindowConfigure), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    g_signal_connect(vbox, "size-allocate", G_CALLBACK(OnVboxSizeAllocated), NULL);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_pMenuBar = gtk_menu_bar_new();
    g_signal_connect(g_pMenuBar, "size-allocate", G_CALLBACK(OnMenubarSizeAllocated), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), g_pMenuBar, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(OnDestroy), NULL);
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(OnDeleteEvent), window);

    // create the handler
    g_handler = new Zephyros::ClientHandler();
    g_handler->SetMainHwnd(vbox);

    CefWindowInfo info;
    CefBrowserSettings settings;
    settings.web_security = STATE_DISABLED;

    // show the GTK window
    gtk_widget_show_all(GTK_WIDGET(window));

    // the GTK window must be visible before we can retrieve the XID
    ::Window xwindow = GDK_WINDOW_XID(gtk_widget_get_window(window));
    info.SetAsChild(xwindow, CefRect(0, 0, 800, 600));

    // create the browser window
    CefBrowserHost::CreateBrowserSync(info, g_handler.get(), Zephyros::GetAppURL(), settings, NULL);

    // install a signal handler so we clean up after ourselves
    signal(SIGINT, OnTerminate);
    signal(SIGTERM, OnTerminate);

    CefRunMessageLoop();

	Zephyros::OSUtil::CleanUp();
}

int XErrorHandlerImpl(Display *display, XErrorEvent *event)
{
    LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
    return 0;
}

int XIOErrorHandlerImpl(Display *display)
{
    return 0;
}

void OnDestroy(GtkWidget* widget, gpointer data)
{
    // quitting CEF is handled in ClientHandler::OnBeforeClose()
}

gboolean OnDeleteEvent(GtkWidget* widget, GdkEvent* event, GtkWindow* window)
{
    if (g_handler.get() && !g_handler->IsClosing())
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser.get())
        {
            // notify the browser window that we would like to close it
            // this will result in a call to ClientHandler::DoClose() if the
            // JavaScript 'onbeforeunload' event handler allows it.
            browser->GetHost()->CloseBrowser(false);

            // cancel the close
            return TRUE;
        }
    }

    // allow the close
    return FALSE;
}

void OnTerminate(int nSignal)
{
    Zephyros::App::QuitMessageLoop();
}

void OnVboxSizeAllocated(GtkWidget* widget, GtkAllocation* allocation, void* data)
{
    if (g_handler)
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();

        // size the browser window to match the GTK widget
        if (browser && !browser->GetHost()->IsWindowRenderingDisabled())
        {
            ::Display* xdisplay = cef_get_xdisplay();
            ::Window xwindow = browser->GetHost()->GetWindowHandle();

            XWindowChanges changes = {0};
            changes.width = allocation->width;
            changes.height = allocation->height - g_nMenubarHeight;
            changes.y = g_nMenubarHeight;

            XConfigureWindow(xdisplay, xwindow, CWHeight | CWWidth | CWY, &changes);
        }
    }
}

void OnMenubarSizeAllocated(GtkWidget* widget, GtkAllocation* allocation, void* data)
{
    g_nMenubarHeight = allocation->height;
}

gboolean OnWindowFocusIn(GtkWidget* widget, GdkEventFocus* event, gpointer user_data)
{
    if (g_handler && event->in)
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser)
        {
            browser->GetHost()->SetFocus(true);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean OnWindowState(GtkWidget* widget, GdkEventWindowState* event, gpointer user_data)
{
    if (!(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED))
        return TRUE;

    if (!g_handler)
        return TRUE;

    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (!browser)
        return TRUE;

    const bool bIsIconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;
    if (browser->GetHost()->IsWindowRenderingDisabled())
    {
        // notify the off-screen browser that it was shown or hidden
        browser->GetHost()->WasHidden(bIsIconified);
    }
    else
    {
        // forward the state change event to the browser window
        ::Display* xdisplay = cef_get_xdisplay();
        ::Window xwindow = browser->GetHost()->GetWindowHandle();

        // retrieve the atoms required by the below XChangeProperty call
        const char* kAtoms[] = {
            "_NET_WM_STATE",
            "ATOM",
            "_NET_WM_STATE_HIDDEN"
        };

        Atom atoms[3];
        int nResult = XInternAtoms(xdisplay, const_cast<char**>(kAtoms), 3, false, atoms);
        if (!nResult)
            NOTREACHED();

        if (bIsIconified)
        {
            // set the hidden property state value
            scoped_ptr<Atom[]> data(new Atom[1]);
            data[0] = atoms[2];

            XChangeProperty(
                xdisplay,
                xwindow,
                atoms[0],   // name
                atoms[1],   // type
                32,         // size in bits of items in 'value'
                PropModeReplace,
                reinterpret_cast<const unsigned char*>(data.get()),
                1           // num items
            );
        }
        else
        {
            // set an empty array of property state values
            XChangeProperty(
                xdisplay,
                xwindow,
                atoms[0],   // name
                atoms[1],   // type
                32,         // size in bits of items in 'value'
                PropModeReplace,
                NULL,
                0           // num items
            );
        }
    }

    return TRUE;
}

gboolean OnWindowConfigure(GtkWindow* window, GdkEvent* event, gpointer data)
{
    // called when size, position or stack order changes
    if (g_handler)
    {
        CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
        if (browser)
        {
            // notify the browser of move/resize events so that:
            // * Popup windows are displayed in the correct location and dismissed when the window moves
            // * Drag&drop areas are updated accordingly
            browser->GetHost()->NotifyMoveOrResizeStarted();
        }
    }

    // don't stop this message
    return FALSE;
}

void LoadWindowPlacement(Rect* pRectNormal, bool* pbIsMaximized)
{
    // TODO

/*
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
	}*/
}

void SaveWindowPlacement(Rect* pRectNormal, bool bIsMaximized)
{
    // TODO

/*
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, Zephyros::GetWindowsInfo().szRegistryKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
	{
		StringStream ss;
		ss << pRectNormal->x << TEXT(',') << pRectNormal->y << TEXT(',') << pRectNormal->w << TEXT(',') << pRectNormal->h << TEXT(',') << showCmd;
		String data = ss.str();

		RegSetValueEx(hKey, TEXT("window"), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (DWORD) ((data.length() + 1) * sizeof(TCHAR)));
		RegCloseKey(hKey);
	}*/
}

void AdjustWindowPlacementToMonitor(Rect* pRect)
{
    // TODO

/*
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
		pRect->y = info.rcWork.bottom - pRect->h;*/
}


