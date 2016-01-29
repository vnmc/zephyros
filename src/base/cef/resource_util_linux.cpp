// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "lib/cef/include/base/cef_logging.h"
#include "lib/cef/include/cef_stream.h"
#include "lib/cef/include/wrapper/cef_byte_read_handler.h"

#include "base/cef/resource_util.h"


CefRefPtr<CefStreamReader> GetBinaryResourceReader(const TCHAR* szResourceName)
{
    char* pData = NULL;
    int nLen = 0;

	if (!Zephyros::GetResource(szResourceName, pData, nLen))
        return NULL;

	return CefStreamReader::CreateForHandler(new CefByteReadHandler((unsigned char*) pData, nLen, NULL));
}
