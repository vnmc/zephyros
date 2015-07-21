//
//  jsbridge_webview.cpp
//  GhostlabMac
//
//  Created by Matthias Christen on 10.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#include <memory>
#include "base/types.h"


extern JSContextRef g_ctx;


namespace Zephyros {
namespace JavaScript {

    
Object CreateObject(JSObjectRef obj)
{
    return Object(obj == NULL ? new ObjectWrapper() : new ObjectWrapper(obj));
}

Array CreateArray(JSObjectRef arr)
{
    return Array(arr == NULL ? new ArrayWrapper() : new ArrayWrapper(arr));
}

    
/////////////////////////////////////////////////////////////////
// ObjectWrapper Implementation
    
ObjectWrapper::ObjectWrapper()
{
    m_obj = JSObjectMake(g_ctx, NULL, NULL);
    JSValueProtect(g_ctx, m_obj);
}
    
ObjectWrapper::ObjectWrapper(JSObjectRef obj)
{
    m_obj = obj;
    JSValueProtect(g_ctx, m_obj);
}
    
ObjectWrapper::~ObjectWrapper()
{
    JSValueUnprotect(g_ctx, m_obj);
}
    
bool ObjectWrapper::HasKey(const KeyType& key)
{
    JSStringRef strKey = JSStringCreateWithUTF8CString(key.c_str());
    bool hasProperty = JSObjectHasProperty(g_ctx, m_obj, strKey);
    JSStringRelease(strKey);
    
    return hasProperty;
}
    
bool ObjectWrapper::GetKeys(KeyList& keys)
{
    JSPropertyNameArrayRef names = JSObjectCopyPropertyNames(g_ctx, m_obj);
    size_t len = JSPropertyNameArrayGetCount(names);
    
    for (size_t i = 0; i < len; ++i)
    {
        JSStringRef name = JSPropertyNameArrayGetNameAtIndex(names, i);
        keys.push_back(JSStringToString(name));
    }
    
    JSPropertyNameArrayRelease(names);
    
    return true;
}
    
bool ObjectWrapper::Remove(const KeyType& key)
{
    JSStringRef strKey = JSStringCreateWithUTF8CString(key.c_str());
    JSObjectDeleteProperty(g_ctx, m_obj, strKey, NULL);
    JSStringRelease(strKey);
    
    return true;
}

JSValueRef ObjectWrapper::GetValue(const KeyType key)
{
    JSStringRef strKey = JSStringCreateWithUTF8CString(key.c_str());
    JSValueRef value = JSObjectGetProperty(g_ctx, m_obj, strKey, NULL);
    JSStringRelease(strKey);

    return value;
}

bool ObjectWrapper::SetValue(const KeyType key, JSValueRef value)
{
    JSStringRef strKey = JSStringCreateWithUTF8CString(key.c_str());
    JSObjectSetProperty(g_ctx, m_obj, strKey, value, kJSPropertyAttributeNone, NULL);
    JSStringRelease(strKey);
    
    return true;
}
    
    
    
/////////////////////////////////////////////////////////////////
// ArrayWrapper Implementation

ArrayWrapper::ArrayWrapper()
{
    m_arr = JSObjectMakeArray(g_ctx, 0, NULL, NULL);
    JSValueProtect(g_ctx, m_arr);
}
    
ArrayWrapper::ArrayWrapper(JSObjectRef array)
{
    m_arr = array;
    JSValueProtect(g_ctx, m_arr);
}
    
ArrayWrapper::~ArrayWrapper()
{
    JSValueUnprotect(g_ctx, m_arr);
}
    
bool ArrayWrapper::SetSize(size_t size)
{
    size_t len = GetSize();
    return Splice(size, len - size);
}

size_t ArrayWrapper::GetSize()
{
    JSStringRef strLength = JSStringCreateWithUTF8CString("length");
    JSValueRef valLength = JSObjectGetProperty(g_ctx, m_arr, strLength, NULL);
    size_t len = (size_t) JSValueToNumber(g_ctx, valLength, NULL);
    JSStringRelease(strLength);

    return len;
}

bool ArrayWrapper::Clear()
{
    return Splice(0, GetSize());
}

bool ArrayWrapper::Remove(int index)
{
    return Splice(index, 1);
}

bool ArrayWrapper::Splice(size_t index, size_t howMany)
{
    JSStringRef strSplice = JSStringCreateWithUTF8CString("splice");
    JSValueRef valSpliceFunc = JSObjectGetProperty(g_ctx, m_arr, strSplice, NULL);
    JSObjectRef objSpliceFunc = JSValueToObject(g_ctx, valSpliceFunc, NULL);
    
    JSValueRef args[2];
    args[0] = JSValueMakeNumber(g_ctx, index);
    args[1] = JSValueMakeNumber(g_ctx, howMany);
    
    JSObjectCallAsFunction(g_ctx, objSpliceFunc, m_arr, 2, args, NULL);
    
    return true;
}

JSValueRef ArrayWrapper::GetValue(int index)
{
    return JSObjectGetPropertyAtIndex(g_ctx, m_arr, index, NULL);
}

bool ArrayWrapper::SetValue(int index, JSValueRef value)
{
    JSObjectSetPropertyAtIndex(g_ctx, m_arr, index, value, NULL);
    return true;
}

//
// Returns the contents of the array as an array of JSValueRef.
// The caller is responsible for deleting the array returned by this method.
//
JSValueRef* ArrayWrapper::AsArray()
{
    size_t len = GetSize();
    JSValueRef* array = new JSValueRef[len];
    
    for (size_t i = 0; i < len; ++i)
    {
        array[i] = JSObjectGetPropertyAtIndex(g_ctx, m_arr, (int) i, NULL);
        JSValueProtect(g_ctx, array[i]);
    }
    
    return array;
}
    
void ArrayWrapper::FreeArray(JSValueRef* array)
{
    size_t len = GetSize();
    for (size_t i = 0; i < len; ++i)
        JSValueUnprotect(g_ctx, array[i]);
    
    delete[] array;
}


/////////////////////////////////////////////////////////////////
// Helper Functions

Type GetType(JSValueRef value)
{
    if (JSValueIsNull(g_ctx, value))
        return VTYPE_NULL;
    if (JSValueIsBoolean(g_ctx, value))
        return VTYPE_BOOL;
    if (JSValueIsNumber(g_ctx, value))
        return VTYPE_DOUBLE;
    if (JSValueIsString(g_ctx, value))
        return VTYPE_STRING;
    
    if (JSValueIsObject(g_ctx, value))
    {
        JSObjectRef obj = JSValueToObject(g_ctx, value, NULL);
            
        if (JSObjectIsFunction(g_ctx, obj))
            return VTYPE_FUNCTION;
        if (IsArray(obj))
            return VTYPE_LIST;

        return VTYPE_DICTIONARY;
    }
        
    return VTYPE_INVALID;
}
    
bool IsArray(JSObjectRef obj)
{
    if (g_fnxIsArray == NULL)
    {
        JSStringRef fnScript = JSStringCreateWithUTF8CString("return arguments[0] instanceof Array");
        g_fnxIsArray = JSObjectMakeFunction(g_ctx, NULL, 0, NULL, fnScript, NULL, 0, NULL);
        JSValueProtect(g_ctx, g_fnxIsArray);
        JSStringRelease(fnScript);
    }
    
    JSValueRef isArray = JSObjectCallAsFunction(g_ctx, g_fnxIsArray, NULL, 1, (JSValueRef*) &obj, NULL);
    return JSValueToBoolean(g_ctx, isArray);
}
    
String JSStringToString(JSStringRef str)
{
    size_t len = JSStringGetMaximumUTF8CStringSize(str);
    char* buf = new char[len + 1];
    JSStringGetUTF8CString(str, buf, len + 1);
    
    String result(buf);
    delete[] buf;
    
    return result;
}
    
    
} // namespace JavaScript
} // namespace Zephyros

