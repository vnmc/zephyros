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


#include <sstream>

#include "lib/cef/include/base/cef_logging.h"

#include "base/app.h"
#include "base/cef/v8_util.h"


/**
 * Transfer a V8 value to a List index.
 */
void SetListValue(CefRefPtr<CefListValue> list, int index, CefRefPtr<CefV8Value> value)
{
    if (value->IsArray())
    {
        CefRefPtr<CefListValue> new_list = CefListValue::Create();
        SetList(value, new_list);
        list->SetList(index, new_list);
    }
    else if (value->IsObject() && !value->IsFunction())
    {
        CefRefPtr<CefDictionaryValue> new_dict = CefDictionaryValue::Create();
        SetDictionary(value, new_dict);
        list->SetDictionary(index, new_dict);
    }
    else if (value->IsString())
        list->SetString(index, value->GetStringValue());
    else if (value->IsBool())
        list->SetBool(index, value->GetBoolValue());
    else if (value->IsInt())
        list->SetInt(index, value->GetIntValue());
    else if (value->IsUInt())
        list->SetInt(index, value->GetUIntValue());
    else if (value->IsDouble())
        list->SetDouble(index, value->GetDoubleValue());
}

/**
 * Transfer a V8 array to a List.
 */
void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target)
{
    DCHECK(source->IsArray());

    int arg_length = source->GetArrayLength();
    if (arg_length == 0)
        return;

    // Start with null types in all spaces.
    target->SetSize(arg_length);

    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source->GetValue(i));
}

void SetDictionaryValue(CefRefPtr<CefDictionaryValue> dict, CefString key, CefRefPtr<CefV8Value> value)
{
    if (value->IsArray())
    {
        CefRefPtr<CefListValue> new_list = CefListValue::Create();
        SetList(value, new_list);
        dict->SetList(key, new_list);
    }
    else if (value->IsObject() && !value->IsFunction())
    {
        CefRefPtr<CefDictionaryValue> new_dict = CefDictionaryValue::Create();
        SetDictionary(value, new_dict);
        dict->SetDictionary(key, new_dict);
    }
    else if (value->IsBool())
        dict->SetBool(key, value->GetBoolValue());
    else if (value->IsString())
        dict->SetString(key, value->GetStringValue());
    else if (value->IsInt())
        dict->SetInt(key, value->GetIntValue());
    else if (value->IsUInt())
        dict->SetInt(key, value->GetUIntValue());
    else if (value->IsDouble())
        dict->SetDouble(key, value->GetDoubleValue());
    else if (value->IsNull())
        dict->SetNull(key);
}

/**
 * Transfer a V8 object (only JSON) to a dictionary.
 */
void SetDictionary(CefRefPtr<CefV8Value> source, CefRefPtr<CefDictionaryValue> target)
{
    DCHECK(source->IsObject());

    std::vector<CefString> keys;
    source->GetKeys(keys);

    for (CefString key : keys)
        SetDictionaryValue(target, key, source->GetValue(key));
}

/**
 * Transfer a List value to a V8 array index.
 */
void SetListValue(CefRefPtr<CefV8Value> list, int index, CefRefPtr<CefListValue> value)
{
    CefRefPtr<CefV8Value> new_value = ListValueToV8Value(value, index);
    if (new_value.get())
        list->SetValue(index, new_value);
    else
        list->SetValue(index, CefV8Value::CreateNull());
}

/**
 * Transfer a List to a V8 array.
 */
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target)
{
    DCHECK(target->IsArray());

    int arg_length = static_cast<int>(source->GetSize());
    if (arg_length == 0)
        return;

    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source);
}

void SetDictionaryValue(CefRefPtr<CefV8Value> obj, CefString key, CefRefPtr<CefDictionaryValue> value)
{
    CefRefPtr<CefV8Value> new_value = DictionaryValueToV8Value(value, key);
    if (new_value.get())
        obj->SetValue(key, new_value, CefV8Value::PropertyAttribute::V8_PROPERTY_ATTRIBUTE_NONE);
    else
        obj->SetValue(key, CefV8Value::CreateNull(), CefV8Value::PropertyAttribute::V8_PROPERTY_ATTRIBUTE_NONE);
}

/**
 * Transfer a dictionary to a V8 object.
 */
void SetDictionary(CefRefPtr<CefDictionaryValue> source, CefRefPtr<CefV8Value> target)
{
    DCHECK(target->IsObject());

    CefDictionaryValue::KeyList keys;
    source->GetKeys(keys);

    for (CefString key : keys)
        SetDictionaryValue(target, key, source);
}

