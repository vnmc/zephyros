#ifdef OS_WIN
#include <tchar.h>
#endif

#ifdef USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#endif

#ifdef USE_WEBVIEW
#include "base/webview/webview_extension.h"
#endif

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