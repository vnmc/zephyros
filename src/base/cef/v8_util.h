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


#ifndef Zephyros_V8Util_h
#define Zephyros_V8Util_h
#pragma once


#include "lib/cef/include/cef_v8.h"
#include "base/types.h"


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


#endif // Zephyros_V8Util_h
