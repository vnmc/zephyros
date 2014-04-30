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

#ifndef __Zephyros__webview_extension__
#define __Zephyros__webview_extension__


#include <map>

#include "types.h"
#include "native_extensions.h"


static const int NO_ERROR                   = 0;
static const int ERR_UNKNOWN                = 1;
static const int ERR_INVALID_PARAM_NUM      = 2;
static const int ERR_INVALID_PARAM_TYPES    = 3;
static const int RET_DELAYED_CALLBACK       = -1;


#define END_MARKER -999


typedef int (*Function)(ExtensionState* ext, JavaScript::Array args, JavaScript::Array ret, JSObjectRef callback);
typedef void (*CallbacksCompleteHandler)(ExtensionState* ext);


class NativeFunction
{
public:
    NativeFunction(Function fnx, ...);
    ~NativeFunction();
    
    int Call(ExtensionState* state, JavaScript::Array args);
    void AddCallback(JSObjectRef objCallback);
    
    int GetNumArgs()
    {
        return (int) m_argNames.size();
    }
    
    String GetArgName(int index)
    {
        return m_argNames.at(index);
    }
    
    void SetAllCallbacksCompletedHandler(CallbacksCompleteHandler fnxAllCallbacksCompleted)
    {
        m_fnxAllCallbacksCompleted = fnxAllCallbacksCompleted;
    }
    
    void SetParamTransform(JSObjectRef paramTransform)
    {
        m_paramTransform = paramTransform;
        JSValueProtect(g_ctx, m_paramTransform);
    }
    
private:
    // A function pointer to the native implementation
    Function m_fnx;
    
    // The argument transformation function (based on the custom JS implementation)
    JSObjectRef m_paramTransform;
    
    std::vector<int> m_argTypes;
    std::vector<String> m_argNames;
    
public:
    String m_name;
    bool m_hasPersistentCallback;
    std::vector<JSObjectRef> m_callbacks;
    
    // Function to invoke when all JavaScript callbacks have completed
    CallbacksCompleteHandler m_fnxAllCallbacksCompleted;
};


class NativeJavaScriptFunctionAdder
{
public:
    inline void AddNativeJavaScriptProcedure(String name, NativeFunction* fnx, String customJavaScriptImplementation = TEXT(""))
    {
        AddNativeJavaScriptFunction(name, fnx, false, false, customJavaScriptImplementation);
    }
    
    inline void AddNativeJavaScriptCallback(String name, NativeFunction* fnx, String customJavaScriptImplementation = TEXT(""))
    {
        AddNativeJavaScriptFunction(name, fnx, false, true, customJavaScriptImplementation);
    }
    
    virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT("")) = 0;    
};


class ClientExtensionHandler : public NativeJavaScriptFunctionAdder
{
public:
    ClientExtensionHandler();
    ~ClientExtensionHandler();
    
	virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT(""));
    
    bool InvokeFunction(String functionName, JavaScript::Array args);
	bool InvokeCallbacks(String functionName, JavaScript::Array args);
    void ThrowJavaScriptException(String functionName, int retval);
    
    inline ExtensionState* GetState()
    {
        return m_state;
    }

private:
    std::map<String, NativeFunction*> m_mapFunctions;
    ExtensionState* m_state;
};


#endif /* defined(__Zephyros__webview_extension__) */
