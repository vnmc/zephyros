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

#ifndef Zephyros_jsbridge_webview_h
#define Zephyros_jsbridge_webview_h


#include <vector>
#include <map>
#include <JavaScriptCore/JavaScriptCore.h>


extern JSContextRef g_ctx;

enum Type {
    VTYPE_INVALID = 0,
    VTYPE_NULL,
    VTYPE_BOOL,
    VTYPE_INT,
    VTYPE_DOUBLE,
    VTYPE_STRING,
    VTYPE_BINARY,
    VTYPE_DICTIONARY,
    VTYPE_LIST,
    VTYPE_FUNCTION
};


namespace JavaScript {
    

typedef std::vector<String> KeyList;
typedef String KeyType;

    
class ObjectWrapper;
class ArrayWrapper;
    
    
typedef std::shared_ptr<ObjectWrapper> Object;
typedef std::shared_ptr<ArrayWrapper> Array;


Object CreateObject(JSObjectRef obj = NULL);
Array CreateArray(JSObjectRef obj = NULL);

static JSObjectRef g_fnxIsArray = NULL;


// Helper functions
Type GetType(JSValueRef value);
bool IsArray(JSObjectRef obj);
String JSStringToString(JSStringRef str);
    
    
template<class K> class WrapperBase
{
public:
    Type GetType(const K key)
    {
        return JavaScript::GetType(GetValue(key));
    }
    
    bool GetBool(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsBoolean(g_ctx, value))
            return JSValueToBoolean(g_ctx, value);
        return false;
    }
    
    int GetInt(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsNumber(g_ctx, value))
            return (int) JSValueToNumber(g_ctx, value, NULL);
        return 0;
    }
    
    double GetDouble(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsNumber(g_ctx, value))
            return JSValueToNumber(g_ctx, value, NULL);
        return 0;
    }
    
    String GetString(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsString(g_ctx, value))
        {
            JSStringRef str = JSValueToStringCopy(g_ctx, value, NULL);
            String result = JSStringToString(str);
            JSStringRelease(str);
            
            return result;
        }
        
        return "";
    }
    
    Object GetDictionary(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsObject(g_ctx, value))
            return CreateObject(JSValueToObject(g_ctx, value, NULL));
        return CreateObject();
    }
    
    Array GetList(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsObject(g_ctx, value))
        {
            JSObjectRef obj = JSValueToObject(g_ctx, value, NULL);
            if (IsArray(obj))
                return CreateArray(obj);
        }
        
        return CreateArray();
    }
    
    JSObjectRef GetFunction(const K key)
    {
        JSValueRef value = GetValue(key);
        if (JSValueIsObject(g_ctx, value))
        {
            JSObjectRef obj = JSValueToObject(g_ctx, value, NULL);
            if (JSObjectIsFunction(g_ctx, obj))
                return obj;
        }
        
        return NULL;
    }
    
    bool SetNull(const K key)
    {
        return SetValue(key, JSValueMakeNull(g_ctx));
    }
    
    bool SetBool(const K key, bool value)
    {
        return SetValue(key, JSValueMakeBoolean(g_ctx, value));
    }
    
    bool SetInt(const K key, int value)
    {
        return SetValue(key, JSValueMakeNumber(g_ctx, value));
    }
    
    bool SetDouble(const K key, double value)
    {
        return SetValue(key, JSValueMakeNumber(g_ctx, value));
    }
    
    bool SetString(const K key, const String& value)
    {
        JSStringRef strValue = JSStringCreateWithUTF8CString(value.c_str());
        bool result = SetValue(key, JSValueMakeString(g_ctx, strValue));
        JSStringRelease(strValue);
        
        return result;
    }
    
    bool SetDictionary(const K key, std::shared_ptr< WrapperBase<KeyType> > value)
    {
        return SetValue(key, value->AsJS());
    }
    
    bool SetList(const K key, std::shared_ptr< WrapperBase<int> > value)
    {
        return SetValue(key, value->AsJS());
    }
    
    bool SetFunction(const K key, JSObjectRef value)
    {
        return SetValue(key, (JSValueRef) value);
    }
    
    virtual JSObjectRef AsJS() = 0;
    
protected:
    virtual JSValueRef GetValue(const K key) = 0;
    virtual bool SetValue(const K key, JSValueRef value) = 0;
};

    
class ObjectWrapper : public WrapperBase<KeyType>
{
public:
    ObjectWrapper();
    ObjectWrapper(JSObjectRef obj);
    virtual ~ObjectWrapper();
    
    bool HasKey(const KeyType& key);
    bool GetKeys(KeyList& keys);
    
    bool Remove(const String& key);
    
    virtual JSObjectRef AsJS() { return m_obj; }
    
protected:
    virtual JSValueRef GetValue(const KeyType key);
    virtual bool SetValue(const KeyType key, JSValueRef value);

private:
    JSObjectRef m_obj;
};
    
    
class ArrayWrapper : public WrapperBase<int>
{
public:
    ArrayWrapper();
    ArrayWrapper(JSObjectRef array);
    virtual ~ArrayWrapper();
    
    bool SetSize(size_t size);
    size_t GetSize();
    bool Clear();
    bool Remove(int index);
    
    bool Splice(size_t index, size_t howMany);
    
    virtual JSObjectRef AsJS() { return m_arr; }
    JSValueRef* AsArray();
    void FreeArray(JSValueRef* array);
    
protected:
    virtual JSValueRef GetValue(const int index);
    virtual bool SetValue(const int index, JSValueRef value);

private:
    JSObjectRef m_arr;
};

    
} // namespace JavaScript


#endif
