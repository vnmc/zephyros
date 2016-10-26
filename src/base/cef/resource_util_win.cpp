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


#include "lib/cef/include/base/cef_logging.h"
#include "lib/cef/include/cef_stream.h"
#include "lib/cef/include/wrapper/cef_byte_read_handler.h"

#include "zephyros.h"
#include "base/cef/resource_util.h"


bool LoadBinaryResource(int binaryId, DWORD &dwSize, LPBYTE &pBytes)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(binaryId), MAKEINTRESOURCE(256));
    if (hRes)
    {
        HGLOBAL hGlob = LoadResource(hInst, hRes);
        if (hGlob)
        {
            dwSize = SizeofResource(hInst, hRes);
            pBytes = (LPBYTE) LockResource(hGlob);
            if (dwSize > 0 && pBytes)
                return true;
        }
    }
    
    return false;
}

bool LoadBinaryResource(const TCHAR* szResourceName, String& resourceData)
{
    int resourceId = Zephyros::GetResourceID(szResourceName);
    if (resourceId == 0)
        return false;

    DWORD dwSize;
    LPBYTE pBytes;

    if (LoadBinaryResource(resourceId, dwSize, pBytes))
    {
        resourceData = String(reinterpret_cast<TCHAR*>(pBytes), dwSize);
        return true;
    }

    // the resource should be found
    NOTREACHED();
    return false;
}

CefRefPtr<CefStreamReader> GetBinaryResourceReader(const TCHAR* szResourceName)
{
    int resource_id = Zephyros::GetResourceID(szResourceName);
    if (resource_id == 0)
        return NULL;

    DWORD dwSize;
    LPBYTE pBytes;

    if (LoadBinaryResource(resource_id, dwSize, pBytes))
        return CefStreamReader::CreateForHandler(new CefByteReadHandler(pBytes, dwSize, NULL));

    // the resource should be found.
    NOTREACHED();
    return NULL;
}
