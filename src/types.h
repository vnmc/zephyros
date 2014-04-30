//
// Copyright (C) 2013-2014 Vanamco AG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#include <string>
#include <sstream>

#ifndef USE_WEBVIEW
#include "lib/Libcef/Include/cef_base.h"
#include "lib/Libcef/Include/cef_values.h"
#else
#endif


#if defined(OS_MACOSX)


#define TEXT(string) string
#define TCHAR char

#define _tprintf    printf
#define _tcscat     strcat
#define _tcslen     strlen

typedef std::string String;
typedef std::stringstream StringStream;


#elif defined(OS_WIN)

#ifdef _UNICODE
typedef std::wstring String;
typedef std::wstringstream StringStream;
#else
typedef std::string String;
typedef std::stringstream StringStream;
#endif


#endif


// dummy for translated text; implement when we're doing i18n
#define TTEXT TEXT


class ClientHandler;
class CefBrowser;
class ExtensionState;
class ClientExtensionHandler;


#ifndef USE_WEBVIEW

#include "jsbridge_v8.h"

typedef CefRefPtr<ClientHandler> ClientHandlerPtr;
typedef CefRefPtr<CefBrowser> BrowserPtr;
typedef CefRefPtr<ExtensionState> ExtensionStatePtr;
typedef CefRefPtr<ClientExtensionHandler> ClientExtensionHandlerPtr;

typedef CefWindowHandle WindowHandle;

#else

#ifdef __OBJC__
#import <Foundation/Foundation.h>
typedef NSView* WindowHandle;
#else
#include <objc/objc.h>
typedef id WindowHandle;
#endif

#include "jsbridge_webview.h"

typedef ClientExtensionHandler* ClientExtensionHandlerPtr;

#endif
