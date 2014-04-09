// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_STRING_UTIL_H_
#define CEF_TESTS_CEFCLIENT_STRING_UTIL_H_
#pragma once

#include <string>
#include "types.h"


class CefRequest;


// Dump the contents of the request into a string.
void DumpRequestContents(CefRefPtr<CefRequest> request, String& str);

// Replace all instances of |from| with |to| in |str|.
String StringReplace(const String& str, const String& from, const String& to);

// Tests whether str ends with suffix.
bool StringEndsWith(const String& str, const String& suffix);


#endif  // CEF_TESTS_CEFCLIENT_STRING_UTIL_H_
