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


#include <algorithm>

#include "lib/cef/include/cef_browser.h"
#include "lib/cef/include/cef_callback.h"
#include "lib/cef/include/cef_frame.h"
#include "lib/cef/include/cef_resource_handler.h"
#include "lib/cef/include/cef_response.h"
#include "lib/cef/include/cef_request.h"
#include "lib/cef/include/cef_scheme.h"
#include "lib/cef/include/wrapper/cef_helpers.h"

#include "base/cef/local_scheme_handler.h"
#include "base/cef/mime_types.h"

#include "native_extensions/file_util.h"


namespace Zephyros {
    
int hexValue(TCHAR c)
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
            out << (TCHAR) ((hexValue(url.at(i + 1)) << 4) | (hexValue(url.at(i + 2))));
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
    

LocalSchemeHandler::LocalSchemeHandler()
    : m_size(0), m_offset(0)
{
}

bool LocalSchemeHandler::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();
        
    String url = request->GetURL();
    Error err;
    m_pData = NULL;
    
    // remove query and hash parts
    String::size_type posQuery = url.find(TEXT("?"));
    String::size_type posHash = url.find(TEXT("#"));
    String::size_type len = String::npos;

    if (posQuery != String::npos && posHash != String::npos)
        len = std::min(posQuery, posHash);
    else if (posQuery != String::npos)
        len = posQuery;
    else if (posHash != String::npos)
        len = posHash;
    
    // the URL is prefixed with "local://"
    FileUtil::ReadFileBinary(DecodeURL(url.substr(8, len - 8)), &m_pData, m_size, err);
    m_mimeType = GetMIMETypeForFilename(url);

    // indicate the headers are available
    callback->Continue();
    return true;
}
    
void LocalSchemeHandler::GetResponseHeaders(CefRefPtr<CefResponse> response, int64& responseLength, CefString& redirectUrl)
{
    CEF_REQUIRE_IO_THREAD();
        
    response->SetMimeType(m_mimeType);
    response->SetStatus(m_pData == NULL ? 404 : 200);

    responseLength = m_size;
}
    
void LocalSchemeHandler::Cancel()
{
    CEF_REQUIRE_IO_THREAD();
}
    
bool LocalSchemeHandler::ReadResponse(void* dataOut, int bytesToRead, int& bytesRead, CefRefPtr<CefCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();
        
    bool hasData = false;
    bytesRead = 0;
    
    if (m_pData != NULL)
    {
        if (m_offset < m_size)
        {
            // copy the next block of data into the buffer
            int transferSize = std::min(bytesToRead, m_size - m_offset);
            memcpy(dataOut, m_pData + m_offset, transferSize);
            m_offset += transferSize;
            
            bytesRead = transferSize;
            hasData = true;
        }
        
        if (m_offset >= m_size)
        {
            delete[] m_pData;
            m_pData = NULL;
        }
    }
    
    return hasData;
}


/**
 * Return a new scheme handler instance to handle the request.
 */
CefRefPtr<CefResourceHandler> LocalSchemeHandlerFactory::Create(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& schemeName, CefRefPtr<CefRequest> request)
{
    CEF_REQUIRE_IO_THREAD();
    return new LocalSchemeHandler();
}

} // namespace Zephyros
