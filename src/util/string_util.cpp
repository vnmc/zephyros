/*******************************************************************************
 * Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
 * reserved. Use of this source code is governed by a BSD-style license that
 * can be found in the LICENSE file.
 *******************************************************************************/


#include "util/string_util.h"
#include "zephyros.h"

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