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


#ifndef Zephyros_Types_h
#define Zephyros_Types_h
#pragma once


#include <string>
#include <sstream>

#ifdef USE_CEF
#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/cef_values.h"
#endif


#ifdef OS_WIN

#ifdef _UNICODE
typedef std::wstring String;
typedef std::wstringstream StringStream;
#define TO_STRING std::to_wstring
#else
typedef std::string String;
typedef std::stringstream StringStream;
#define TO_STRING std::to_string
#endif

#else

#define _tprintf     printf
#define _tcscpy      strcpy
#define _tcscat      strcat
#define _tcslen      strlen
#define _ttoi		 atoi
#define _tcsnccmp    strncmp
#define _wtoi64      atoi

typedef std::string String;
typedef std::stringstream StringStream;

#endif  // OS_WIN



#ifdef USE_CEF

#include "base/cef/jsbridge_v8.h"

namespace Zephyros {
class ClientHandler;
}

class CefBrowser;

typedef CefRefPtr<Zephyros::ClientHandler> ClientHandlerPtr;
typedef CefRefPtr<CefBrowser> BrowserPtr;

#if defined(OS_LINUX)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

typedef CefWindowHandle WindowHandle;

typedef int CallbackId;

#endif  // USE_CEF


#ifdef USE_WEBVIEW

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
typedef NSView* WindowHandle;
#else
#include <objc/objc.h>
typedef id WindowHandle;
#endif

#include "jsbridge_webview.h"

#endif  // USE_WEBVIEW

#endif // Zephyros_Types_h