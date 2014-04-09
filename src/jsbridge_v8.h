//
//  jsbridge_v8.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.12.13.
//
//

#ifndef Ghostlab_jsbridge_v8_h
#define Ghostlab_jsbridge_v8_h


namespace JavaScript {

    
typedef CefRefPtr<CefDictionaryValue> Object;
typedef CefRefPtr<CefListValue> Array;

typedef CefString KeyType;
typedef CefDictionaryValue::KeyList KeyList;


inline Object CreateObject()
{
    return CefDictionaryValue::Create();
}
    
inline Array CreateArray()
{
    return CefListValue::Create();
}
    
inline void FreeObject(Object obj) {}
inline void FreeArray(Array arr) {}

    
} // namespace JavaScript



#endif
