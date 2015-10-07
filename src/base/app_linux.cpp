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

#include <gtk/gtk.h>

#include "lib/cef/include/cef_app.h"

#include "zephyros.h"
#include "base/app.h"
#include "base/cef/client_handler.h"

#include "native_extensions/os_util.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;

std::ofstream* g_pLogFile = NULL;


namespace Zephyros {
namespace App {

String GetUserAgent()
{
    StringStream ssUserAgent;

    ssUserAgent << Zephyros::GetAppName() << " " << Zephyros::GetAppVersion() << "; " << Zephyros::OSUtil::GetOSVersion();

    bool isLangAdded = false;

    // TODO: get system language

    if (!isLangAdded)
    {
        // default language if no languages are found
        ssUserAgent << "en";
    }

    return ssUserAgent.str();
}

void Quit()
{
	// close the app's main window
	// TODO

	if (g_pLogFile != NULL)
	{
        g_pLogFile->close();
        delete g_pLogFile;
        g_pLogFile = NULL;
    }
}

void QuitMessageLoop()
{
    CefQuitMessageLoop();

	if (g_pLogFile != NULL)
	{
        g_pLogFile->close();
        delete g_pLogFile;
        g_pLogFile = NULL;
    }
}

void BeginWait()
{
    // TODO
}

void EndWait()
{
    // TODO
}

void Alert(String title, String msg, AlertStyle style)
{
    GtkMessageType type = GTK_MESSAGE_INFO;
    switch (style)
    {
    case AlertWarning:
        type = GTK_MESSAGE_WARNING;
        break;
    case AlertError:
        type = GTK_MESSAGE_WARNING;
        break;
    }

    GtkWidget* pDialog = gtk_message_dialog_new(
        GTK_WINDOW(App::GetMainHwnd()),
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        title.c_str()
    );

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pDialog), msg.c_str());

    gtk_dialog_run (GTK_DIALOG(pDialog));
    gtk_widget_destroy(pDialog);
}

std::ofstream* OpenLogFile()
{
    String strFilename(OSUtil::GetConfigDirectory());
	strFilename.append(TEXT("/debug.log"));

    std::ofstream* pFile = new std::ofstream();
    pFile->open(strFilename, std::ios::out);

    return pFile;
}

void Log(String msg)
{
    *g_pLogFile << msg << std::endl;
}

void SetMenuItemStatuses(MenuItemStatuses& statuses)
{
	if (!g_handler.get())
		return;

    // TODO
    /*
	HMENU hMenu = GetMenu(g_handler->GetMainHwnd());
	if (!hMenu)
		return;
    */

    for (MenuItemStatuses::iterator it = statuses.begin(); it != statuses.end(); ++it)
    {
        int nID = Zephyros::GetMenuIDForCommand(it->first.c_str());
        if (nID)
        {
            int nStatus = it->second;
			/*
			EnableMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x01) ? MF_ENABLED : MF_DISABLED));
			CheckMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x02) ? MF_CHECKED : MF_UNCHECKED));
			*/
        }
    }
}

} // namespace App
} // namespace Zephyros
