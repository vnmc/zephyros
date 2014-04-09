//
//  v8_util.h
//
//  Created by Matthias Christen on 03.09.13.
//
//

#ifndef __v8_util__
#define __v8_util__

#include "..\..\Lib\Libcef\Include/cef_v8.h"
#include "types.h"


void SetListValue(CefRefPtr<CefListValue> list, int index, CefRefPtr<CefV8Value> value);
void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target);
void SetDictionaryValue(CefRefPtr<CefDictionaryValue> dict, CefString key, CefRefPtr<CefV8Value> value);
void SetDictionary(CefRefPtr<CefV8Value> source, CefRefPtr<CefDictionaryValue> target);

void SetListValue(CefRefPtr<CefV8Value> list, int index, CefRefPtr<CefListValue> value);
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target);
void SetDictionaryValue(CefRefPtr<CefV8Value> obj, CefString key, CefRefPtr<CefDictionaryValue> value);
void SetDictionary(CefRefPtr<CefDictionaryValue>, CefRefPtr<CefV8Value> target);

CefRefPtr<CefV8Value> ListValueToV8Value(CefRefPtr<CefListValue> value, int index);
CefRefPtr<CefV8Value> DictionaryValueToV8Value(CefRefPtr<CefDictionaryValue> value, CefString key);

void CopyList(CefRefPtr<CefListValue> source, CefRefPtr<CefListValue> dest, int offset = 0);
void CopyDictionary(CefRefPtr<CefDictionaryValue> source, CefRefPtr<CefDictionaryValue> dest);


#endif /* defined(__v8_util__) */
