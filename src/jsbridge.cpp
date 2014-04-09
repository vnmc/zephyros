//
//  jsbridge.cpp
//  GhostlabMac
//
//  Created by Matthias Christen on 12.12.13.
//  Copyright (c) 2013 Vanamco AG. All rights reserved.
//

#include "jsbridge.h"


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