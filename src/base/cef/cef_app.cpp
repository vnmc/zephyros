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


#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <string>

#include "lib/cef/include/cef_app.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_command_line.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/cef_runnable.h"
#include "lib/cef/include/cef_web_plugin.h"

#include "base/app.h"

#include "base/cef/client_handler.h"


CefRefPtr<Zephyros::ClientHandler> g_handler;
CefRefPtr<CefCommandLine> g_command_line;


namespace Zephyros {
namespace App {

CefRefPtr<CefBrowser> GetBrowser()
{
    if (!g_handler.get())
        return NULL;
    return g_handler->GetBrowser();
}

ClientWindowHandle GetMainHwnd()
{
    if (!g_handler.get())
        return NULL;
    return g_handler->GetMainHwnd();
}

void InitCommandLine(int argc, const char* const* argv)
{
    g_command_line = CefCommandLine::CreateCommandLine();

#if defined(OS_WIN)
    g_command_line->InitFromString(::GetCommandLineW());
#else
    g_command_line->InitFromArgv(argc, argv);
#endif
}

// Returns the application command line object.
CefRefPtr<CefCommandLine> GetCommandLine()
{
    return g_command_line;
}

// Returns the application settings based on command line arguments.
void GetSettings(CefSettings& settings)
{
    DCHECK(g_command_line.get());
    if (!g_command_line.get())
        return;

	settings.no_sandbox = true;

#if defined(OS_WIN)
    settings.multi_threaded_message_loop = false;// g_command_line->HasSwitch(cefclient::kMultiThreadedMessageLoop);
#endif

//    CefString(&settings.cache_path) = g_command_line->GetSwitchValue(cefclient::kCachePath);

#ifndef NDEBUG
    // Specify a port to enable DevTools if one isn't already specified.
    if (!g_command_line->HasSwitch("remote-debugging-port"))
        settings.remote_debugging_port = 19384;
#endif
}

} // namespace App
} // namespace Zephyros
