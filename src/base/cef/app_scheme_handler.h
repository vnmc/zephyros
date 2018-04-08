/*******************************************************************************
 * Copyright (c) 2015-2018 Vanamco AG, http://www.vanamco.com
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


#include "lib/cef/include/cef_resource_handler.h"
#include "base/types.h"


namespace Zephyros {

class AppSchemeHandler : public CefResourceHandler
{
public:
    AppSchemeHandler();

    virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) OVERRIDE;
    virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& responseLength, CefString& redirectUrl) OVERRIDE;
    virtual void Cancel() OVERRIDE;
    virtual bool ReadResponse(void* dataOut, int bytesToRead, int& bytesRead, CefRefPtr<CefCallback> callback) OVERRIDE;
    
private:
    std::string m_data;
    String m_mimeType;
    size_t m_offset;
    int m_status;
    
    IMPLEMENT_REFCOUNTING(AppSchemeHandler);
};

class AppSchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
    virtual CefRefPtr<CefResourceHandler> Create(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& schemeName, CefRefPtr<CefRequest> request) OVERRIDE;
    
    IMPLEMENT_REFCOUNTING(AppSchemeHandlerFactory);
};

} // namespace Zephyros
