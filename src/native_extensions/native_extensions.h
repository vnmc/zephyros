//
//  native_extensions.h
//
//  Created by Matthias Christen on 05.09.13.
//
//

#ifndef __native_extensions_h
#define __native_extensions_h


#include <vector>
#include "base/types.h"


#ifdef USE_CEF

#define FUNC(code, ...) new NativeFunction( \
    [](CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefRefPtr<CefListValue> args, CefRefPtr<CefListValue> ret, CallbackId callback) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) [](CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser) code

#endif


#ifdef USE_WEBVIEW

#define FUNC(code, ...) new NativeFunction( \
    [](JavaScript::Array args, JavaScript::Array ret, CallbackId callback) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) []() code

#endif


#define ARG(type, name) ,type,TEXT(name)


namespace Zephyros {

class NativeJavaScriptFunctionAdder;
class ClientExtensionHandler;
class FileWatcher;
class CustomURLManager;
class Browser;
class Path;


class NativeExtensions
#ifdef USE_CEF
    : public CefBase
#endif
{
public:
    virtual void AddNativeExtensions(NativeJavaScriptFunctionAdder* extensionHandler) = 0;
    virtual void SetClientExtensionHandler(ClientExtensionHandlerPtr e);
    
    inline std::vector<Zephyros::Path>& GetDroppedURLs() { return m_droppedURLs; }
    inline CustomURLManager* GetCustomURLManager() { return m_customURLManager; }

protected:
    ClientExtensionHandlerPtr m_e;
    std::vector<Zephyros::Path> m_droppedURLs;
    CustomURLManager* m_customURLManager;

#ifdef USE_CEF
    IMPLEMENT_REFCOUNTING(NativeExtensions);
#endif
};

class DefaultNativeExtensions : public NativeExtensions
{
public:
    DefaultNativeExtensions();
    ~DefaultNativeExtensions();
    
    virtual void AddNativeExtensions(NativeJavaScriptFunctionAdder* extensionHandler);
    virtual void SetClientExtensionHandler(ClientExtensionHandlerPtr e);
    
public:
    Zephyros::FileWatcher* m_fileWatcher;
    std::vector<Zephyros::Browser*> m_browsers;
};
    
} // namespace Zephyros

#endif
