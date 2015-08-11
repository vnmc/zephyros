// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <Foundation/Foundation.h>

#include "lib/cef/include/cef_app.h"

#include "base/types.h"
#include "base/app.h"
#include "base/cef/client_app.h"


bool g_isWindowLoaded = false;
bool g_isWindowBeingLoaded = true;


namespace Zephyros {
namespace App {
    
CefWindowHandle GetMainHwnd()
{
    return NULL;
}

void QuitMessageLoop()
{
}
    
void Alert(String title, String msg, AlertStyle style)
{
}
    
void BeginWait()
{
}
    
void EndWait()
{
}
    
void Log(String msg)
{
    NSLog(@"%s", msg.c_str());
}
    
void SetMenuItemStatuses(CefRefPtr<CefDictionaryValue> items)
{
}
    
} // namespace App
} // namespace Zephyros




//TODO put this in the right location
/*
// Process entry point.
int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
  
    CefRefPtr<CefApp> app(new ClientApp);

    // Execute the secondary process.
    return CefExecuteProcess(main_args, app);
}*/
