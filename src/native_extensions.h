//
//  native_extensions.h
//
//  Created by Matthias Christen on 05.09.13.
//
//

#ifndef __native_extensions_h
#define __native_extensions_h


#include <vector>
#include "types.h"


#ifndef USE_WEBVIEW

#define FUNC(code, ...) new NativeFunction( \
    [](CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefRefPtr<ExtensionState> state, CefRefPtr<CefListValue> args, CefRefPtr<CefListValue> ret) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) [](CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefRefPtr<ExtensionState> state) code

#else

#define FUNC(code, ...) new NativeFunction( \
    [](ExtensionState* state, JavaScript::Array args, JavaScript::Array ret, JSObjectRef callback) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) [](ExtensionState* state) code

#endif


#define ARG(type, name) ,type,TEXT(name)


class NativeJavaScriptFunctionAdder;
class ClientExtensionHandler;
class Path;


void AddNativeExtensions(NativeJavaScriptFunctionAdder* extensionHandler);


class ExtensionState
#ifndef USE_WEBVIEW
    : public CefBase
#endif
{
public:
    ExtensionState();
    ~ExtensionState();
    void SetClientExtensionHandler(ClientExtensionHandlerPtr e);
    
private:
    ClientExtensionHandlerPtr m_e;

#ifndef USE_WEBVIEW
    IMPLEMENT_REFCOUNTING(ExtensionState);
#endif
};


#endif
