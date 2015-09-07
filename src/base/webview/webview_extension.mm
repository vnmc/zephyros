//
//  webview_extension.cpp
//  GhostlabMac
//
//  Created by Matthias Christen on 10.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#include <Foundation/Foundation.h>
#include <stdint.h>

#include "base/app.h"
#include "base/jsbridge.h"
#include "base/webview/webview_extension.h"

#include "native_extensions/path.h"


extern JSContextRef g_ctx;


namespace Zephyros {


///////////////////////////////////////////////////////////////
// NativeFunction Implementation

NativeFunction::NativeFunction(Function fnx, ...)
    : m_paramTransform(NULL), m_fnxAllCallbacksCompleted(NULL)
{
    m_fnx = fnx;
    
    va_list vl;
    va_start(vl, fnx);
    for (int i = 0; ; i += 2)
    {
        int nType = va_arg(vl, int);
        if (nType == END_MARKER)
            break;
        
        m_argTypes.push_back(nType);
        m_argNames.push_back(va_arg(vl, TCHAR*));
    }
    va_end(vl);
}

NativeFunction::~NativeFunction()
{
    for (JSObjectRef pCallback : m_callbacks)
        JSValueUnprotect(g_ctx, pCallback);
    
    if (m_paramTransform != NULL)
        JSValueUnprotect(g_ctx, m_paramTransform);
}

/**
 * Calls the native function.
 */
int NativeFunction::Call(JavaScript::Array args)
{
//#ifndef NDEBUG
//    App::Log("NativeFunction::Call " + m_name);
//#endif
   
    // prepare the arguments for the native implementation
    // (call the paramTransform function if there is one)
    JavaScript::Array nativeArgs = NULL;
    if (m_paramTransform != NULL)
    {
        JSValueRef* transformArgs = args->AsArray();
        JSValueRef result = JSObjectCallAsFunction(g_ctx, m_paramTransform, NULL, args->GetSize(), transformArgs, NULL);
        
        if (JSValueIsObject(g_ctx, result))
            nativeArgs = JavaScript::CreateArray(JSValueToObject(g_ctx, result, NULL));
        else
            nativeArgs = args;

        args->FreeArray(transformArgs);
    }
    else
        nativeArgs = args;
    
   
    // check the number of arguments (first last argument is the (optional) callback)
    size_t argsCount = nativeArgs->GetSize();
    if (argsCount != m_argTypes.size() && argsCount != m_argTypes.size() + 1)
        return ERR_INVALID_PARAM_NUM;
    
    // check the argument types
    for (size_t i = 0; i < m_argTypes.size(); ++i)
        if (m_argTypes.at(i) != VTYPE_INVALID && !JavaScript::HasType(nativeArgs->GetType((int) i), m_argTypes.at(i)))
            return ERR_INVALID_PARAM_TYPES;
    
    // get the callback if there is one
    JSObjectRef callback = NULL;
    int idxLastArg = (int) argsCount - 1;
    if (idxLastArg >= 0 && nativeArgs->GetType(idxLastArg) == VTYPE_FUNCTION)
        callback = nativeArgs->GetFunction(idxLastArg);
    
    // call the native implementation
    JavaScript::Array returnValues = JavaScript::CreateArray();
    int ret = m_fnx(nativeArgs, returnValues, callback);
    
    // invoke the callback argument if there is one
    if (callback != NULL)
    {
        if (m_hasPersistentCallback)
            AddCallback(callback);
        else if (ret != RET_DELAYED_CALLBACK)
        {
            JSValueRef* retArgs = returnValues->AsArray();
            JSValueRef exception = NULL;
            JSObjectCallAsFunction(g_ctx, callback, NULL, returnValues->GetSize(), retArgs, &exception);
            returnValues->FreeArray(retArgs);
            
            if (exception != NULL)
            {
                JSObjectRef obj = JSValueToObject(g_ctx, exception, NULL);
                JavaScript::Object objException(new JavaScript::ObjectWrapper(obj));
                
                // call exception.toString()
                JSObjectRef objToStringFunc = objException->GetFunction("toString");
                JSValueRef valResult = JSObjectCallAsFunction(g_ctx, objToStringFunc, obj, 0, NULL, NULL);
                JSStringRef strRes = JSValueToStringCopy(g_ctx, valResult, NULL);
                String strMessage = JavaScript::JSStringToString(strRes);
                JSStringRelease(strRes);
                
                // get exception.line
                int line = objException->GetInt("line");
                
                // get exception.stack
                String strStack = objException->GetString("stack");
                
                /*
                JSStringRef script = JSStringCreateWithUTF8CString("arguments.callee");
                valResult = JSEvaluateScript(g_ctx, script, NULL, NULL, 0, NULL);
                strRes = JSValueToStringCopy(g_ctx, valResult, NULL);
                String strCallee = JavaScript::JSStringToString(strRes);
                JSStringRelease(strRes);
                JSStringRelease(script);
                 */

                NSLog(@"%s:%d\n%s", strMessage.c_str(), line, strStack.c_str());
            }
        }
    }
    
    return ret;
}

void NativeFunction::AddCallback(JSObjectRef objCallback)
{
    JSValueProtect(g_ctx, objCallback);
    m_callbacks.push_back(objCallback);
}
    
void NativeFunction::SetParamTransform(JSObjectRef paramTransform)
{
    m_paramTransform = paramTransform;
    JSValueProtect(g_ctx, m_paramTransform);
}


///////////////////////////////////////////////////////////////
// ClientExtensionHandler Implementation

ClientExtensionHandler::ClientExtensionHandler()
{
    Zephyros::GetNativeExtensions()->SetClientExtensionHandler(this);
}

ClientExtensionHandler::~ClientExtensionHandler()
{
}

/**
 * Add a native function callable from JavaScript.
 */
void ClientExtensionHandler::AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue, bool hasPersistentCallback, String customJavaScriptImplementation)
{
    fnx->m_name = name;
    fnx->m_hasPersistentCallback = hasPersistentCallback;
    
    if (customJavaScriptImplementation != "")
    {
        // Format:
        //     return <fnx-name>(<expr-1>, ..., <expr-N> [, <callback-fnx>])
        //
        // This calls the "native" implementation of <fnx-name> with arguments
        // and an optional callback function, which is passed the result(s).
        // <expr-1>, ..., <expr-N> can be any expressions referencing the parameters
        // passed to the NativeFunction object.
        // The identifier "callback" refers to the optional callback function passed
        // to the NativeFunction object as last parameter.
        //
        // In the default implementation (customJavaScriptImplementation == ""),
        // <expr-I> is the I-th argument passed to the NativeFunction object.
        //
        // Transform this to an array of arguments to fnx:
        //     [<expr-1>, ..., <expr-N>, <callback-fnx>*]
        //
        // Do this via JSC:
        // Create a function
        //     function(<argname-0>, ..., <argname-M>, callback) {
        //         var args = [];
        //         args.splice(0, 0, ``<expr-1>, ..., <expr-N>, <callback-fnx>``)
        //         return args;
        //     }
        // and call it from JSC, passing in the actual parameters passed in by the JavaScript app.
        // <argname-0>, ..., <argname-M> the argument names passed to the NativeFunction object.
        // The string within the "``" is the last part of customJavaScriptImplementation.
        
        NSString *strCustomJS = [NSString stringWithUTF8String: customJavaScriptImplementation.c_str()];
        NSError *err = nil;
        NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern: [NSString stringWithFormat: @"return\\s+%s\\s*\\((.+)", name.c_str()]
                                                                               options: 0
                                                                                 error: &err];
        NSString *jsCode = [regex stringByReplacingMatchesInString: strCustomJS
                                                           options: 0
                                                             range: NSMakeRange(0, strCustomJS.length)
                                                      withTemplate: @"var __args__=[]; __args__.splice(0,0,$1; return __args__;"];
        
        JSStringRef customFnx = JSStringCreateWithCFString((__bridge CFStringRef) jsCode);
        
        int numArgs = fnx->GetNumArgs() + 1;
        JSStringRef* args = new JSStringRef[numArgs];
        for (int i = 0; i < numArgs - 1; ++i)
            args[i] = JSStringCreateWithUTF8CString(fnx->GetArgName(i).c_str());
        args[numArgs - 1] = JSStringCreateWithUTF8CString("callback");
        
        fnx->SetParamTransform(JSObjectMakeFunction(g_ctx, NULL, numArgs, args, customFnx, NULL, 0, NULL));
        
        for (int i = 0; i < numArgs; ++i)
            JSStringRelease(args[i]);
        delete[] args;
        JSStringRelease(customFnx);
    }
    
    m_mapFunctions[name] = fnx;
}

