/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
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


#include "native_extensions/error.h"


namespace Zephyros {

void Error::FromError(DWORD windowsErrorCode)
{
    if (windowsErrorCode == ERROR_SUCCESS)
    {
        SetError(ERR_OK);
        return;
    }

    TCHAR* msg = NULL;
    String strMsg;

    int code = ERR_UNKNOWN;
    switch (windowsErrorCode)
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        code = ERR_FILE_NOT_FOUND;
        break;
    case ERROR_TOO_MANY_OPEN_FILES:
    case ERROR_NO_MORE_FILES:
    case ERROR_TOO_MANY_DESCRIPTORS:
        code = ERR_TOO_MANY_OPEN_FILES;
        break;
    case ERROR_ACCESS_DENIED:
    case ERROR_DELETE_PENDING:
    case ERROR_SHARING_VIOLATION:
        code = ERR_FILE_NO_READ_PERMISSION;
        break;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        code = ERR_INSUFFICIENT_MEMORY;
        break;
    case ERROR_WRITE_PROTECT:
    case ERROR_DRIVE_LOCKED:
        code = ERR_FILE_NO_WRITE_PERMISSION;
        break;
    case ERROR_HANDLE_DISK_FULL:
    case ERROR_DISK_RESOURCES_EXHAUSTED:
        code = ERR_DISK_FULL;
        break;
    case ERROR_DEV_NOT_EXIST:
        code = ERR_NO_NETWORK;
        break;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        code = ERR_FILE_EXISTS;
        break;
    case ERROR_INVALID_NAME:
    case ERROR_DIRECTORY:
    case ERROR_FILENAME_EXCED_RANGE:
        code = ERR_INVALID_FILENAME;
        break;
    case ERROR_FILE_TOO_LARGE:
        code = ERR_FILE_TOO_LARGE;
        break;
    case WAIT_TIMEOUT:
        code = ERR_TIMED_OUT;
        break;
    case ERROR_INSUFFICIENT_BUFFER:
        code = ERR_INSUFFICIENT_MEMORY;
        break;
    case ERROR_NO_UNICODE_TRANSLATION:
        code = ERR_DECODING_FAILED;
        break;
    }

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, windowsErrorCode, 0, (LPWSTR) &msg, 0, NULL);

    if (msg != NULL)
    {
        strMsg = String(msg);
        LocalFree(msg);
    }

    SetError(code, strMsg);
}

void Error::FromLastError()
{
    FromError(GetLastError());
}

} // namespace Zephyros
