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
extern bool g_isMessageLoopRunning;
extern bool g_isMultithreadedMessageLoop;
//extern HWND g_hMessageWnd;

TCHAR g_szLogFileName[MAX_PATH];


namespace Zephyros {
namespace App {

void Quit()
{
	// close the app's main window
}

void QuitMessageLoop()
{
	if (!g_isMessageLoopRunning)
		return;
/*
	if (g_isMultithreadedMessageLoop)
	{
		// running in multi-threaded message loop mode
		// need to execute PostQuitMessage on the main application thread
		DCHECK(g_hMessageWnd);
		PostMessage(g_hMessageWnd, WM_COMMAND, ID_QUIT, 0);
	}
	else
		CefQuitMessageLoop();
*/
}

void BeginWait()
{
}

void EndWait()
{
}

void Alert(String title, String msg, AlertStyle style)
{
}

/*
HANDLE OpenLogFile()
{
}*/

void Log(String msg)
{
}

String ShowErrorMessage()
{
}

void SetMenuItemStatuses(JavaScript::Object items)
{
	if (!g_handler.get())
		return;

    /*
	HMENU hMenu = GetMenu(g_handler->GetMainHwnd());
	if (!hMenu)
		return;
    */

	JavaScript::KeyList keys;
	items->GetKeys(keys);
	for (JavaScript::KeyType commandId : keys)
	{
		String strCmdId = commandId;
		int nID = Zephyros::GetMenuIDForCommand(strCmdId.c_str());
		if (nID)
		{
			int status = items->GetInt(commandId);
			/*
			EnableMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x01) ? MF_ENABLED : MF_DISABLED));
			CheckMenuItem(hMenu, nID, MF_BYCOMMAND | ((status & 0x02) ? MF_CHECKED : MF_UNCHECKED));
			*/
		}
	}
}

} // namespace App
} // namespace Zephyros
