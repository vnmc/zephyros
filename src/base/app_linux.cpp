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


#include "lib/cef/include/cef_app.h"

#include "zephyros.h"
#include "base/app.h"
#include "base/cef/client_handler.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;

FILE* g_hndLogFile;


namespace Zephyros {
namespace App {

void Quit()
{
	// close the app's main window
	// TODO
}

void QuitMessageLoop()
{
    CefQuitMessageLoop();
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
    // TODO
}

FILE* OpenLogFile()
{
    // TODO
    return NULL;
}

void Log(String msg)
{
    // TODO
}

String ShowErrorMessage()
{
    // TODO
    return TEXT("");
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
