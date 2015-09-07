// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

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
