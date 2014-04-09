#include <string>
#include <sstream>

#ifndef USE_WEBVIEW
#include "../../../Lib/Libcef/Include/cef_base.h"
#include "../../../Lib/Libcef/Include/cef_values.h"
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
