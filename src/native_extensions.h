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
