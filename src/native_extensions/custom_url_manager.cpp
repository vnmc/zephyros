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


#ifdef OS_WIN
#include <tchar.h>
#endif

#ifdef USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#endif

#include "native_extensions.h"

#include "native_extensions/custom_url_manager.h"
#include "native_extensions/path.h"


namespace Zephyros {
    
CustomURLManager::CustomURLManager()
    : m_e(NULL)
{
}

void CustomURLManager::AddURL(String url)
{
#ifndef APPSTORE
    if (Zephyros::GetLicenseManager() != NULL && Zephyros::GetLicenseManager()->IsLicensingLink(url))
        Zephyros::GetLicenseManager()->ActivateFromURL(url);
	else
#endif
		m_urls.push_back(url);
}

void CustomURLManager::FireCustomURLs()
{
	if (!m_e)
		return;

	for (String url : m_urls)
	{
        Zephyros::JavaScript::Array args = Zephyros::JavaScript::CreateArray();
		args->SetString(0, url);
		m_e->InvokeCallbacks(TEXT("onCustomURL"), args);
	}

	m_urls.clear();
}

} // namespace Zephyros