/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_JsbridgeV8_h
#define Zephyros_JsbridgeV8_h
#pragma once


template<class T> class CefRefPtr;
class CefListValue;
class CefDictionaryValue;


namespace Zephyros {
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
} // namespace Zephyros


#endif // Zephyros_JsbridgeV8_h
