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


#include "base/types.h"

#include "zephyros.h"
#include "jsbridge.h"


namespace Zephyros {
namespace JavaScript {

String GetStringFromDictionary(Object dict, KeyType key)
{
    StringStream ss;
    switch (dict->GetType(key))
    {
        case VTYPE_BOOL:
            ss << (dict->GetBool(key) ? TEXT("true") : TEXT("false"));
            break;
        case VTYPE_INT:
            ss << dict->GetInt(key);
            break;
        case VTYPE_DOUBLE:
            ss << dict->GetDouble(key);
            break;
        case VTYPE_STRING:
            ss << String(dict->GetString(key));
            break;
        case VTYPE_LIST:
        {
            ss << TEXT("[");
            Array list = dict->GetList(key);
            size_t len = list->GetSize();
            for (size_t i = 0; i < len; ++i)
            {
                if (i > 0)
                    ss << TEXT(",");
                ss << TEXT("\"") << JSONEscape(list->GetString((int) i)) << TEXT("\"");
            }
            ss << TEXT("]");
        }
            break;
        case VTYPE_INVALID:
            ss << TEXT("invalid");
            break;
        case VTYPE_NULL:
            ss << TEXT("null");
            break;
        default:
            break;
    }

    return ss.str();
}

String JSONEscape(String s)
{
    StringStream ss;
    for (String::iterator it = s.begin(); it != s.end(); ++it)
    {
        if (*it == '"')
            ss << TEXT("\\\"");
        else
            ss << *it;
    }

    return ss.str();
}

bool HasType(int type, int expectedType)
{
    if (type == expectedType)
        return true;

    // treat "int" and "double" as synonyms for "number"
    if (expectedType == VTYPE_INT || expectedType == VTYPE_DOUBLE)
        return type == VTYPE_INT || type == VTYPE_DOUBLE;

    return false;
}

} // namespace JavaScript
} // namespace Zephyros
