// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
#define CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
#pragma once

#include <string>
#include "lib/cef/include/cef_stream.h"
#include "zephyros.h"


#if defined(OS_POSIX)
// Returns the directory containing resource files.
bool GetResourceDir(String& dir);
#endif

// Retrieve a resource as a string.
bool LoadBinaryResource(const TCHAR* resource_name, String& resource_data);

// Retrieve a resource as a steam reader.
CefRefPtr<CefStreamReader> GetBinaryResourceReader(const TCHAR* resource_name);

#endif  // CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
