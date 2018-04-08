/*******************************************************************************
 * Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
 * reserved. Use of this source code is governed by a BSD-style license that
 * can be found in the LICENSE file.
 *******************************************************************************/


#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>

#include "util/string_util.h"

#ifdef USE_CEF
#include "lib/cef/include/cef_request.h"


void DumpRequestContents(CefRefPtr<CefRequest> request, String& str)
{
    StringStream ss;

    ss << TEXT("URL: ") << String(request->GetURL());
    ss << TEXT("\nMethod: ") << String(request->GetMethod());

    CefRequest::HeaderMap headerMap;
    request->GetHeaderMap(headerMap);
    if (headerMap.size() > 0)
    {
        ss << TEXT("\nHeaders:");
        CefRequest::HeaderMap::const_iterator it = headerMap.begin();
        for (; it != headerMap.end(); ++it)
            ss << TEXT("\n\t") << String((*it).first) << TEXT(": ") << String((*it).second);
    }

    CefRefPtr<CefPostData> postData = request->GetPostData();
    if (postData.get())
    {
        CefPostData::ElementVector elements;
        postData->GetElements(elements);
        if (elements.size() > 0)
        {
            ss << TEXT("\nPost Data:");

            CefRefPtr<CefPostDataElement> element;
            CefPostData::ElementVector::const_iterator it = elements.begin();
            for (; it != elements.end(); ++it)
            {
                element = (*it);

                if (element->GetType() == PDE_TYPE_BYTES)
                {
                    // the element is composed of bytes
                    ss << TEXT("\n\tBytes: ");
                    if (element->GetBytesCount() == 0)
                        ss << TEXT("(empty)");
                    else
                    {
                        // retrieve the data
                        size_t size = element->GetBytesCount();
                        TCHAR* bytes = new TCHAR[size];
                        element->GetBytes(size, bytes);
                        ss << String(bytes, size);
                        delete [] bytes;
                    }
                }
                else if (element->GetType() == PDE_TYPE_FILE)
                    ss << TEXT("\n\tFile: ") << String(element->GetFile());
            }
        }
    }

    str = ss.str();
}
#endif


String StringReplace(const String& str, const String& from, const String& to)
{
    String result = str;
    String::size_type pos = 0;
    String::size_type from_len = from.length();
    String::size_type to_len = to.length();

    do
    {
        pos = result.find(from, pos);
        if (pos != String::npos)
        {
            result.replace(pos, from_len, to);
            pos += to_len;
        }
    } while (pos != String::npos);

    return result;
}

bool StringEndsWith(const String& str, const String& suffix)
{
    String::size_type lenStr = str.length();
    String::size_type lenSuffix = suffix.length();

    if (lenSuffix > lenStr)
        return false;
    return str.compare(lenStr - lenSuffix, lenSuffix, suffix) == 0;
}

std::vector<String>& Split(const String &s, TCHAR delim, std::vector<String> &elems)
{
    StringStream ss(s);
    String item;

    while (std::getline(ss, item, delim))
        elems.push_back(item);

    return elems;
}

std::vector<String> Split(const String &s, TCHAR delim)
{
    std::vector<String> elems;
    Split(s, delim, elems);
    return elems;
}

/**
 * Trim from start.
 */
String& LTrim(String &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

/**
 * Trim from end.
 */
String& RTrim(String &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

/**
 * Trim from both ends.
 */
String& Trim(String &s)
{
    return LTrim(RTrim(s));
}

String ToString(BYTE* data, const int len)
{
    int length = len;

    if (sizeof(TCHAR) == 2)
    {
        // make sure that the length is a multiple of 2
        // length = len - sizeof(TCHAR) = len - 2
        length = (len - 1) & ~1;
    }
    else
        length -= sizeof(TCHAR);

    return String(reinterpret_cast<TCHAR*>(data), reinterpret_cast<TCHAR*>(data + length));
}

String ToLower(String s)
{
    String ret = s;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return ret;
}

String ToUpper(String s)
{
    String ret = s;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    
    return ret;
}

int HexValue(TCHAR c)
{
    if (TEXT('0') <= c && c <= TEXT('9'))
        return c - TEXT('0');
    if (TEXT('a') <= c && c <= TEXT('f'))
        return c - TEXT('a') + 10;
    if (TEXT('A') <= c && c <= TEXT('F'))
        return c - TEXT('A') + 10;
    
    return 0;
}

String DecodeURL(String url)
{
    StringStream out;
    
    for (String::size_type i = 0; i < url.length(); ++i)
    {
        switch (url.at(i))
        {
            case TEXT('%'):
                out << (TCHAR) ((HexValue(url.at(i + 1)) << 4) | (HexValue(url.at(i + 2))));
                i += 2;
                break;
                
            case TEXT('+'):
                out << TEXT(' ');
                break;
                
            default:
                out << url.at(i);
                break;
        }
    }
    
    return out.str();
}
