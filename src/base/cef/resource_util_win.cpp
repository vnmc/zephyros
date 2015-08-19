// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <string.h>
#include <tchar.h>

#include "include/cef_stream.h"
#include "include/wrapper/cef_byte_read_handler.h"

#include "base/cef/resource_util.h"
#include "base/cef/util.h"

#include "resource.h"


namespace {

LPBYTE g_szMainCSS = NULL;


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

int GetResourceId(const TCHAR* resource_name)
{
	// Map of resource labels to BINARY id values.
	static struct _resource_map {
		TCHAR* name;
		int id;
	} resource_map[] = {
		// @RESOURCE_MAPPING_START
		{ TEXT("index.html"), 3001 },
		{ TEXT("app/default-data/default-data.json"), 3002 },
		{ TEXT("app/i18n/load-strings.js"), 3003 },
		{ TEXT("app/i18n/strings_de.js"), 3004 },
		{ TEXT("app/i18n/strings_en.js"), 3005 },
		{ TEXT("app/lib/cm/codemirror.css"), 3006 },
		{ TEXT("app/lib/cm/codemirror.js"), 3007 },
		{ TEXT("app/lib/cm/javascript.js"), 3008 },
		{ TEXT("assets/icons.svg"), 3009 },
		{ TEXT("assets/elements/ghostie-64.png"), 3010 },
		{ TEXT("assets/elements/ghostlab.png"), 3011 },
		{ TEXT("assets/elements/ghostlab@2x.png"), 3012 },
		{ TEXT("assets/elements/ghost_anim.gif"), 3013 },
		{ TEXT("assets/elements/ghost_anim@2x.gif"), 3014 },
		{ TEXT("assets/elements/loader.gif"), 3015 },
		{ TEXT("assets/elements/loader@2x.gif"), 3016 },
		{ TEXT("assets/elements/settings_tabs_left.svg"), 3017 },
		{ TEXT("assets/elements/settings_tabs_right.svg"), 3018 },
		{ TEXT("assets/elements/browsericons/chrome.png"), 3019 },
		{ TEXT("assets/elements/browsericons/firefox.png"), 3020 },
		{ TEXT("assets/elements/browsericons/ie.png"), 3021 },
		{ TEXT("assets/elements/browsericons/konqueror.png"), 3022 },
		{ TEXT("assets/elements/browsericons/opera.png"), 3023 },
		{ TEXT("assets/elements/browsericons/safari.png"), 3024 },
		{ TEXT("assets/elements/browsericons/xbox.png"), 3025 },
		{ TEXT("assets/fonts/source-code-pro/sourcecodepro-bold-webfont.woff"), 3026 },
		{ TEXT("assets/fonts/source-code-pro/sourcecodepro-regular-webfont.woff"), 3027 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-Black.otf.woff"), 3028 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-BlackIt.otf.woff"), 3029 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-Bold.otf.woff"), 3030 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-BoldIt.otf.woff"), 3031 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-ExtraLight.otf.woff"), 3032 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-ExtraLightIt.otf.woff"), 3033 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-It.otf.woff"), 3034 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-Light.otf.woff"), 3035 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-LightIt.otf.woff"), 3036 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-Regular.otf.woff"), 3037 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-Semibold.otf.woff"), 3038 },
		{ TEXT("assets/fonts/source-sans-pro/SourceSansPro-SemiboldIt.otf.woff"), 3039 },
		// @RESOURCE_MAPPING_END
	};

	for (int i = 0; i < sizeof(resource_map) / sizeof(_resource_map); ++i)
		if (!_tcscmp(resource_map[i].name, resource_name))
			return resource_map[i].id;

	return 0;
}

}  // namespace


bool LoadBinaryResource(const TCHAR* resource_name, String& resource_data)
{
	int resource_id = GetResourceId(resource_name);
	if (resource_id == 0)
		return false;

	DWORD dwSize;
	LPBYTE pBytes;

	if (LoadBinaryResource(resource_id, dwSize, pBytes))
	{
		resource_data = String(reinterpret_cast<TCHAR*>(pBytes), dwSize);
		return true;
	}

	ASSERT(FALSE);  // The resource should be found.
	return false;
}

CefRefPtr<CefStreamReader> GetBinaryResourceReader(const TCHAR* resource_name)
{
	int resource_id = GetResourceId(resource_name);
	if (resource_id == 0)
		return NULL;

	DWORD dwSize;
	LPBYTE pBytes;

	if (LoadBinaryResource(resource_id, dwSize, pBytes))
	{
		// patch "_system-font_"
		if (_tcscmp(resource_name, TEXT("style/base.css")) == 0)
		{
			if (g_szMainCSS != NULL)
				pBytes = g_szMainCSS;
			else
			{
				char* ptr = strstr((char*) pBytes, "_system-font_");
				if (ptr != NULL)
				{
					g_szMainCSS = new BYTE[dwSize];
					memcpy(g_szMainCSS, pBytes, dwSize);
					ptr = (char*) (g_szMainCSS + ((LPBYTE) ptr - pBytes));
					pBytes = g_szMainCSS;

					LOGFONT logfont;
					SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &logfont, 0);
					char szFont[] = "             "; // strlen("_system-font_") + 1
					WideCharToMultiByte(CP_UTF8, 0, logfont.lfFaceName, (int) _tcslen(logfont.lfFaceName), (LPSTR) szFont, 13, NULL, NULL);
					for (int j = 0; j < 13; j++)
						ptr[j] = szFont[j];
				}
			}
		}

		return CefStreamReader::CreateForHandler(new CefByteReadHandler(pBytes, dwSize, NULL));
	}

	ASSERT(FALSE);  // The resource should be found.
	return NULL;
}

void FreeResources()
{
	if (g_szMainCSS != NULL)
		delete[] g_szMainCSS;
}
