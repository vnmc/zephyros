// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <sstream>

#include "lib/cef/include/cef_app.h"
#include "lib/cef/include/cef_application_mac.h"
#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/cef_runnable.h"

#include "base/zephyros_impl.h"
#include "base/app.h"

#include "base/cef/client_app.h"
//#include "client_handler.h"
//#include "resource_util.h"
//#include "string_util.h"
//#include "extension_handler.h"
//#include "licensing.h"


// the global ClientHandler reference
extern CefRefPtr<Zephyros::ClientHandler> g_handler;


namespace Zephyros {

int InitCEFApplication(int argc, const char* argv[])
{
    CefMainArgs main_args(argc, (char**) argv);
    CefRefPtr<Zephyros::ClientApp> app(new Zephyros::ClientApp);

    // execute the secondary process
    return CefExecuteProcess(main_args, app.get(), NULL);
}
    
} // namespace Zephyros
