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


#ifndef Zephyros_ExtensionHandler_h
#define Zephyros_ExtensionHandler_h
#pragma once


#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/cef_process_message.h"
#include "lib/cef/include/cef_v8.h"

#include "zephyros.h"
#include "native_extensions.h"

#include "base/types.h"
#include "base/cef/client_app.h"
#include "base/cef/client_handler.h"


#define INVOKE_CALLBACK TEXT("@invokeCallback")
#define CALLBACK_COMPLETED TEXT("@callbackCompleted")
#define THROW_EXCEPTION TEXT("@throwException")


namespace Zephyros {

class ClientCallback
{
public:
    ClientCallback(int32 messageId, CefRefPtr<CefBrowser> browser)
        : m_messageId(messageId), m_browser(browser), m_invokeJavaScriptCallbackCount(0)
    {
    }
    
    ~ClientCallback();
    
    void Invoke(String functionName, CefRefPtr<CefListValue> args);
    
    inline int32 GetMessageId()
    {
        return m_messageId;
    }
    
    inline int IncrementJavaScriptInvokeCallbackCount()
    {
        return ++m_invokeJavaScriptCallbackCount;
    }
    
    inline int GetJavaScriptInvokeCallbackCount()
    {
        return m_invokeJavaScriptCallbackCount;
    }
    
private:
    int32 m_messageId;
    CefRefPtr<CefBrowser> m_browser;
    int m_invokeJavaScriptCallbackCount;
};


class AppCallback
{
public:
    AppCallback(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> function)
        : m_context(context), m_function(function)
    {
    }

	~AppCallback()
	{
		m_context = NULL;
		m_function = NULL;
	}
    
    CefRefPtr<CefV8Context> GetContext()
    {
        return m_context;
    }
    
    CefRefPtr<CefV8Value> GetFunction()
    {
        return m_function;
    }
    
private:
    CefRefPtr<CefV8Context> m_context;
    CefRefPtr<CefV8Value> m_function;
};


// Handles the native implementation for the JavaScript app extension.
class AppExtensionHandler : public NativeJavaScriptFunctionAdder, public CefV8Handler, public ClientApp::RenderDelegate
{
public:
    AppExtensionHandler();
	~AppExtensionHandler();

    // NativeJavaScriptFunctionAdder Implementation
	virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT("")) override;
	String GetJavaScriptCode();

    // CefV8Handler Implementation
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;

    // RenderDelegate Implementation
    virtual void OnBrowserCreated(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser) override;
    virtual void OnContextReleased(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual bool OnProcessMessageReceived(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;
    
private:
    void AddCallback(CefRefPtr<CefV8Value> fnx);
    bool HasPersistentCallback(String functionName);
    void ThrowJavaScriptException(CefRefPtr<CefV8Context> context, CefString functionName, int retval);
    
private:
    String m_JavaScriptCode;
    std::map<String, bool> m_mapFunctionHasPersistentCallback;

    // map of message callbacks
    std::map<int32, AppCallback*> m_mapCallbacks;
    
    int32 m_messageId;
        
    IMPLEMENT_REFCOUNTING(AppExtensionHandler);
};
    
} // namespace Zephyros


#endif // Zephyros_ExtensionHandler_h
