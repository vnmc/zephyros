/*******************************************************************************
 * Copyright (c) 2015-2017 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_Error_h
#define Zephyros_Error_h
#pragma once


#ifdef USE_CEF
#include "lib/cef/include/base/cef_lock.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/wrapper/cef_helpers.h"
#endif

#include "native_extensions.h"


#define ERR_OK 0
#define ERR_UNKNOWN -1
#define ERR_FILE_NOT_FOUND 2
#define ERR_FILE_NO_READ_PERMISSION 13
#define ERR_FILE_NO_WRITE_PERMISSION 14
#define ERR_FILE_EXISTS 17
#define ERR_NO_DIRECTORY 20
#define ERR_IS_DIRECTORY 21
#define ERR_INVALID_FILENAME 22
#define ERR_TOO_MANY_OPEN_FILES 24
#define ERR_FILE_TOO_LARGE 27
#define ERR_DISK_FULL 28
#define ERR_ADDRESS_IN_USE 48
#define ERR_ADDRESS_NOT_AVAILABLE 49
#define ERR_NO_NETWORK 50
#define ERR_CONNECTION_RESET 54
#define ERR_NOT_CONNECTED 57
#define ERR_TIMED_OUT 60
#define ERR_CONNECTION_REFUSED 61
#define ERR_NAME_TOO_LONG 63
#define ERR_INSUFFICIENT_MEMORY 101
#define ERR_UNKNOWN_ENCODING 102
#define ERR_DECODING_FAILED 103

// the class "Error" is declared in native_extensions.h


#endif // Zephyros_Path_h
