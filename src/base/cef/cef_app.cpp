// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

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
#include "base/cef/util.h"


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

CefWindowHandle GetMainHwnd()
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
    ASSERT(g_command_line.get());
    if (!g_command_line.get())
        return;
    
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
