/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_Browser_h
#define Zephyros_Browser_h
#pragma once


#include <vector>
#include <string>

#include "base/types.h"

#include "jsbridge.h"


#define BROWSER_CODE_NO_EXE_FOUND 3001


namespace Zephyros {

class Browser
{
public:
    Browser(String name, String version, String identifier, String image, bool isDefaultBrowser, int statusCode = 0)
      : m_name(name),
        m_version(version),
        m_identifier(identifier),
        m_image(image),
        m_isDefaultBrowser(isDefaultBrowser),
        m_statusCode(statusCode)
    {
    }

    inline String GetName()
    {
        return m_name;
    }

    inline String GetVersion()
    {
        return m_version;
    }

    inline String GetIdentifier()
    {
        return m_identifier;
    }

    inline String GetImage()
    {
        return m_image;
    }

    inline bool IsDefaultBrowser()
    {
        return m_isDefaultBrowser;
    }

    inline int GetStatusCode()
    {
        return m_statusCode;
    }
    
    JavaScript::Object CreateJSRepresentation()
    {
        JavaScript::Object obj = JavaScript::CreateObject();

        obj->SetString("name", m_name);
        obj->SetString("version", m_version);
        obj->SetString("id", m_identifier);
        obj->SetString("image", m_image);
        obj->SetBool("isDefaultBrowser", m_isDefaultBrowser);
        obj->SetInt("statusCode", m_statusCode);

        return obj;
    }


private:
    String m_name;
    String m_version;
    String m_identifier;
    String m_image;
    bool m_isDefaultBrowser;
    int m_statusCode;
};


namespace BrowserUtil {

void FindBrowsers(std::vector<Browser*>** ppBrowsers);

Browser* GetDefaultBrowser(std::vector<Browser*>* pBrowsers);
Browser* GetBrowserForIdentifier(std::vector<Browser*>* pBrowsers, String identifier);
Browser* GetBrowserForUserAgent(std::vector<Browser*>* pBrowsers, JavaScript::Object userAgent);
Browser* GetBrowserFromJSRepresentation(std::vector<Browser*>* pBrowsers, JavaScript::Object obj);

bool OpenURLInBrowser(String url, Browser* browser);

bool IsKnownBrowser(String browserName);

void CleanUp(std::vector<Browser*>* pBrowsers);

} // namespace BrowserUtil
} // namespace Zephyros

#endif // Zephyros_Browser_h
