// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <string.h>
#include <tchar.h>

#include "lib/Libcef/Include/cef_stream.h"
#include "lib/Libcef/Include/wrapper/cef_byte_read_handler.h"

#include "resource_util.h"
#include "resource.h"
#include "util.h"


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
		{ TEXT("css/screen.css"), 3002 },
		{ TEXT("js/mock-app.js"), 3003 },
		{ TEXT("js/zepto.js"), 3004 },
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
