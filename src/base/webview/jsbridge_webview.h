//
//  jsbridge_webview.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.12.13.
//
//

#ifndef Ghostlab_jsbridge_webview_h
#define Ghostlab_jsbridge_webview_h


#include <vector>
#include <map>

#include <JavaScriptCore/JavaScriptCore.h>


// common methods of ObjectWrapper and ArrayWrapper
// (like a template but without templates so we don't need to provide
// the implementation of the methods to the user of the framework)
#define DECLARE_METHODS(K)                                                   \
    Type GetType(const K key) { return JavaScript::GetType(GetValue(key)); } \
                                                                             \
    bool GetBool(const K key);                                               \
    int GetInt(const K key);                                                 \
    double GetDouble(const K key);                                           \
    String GetString(const K key);                                           \
    Object GetDictionary(const K key);                                       \
    Array GetList(const K key);                                              \
    JSObjectRef GetFunction(const K key);                                    \
                                                                             \
    bool SetNull(const K key);                                               \
    bool SetBool(const K key, bool value);                                   \
    bool SetInt(const K key, int value);                                     \
    bool SetDouble(const K key, double value);                               \
    bool SetString(const K key, const String& value);                        \
    bool SetDictionary(const K key, Object value);                           \
    bool SetList(const K key, Array value);                                  \
    bool SetFunction(const K key, JSObjectRef value);


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


namespace Zephyros {
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

    
class ObjectWrapper
{
public:
    ObjectWrapper();
    ObjectWrapper(JSObjectRef obj);
    virtual ~ObjectWrapper();
    
    bool HasKey(const KeyType& key);
    bool GetKeys(KeyList& keys);
    
    bool Remove(const String& key);
    
    DECLARE_METHODS(KeyType)
    
    virtual JSObjectRef AsJS() { return m_obj; }
    
protected:
    virtual JSValueRef GetValue(const KeyType key);
    virtual bool SetValue(const KeyType key, JSValueRef value);

private:
    JSObjectRef m_obj;
};
    
    
class ArrayWrapper
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
    
    DECLARE_METHODS(int)
    
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
} // namespace Zephyros


#endif
