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


#include <Cocoa/Cocoa.h>
#include <sys/errno.h>

#include "native_extensions/error.h"


namespace Zephyros {

void Error::FromError(id error)
{
    if (!error)
    {
        SetError(ERR_OK);
        return;
    }

    int code = ERR_UNKNOWN;

    if ([((NSError*) error).domain isEqualToString: NSCocoaErrorDomain])
    {
        switch (((NSError*) error).code)
        {
        case NSFileNoSuchFileError:
        case NSFileReadNoSuchFileError:
            code = ERR_FILE_NOT_FOUND;
            break;
        case NSFileReadNoPermissionError:
            code = ERR_FILE_NO_READ_PERMISSION;
            break;
        case NSFileWriteNoPermissionError:
        case NSFileWriteVolumeReadOnlyError:
            code = ERR_FILE_NO_WRITE_PERMISSION;
            break;
        case NSFileWriteInvalidFileNameError:
        case NSFileReadInvalidFileNameError:
            code = ERR_INVALID_FILENAME;
            break;
        case NSFileWriteFileExistsError:
            code = ERR_FILE_EXISTS;
            break;
        case NSFileReadInapplicableStringEncodingError:
            code = ERR_DECODING_FAILED;
            break;
        case NSFileReadTooLargeError:
            code = ERR_FILE_TOO_LARGE;
            break;
        case NSFileReadUnknownStringEncodingError:
            code = ERR_UNKNOWN_ENCODING;
            break;
        case NSFileWriteOutOfSpaceError:
            code = ERR_DISK_FULL;
            break;
        }
    }
    else if ([((NSError*) error).domain isEqualToString: NSPOSIXErrorDomain])
    {
        switch (((NSError*) error).code)
        {
        case ENOENT:
            code = ERR_FILE_NOT_FOUND;
            break;
        case EACCES:
            code = ERR_FILE_NO_READ_PERMISSION;
            break;
        case EEXIST:
            code = ERR_FILE_EXISTS;
            break;
        case ENOTDIR:
            code = ERR_NO_DIRECTORY;
            break;
        case EISDIR:
            code = ERR_IS_DIRECTORY;
            break;
        case EMFILE:
        case ENFILE:
            code = ERR_TOO_MANY_OPEN_FILES;
            break;
        case EFBIG:
            code = ERR_FILE_TOO_LARGE;
            break;
        case ENOSPC:
            code = ERR_DISK_FULL;
            break;
        case EROFS:
            code = ERR_FILE_NO_WRITE_PERMISSION;
            break;
        case EADDRINUSE:
            code = ERR_ADDRESS_IN_USE;
            break;
        case EADDRNOTAVAIL:
            code = ERR_ADDRESS_NOT_AVAILABLE;
            break;
        case ENETDOWN:
        case ENETUNREACH:
            code = ERR_NO_NETWORK;
            break;
        case ECONNRESET:
            code = ERR_CONNECTION_RESET;
            break;
        case ENOTCONN:
            code = ERR_NOT_CONNECTED;
            break;
        case ETIMEDOUT:
            code = ERR_TIMED_OUT;
            break;
        case ECONNREFUSED:
            code = ERR_CONNECTION_REFUSED;
            break;
        case ENAMETOOLONG:
            code = ERR_NAME_TOO_LONG;
            break;
        }
    }

    SetError(code, [((NSError*) error).localizedDescription UTF8String]);
}

} // namespace Zephyros