bool ClientExtensionHandler::InvokeFunction(String functionName, JavaScript::Array args)
{
    // try to find the function object for the message name
    std::map<String, NativeFunction*>::iterator it = m_mapFunctions.find(functionName);
    if (it == m_mapFunctions.end())
        return false;
    
    // invoke the native function
    NativeFunction* fnx = it->second;
    int ret = fnx->Call(args);

    // throw an exception if there was an error
    if (ret > NO_ERROR)
        ThrowJavaScriptException(functionName, ret);
    
    return ret > NO_ERROR;
}

/**
 * Invokes the registred callback functions of the function named functionName
 * with arguments args.
 */
bool ClientExtensionHandler::InvokeCallbacks(String functionName, JavaScript::Array args)
{
    //NSLog(@"invoking callback %s", functionName.c_str());
    
    // try to find the function object for the message name
    std::map<String, NativeFunction*>::iterator it = m_mapFunctions.find(functionName);
    if (it == m_mapFunctions.end())
        return false;
    
    // invoke the callbacks
    NativeFunction* fnx = it->second;
    bool isCallbackCalled = false;
    JSValueRef* callbackArgs = args->AsArray();
    for (JSObjectRef pCallback : fnx->m_callbacks)
    {
        JSObjectCallAsFunction(g_ctx, pCallback, NULL, args->GetSize(), callbackArgs, NULL);
        isCallbackCalled = true;
    }
    args->FreeArray(callbackArgs);
    
    // all callbacks have been called
    if (fnx->m_fnxAllCallbacksCompleted != NULL)
        fnx->m_fnxAllCallbacksCompleted();
    
    return isCallbackCalled;
}

void ClientExtensionHandler::ThrowJavaScriptException(String functionName, int retval)
{
    String code = TEXT("throw new Error('");
    switch (retval)
    {
    case ERR_INVALID_PARAM_NUM:
        code.append(TEXT("Invalid number of parameters for function "));
        code.append(functionName);
        break;
    case ERR_INVALID_PARAM_TYPES:
        code.append(TEXT("Invalid parameter types for function "));
        code.append(functionName);
        break;
    }
    code.append(TEXT("')"));
    
    JSStringRef strCode = JSStringCreateWithUTF8CString(code.c_str());
    JSEvaluateScript(g_ctx, strCode, NULL, NULL, 0, NULL);
    JSStringRelease(strCode);
}
    
} // namespace Zephyros
