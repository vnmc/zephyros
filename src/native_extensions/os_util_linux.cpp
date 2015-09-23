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


#include <vector>
#include <map>

#include <sys/utsname.h>

#include "base/app.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "native_extensions/os_util.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;

extern int g_nMinWindowWidth;
extern int g_nMinWindowHeight;


namespace Zephyros {
namespace OSUtil {

String GetOSVersion()
{
    utsname info;
    uname(&info);

    StringStream ss;
    ss << info.sysname << TEXT("-") << info.version;

    return ss.str();
}

String GetUserName()
{
	// TODO: implement
	return TEXT("");
}

String GetHomeDirectory()
{
	// TODO: implement
	return TEXT("");
}

String GetComputerName()
{
	// TODO: implement
	return TEXT("");
}

void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
    // TODO: implement
}

String Exec(String command)
{
    FILE* pipe = popen(command.c_str(), TEXT("r"));
    if (!pipe)
    	return TEXT("");

    char buffer[128];
    StringStream result;

    while (!feof(pipe))
    {
    	if (fgets(buffer, 128, pipe) != NULL)
    		result << buffer;
    }

    pclose(pipe);
    return result.str();
}

MenuHandle CreateContextMenu(JavaScript::Array menuItems)
{
    // TODO: implement
	return (MenuHandle) 0;
}

String ShowContextMenu(MenuHandle nMenuHandle, int x, int y)
{
    // TODO: implement
    return TEXT("");
}


int lastOriginX = INT_MIN;
int lastOriginY = INT_MIN;
int lastX = -1;
int lastY = -1;
int lastWidth = -1;
int lastHeight = -1;

void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
    // TODO: implement
}


void SetMinimumWindowSize(int width, int height)
{
    // TODO: implement
}


void DisplayNotification(String title, String details)
{
    // TODO: implement
}

void RequestUserAttention()
{
    // TODO: implement
}

void CopyToClipboard(String text)
{
    // TODO: implement
}

void CleanUp()
{
}

} // namespace OSUtil
} // namespace Zephyros