CefRefPtr<CefV8Value> ListValueToV8Value(CefRefPtr<CefListValue> value, int index)
{
    CefRefPtr<CefV8Value> new_value;

    CefValueType type = value->GetType(index);
    switch (type)
    {
    case VTYPE_LIST:
        {
            CefRefPtr<CefListValue> list = value->GetList(index);
            new_value = CefV8Value::CreateArray((int) list->GetSize());
            SetList(list, new_value);
        }
        break;
    case VTYPE_DICTIONARY:
        {
            CefRefPtr<CefDictionaryValue> dict = value->GetDictionary(index);
            new_value = CefV8Value::CreateObject(CefRefPtr<CefV8Accessor>());
            SetDictionary(dict, new_value);
        }
        break;
    case VTYPE_BOOL:
        new_value = CefV8Value::CreateBool(value->GetBool(index));
        break;
    case VTYPE_DOUBLE:
        new_value = CefV8Value::CreateDouble(value->GetDouble(index));
        break;
    case VTYPE_INT:
        new_value = CefV8Value::CreateInt(value->GetInt(index));
        break;
    case VTYPE_STRING:
        new_value = CefV8Value::CreateString(value->GetString(index));
        break;
    default:
        new_value = CefV8Value::CreateNull();
        break;
    }

    return new_value;
}

CefRefPtr<CefV8Value> DictionaryValueToV8Value(CefRefPtr<CefDictionaryValue> value, CefString key)
{
    CefRefPtr<CefV8Value> new_value;

    CefValueType type = value->GetType(key);
    switch (type)
    {
    case VTYPE_LIST:
        {
            CefRefPtr<CefListValue> list = value->GetList(key);
            new_value = CefV8Value::CreateArray((int) list->GetSize());
            SetList(list, new_value);
        }
        break;
    case VTYPE_DICTIONARY:
        {
            CefRefPtr<CefDictionaryValue> dict = value->GetDictionary(key);
            new_value = CefV8Value::CreateObject(CefRefPtr<CefV8Accessor>());
            SetDictionary(dict, new_value);
        }
        break;
    case VTYPE_BOOL:
        new_value = CefV8Value::CreateBool(value->GetBool(key));
        break;
    case VTYPE_DOUBLE:
        new_value = CefV8Value::CreateDouble(value->GetDouble(key));
        break;
    case VTYPE_INT:
        new_value = CefV8Value::CreateInt(value->GetInt(key));
        break;
    case VTYPE_STRING:
        new_value = CefV8Value::CreateString(value->GetString(key));
        break;
    default:
        new_value = CefV8Value::CreateNull();
        break;
    }

    return new_value;
}

void CopyList(CefRefPtr<CefListValue> source, CefRefPtr<CefListValue> dest, int offset)
{
	int size = (int) source->GetSize();
    for (int i = 0; i < size; i++)
    {
        if (i + offset < 0)
            continue;

        switch (source->GetType(i))
        {
        case VTYPE_BINARY:
            dest->SetBinary(i + offset, source->GetBinary(i));
            break;
        case VTYPE_BOOL:
            dest->SetBool(i + offset, source->GetBool(i));
            break;
        case VTYPE_DICTIONARY:
            {
                CefRefPtr<CefDictionaryValue> dictCopy = CefDictionaryValue::Create();
                CopyDictionary(source->GetDictionary(i), dictCopy);
                dest->SetDictionary(i + offset, dictCopy);
            }
            break;
        case VTYPE_DOUBLE:
            dest->SetDouble(i + offset, source->GetDouble(i));
            break;
        case VTYPE_INT:
            dest->SetInt(i + offset, source->GetInt(i));
            break;
        case VTYPE_LIST:
            {
                CefRefPtr<CefListValue> listCopy = CefListValue::Create();
                CopyList(source->GetList(i), listCopy);
                dest->SetList(i + offset, listCopy);
            }
            break;
        case VTYPE_NULL:
            dest->SetNull(i + offset);
            break;
        case VTYPE_STRING:
            dest->SetString(i + offset, source->GetString(i));
            break;
        default:
            break;
        }
    }
}

void CopyDictionary(CefRefPtr<CefDictionaryValue> source, CefRefPtr<CefDictionaryValue> dest)
{
    CefDictionaryValue::KeyList keys;
    source->GetKeys(keys);

    for (CefString key : keys)
    {
        switch (source->GetType(key))
        {
        case VTYPE_BINARY:
            dest->SetBinary(key, source->GetBinary(key));
            break;
        case VTYPE_BOOL:
            dest->SetBool(key, source->GetBool(key));
            break;
        case VTYPE_DICTIONARY:
            {
                CefRefPtr<CefDictionaryValue> dictCopy = CefDictionaryValue::Create();
                CopyDictionary(source->GetDictionary(key), dictCopy);
                dest->SetDictionary(key, dictCopy);
            }
            break;
        case VTYPE_DOUBLE:
            dest->SetDouble(key, source->GetDouble(key));
            break;
        case VTYPE_INT:
            dest->SetInt(key, source->GetInt(key));
            break;
        case VTYPE_LIST:
            {
                CefRefPtr<CefListValue> listCopy = CefListValue::Create();
                CopyList(source->GetList(key), listCopy);
                dest->SetList(key, listCopy);
            }
            break;
        case VTYPE_NULL:
            dest->SetNull(key);
            break;
        case VTYPE_STRING:
            dest->SetString(key, source->GetString(key));
            break;
        default:
            break;
        }
    }
}
