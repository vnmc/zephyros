/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#include "native_extensions/path.h"
#include "native_extensions/file_util.h"


namespace Zephyros {

Path::Path(String path)
    : m_path(path), m_urlWithSecurityAccessData(TEXT("")), m_hasSecurityAccessData(false)
{
	m_isDirectory = FileUtil::IsDirectory(path);
}
    
Path::Path(String path, String urlWithSecurityAccessData, bool hasSecurityAccessData)
    : m_path(path), m_urlWithSecurityAccessData(urlWithSecurityAccessData), m_hasSecurityAccessData(hasSecurityAccessData)
{
	m_isDirectory = FileUtil::IsDirectory(path);
}

Path::Path(JavaScript::Object jsonObj)
{
    m_path = jsonObj->GetString("path");
	m_isDirectory = FileUtil::IsDirectory(m_path);
    m_urlWithSecurityAccessData = jsonObj->GetString("url");
    m_hasSecurityAccessData = jsonObj->GetBool("hasSecurityAccessData");
}

JavaScript::Object Path::CreateJSRepresentation()
{
    JavaScript::Object obj = JavaScript::CreateObject();
        
    obj->SetString("path", m_path);
    obj->SetBool("isDirectory", m_isDirectory);
    obj->SetString("url", m_urlWithSecurityAccessData);
    obj->SetBool("hasSecurityAccessData", m_hasSecurityAccessData);
        
    return obj;
}

} // namespace Zephyros
