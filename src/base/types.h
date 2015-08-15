#include <string>
#include <sstream>

#ifdef USE_CEF
#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/cef_values.h"
#endif


#ifdef OS_MACOSX

#define TCHAR        char
#define TEXT(string) string

#define _tprintf     printf
#define _tcscpy      strcpy
#define _tcscat      strcat
#define _tcslen      strlen
#define _ttoi		 atoi
#define _tcsnccmp    strncmp

typedef std::string String;
typedef std::stringstream StringStream;

#endif  // OS_MACOSX


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

#endif  // OS_WIN


namespace Zephyros {

class ClientExtensionHandler;

} // namespace Zephyros


#ifdef USE_CEF

#include "jsbridge_v8.h"

class ClientHandler;
class CefBrowser;

typedef CefRefPtr<ClientHandler> ClientHandlerPtr;
typedef CefRefPtr<CefBrowser> BrowserPtr;
typedef CefRefPtr<Zephyros::ClientExtensionHandler> ClientExtensionHandlerPtr;

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

typedef Zephyros::ClientExtensionHandler* ClientExtensionHandlerPtr;
typedef JSObjectRef CallbackId;

#endif  // USE_WEBVIEW


#include "logging.h"
