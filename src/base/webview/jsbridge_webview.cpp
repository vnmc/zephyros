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


#include <memory>
#include "base/types.h"


#define IMPLEMENT_METHODS(ClassName, K)                                  \
bool ClassName::GetBool(const K key)                                     \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsBoolean(g_ctx, value))                                  \
        return JSValueToBoolean(g_ctx, value);                           \
    return false;                                                        \
}                                                                        \
int ClassName::GetInt(const K key)                                       \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsNumber(g_ctx, value))                                   \
        return (int) JSValueToNumber(g_ctx, value, NULL);                \
    return 0;                                                            \
}                                                                        \
double ClassName::GetDouble(const K key)                                 \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsNumber(g_ctx, value))                                   \
        return JSValueToNumber(g_ctx, value, NULL);                      \
    return 0;                                                            \
}                                                                        \
String ClassName::GetString(const K key)                                 \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsString(g_ctx, value))                                   \
    {                                                                    \
        JSStringRef str = JSValueToStringCopy(g_ctx, value, NULL);       \
        String result = JSStringToString(str);                           \
        JSStringRelease(str);                                            \
        return result;                                                   \
    }                                                                    \
    return "";                                                           \
}                                                                        \
Object ClassName::GetDictionary(const K key)                             \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsObject(g_ctx, value))                                   \
        return CreateObject(JSValueToObject(g_ctx, value, NULL));        \
    return CreateObject();                                               \
}                                                                        \
Array ClassName::GetList(const K key)                                    \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsObject(g_ctx, value))                                   \
    {                                                                    \
        JSObjectRef obj = JSValueToObject(g_ctx, value, NULL);           \
        if (IsArray(obj))                                                \
            return CreateArray(obj);                                     \
    }                                                                    \
    return CreateArray();                                                \
}                                                                        \
JSObjectRef ClassName::GetFunction(const K key)                          \
{                                                                        \
    JSValueRef value = GetValue(key);                                    \
    if (JSValueIsObject(g_ctx, value))                                   \
    {                                                                    \
        JSObjectRef obj = JSValueToObject(g_ctx, value, NULL);           \
        if (JSObjectIsFunction(g_ctx, obj))                              \
            return obj;                                                  \
    }                                                                    \
    return NULL;                                                         \
}                                                                        \
                                                                         \
bool ClassName::SetNull(const K key)                                     \
{                                                                        \
    return SetValue(key, JSValueMakeNull(g_ctx));                        \
}                                                                        \
bool ClassName::SetBool(const K key, bool value)                         \
{                                                                        \
    return SetValue(key, JSValueMakeBoolean(g_ctx, value));              \
}                                                                        \
bool ClassName::SetInt(const K key, int value)                           \
{                                                                        \
    return SetValue(key, JSValueMakeNumber(g_ctx, value));               \
}                                                                        \
bool ClassName::SetDouble(const K key, double value)                     \
{                                                                        \
    return SetValue(key, JSValueMakeNumber(g_ctx, value));               \
}                                                                        \
bool ClassName::SetString(const K key, const String& value)              \
{                                                                        \
    JSStringRef strValue = JSStringCreateWithUTF8CString(value.c_str()); \
    bool result = SetValue(key, JSValueMakeString(g_ctx, strValue));     \
    JSStringRelease(strValue);                                           \
    return result;                                                       \
}                                                                        \
bool ClassName::SetDictionary(const K key, Object value)                 \
{                                                                        \
    return SetValue(key, value->AsJS());                                 \
}                                                                        \
bool ClassName::SetList(const K key, Array value)                        \
{                                                                        \
    return SetValue(key, value->AsJS());                                 \
}                                                                        \
bool ClassName::SetFunction(const K key, JSObjectRef value)              \
{                                                                        \
    return SetValue(key, (JSValueRef) value);                            \
}


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
    
IMPLEMENT_METHODS(ObjectWrapper, KeyType)

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

IMPLEMENT_METHODS(ArrayWrapper, int)
    
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

