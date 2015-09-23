/*******************************************************************************
 * Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
 * reserved. Use of this source code is governed by a BSD-style license that
 * can be found in the LICENSE file.
 *******************************************************************************/


#ifndef Zephyros_StringUtil_h
#define Zephyros_StringUtil_h
#pragma once


#include <string>
#include <vector>
#include "base/types.h"

#include "zephyros.h"


#ifdef USE_CEF
class CefRequest;

// Dump the contents of the request into a string.
void DumpRequestContents(CefRefPtr<CefRequest> request, String& str);
#endif

// Replace all instances of |from| with |to| in |str|.
String StringReplace(const String& str, const String& from, const String& to);

// Tests whether str ends with suffix.
bool StringEndsWith(const String& str, const String& suffix);

std::vector<String>& Split(const String &s, TCHAR delim, std::vector<String> &elems);

std::vector<String> Split(const String &s, TCHAR delim);

String& LTrim(String &s);

String& RTrim(String &s);

String& Trim(String &s);


#endif // Zephyros_StringUtil_h
