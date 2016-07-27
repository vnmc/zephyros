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


#import <Foundation/Foundation.h>
#import <mach-o/dyld.h>
#import <stdio.h>

#import "base/cef/resource_util.h"


bool GetResourceDir(std::string& dir)
{
    // retrieve the executable directory
    uint32_t pathSize = 0;
    _NSGetExecutablePath(NULL, &pathSize);
    if (pathSize > 0)
    {
        dir.resize(pathSize);
        _NSGetExecutablePath(const_cast<char*>(dir.c_str()), &pathSize);
    }

    // trim executable name up to the last separator
    std::string::size_type last_separator = dir.find_last_of("/");
    dir.resize(last_separator);
    dir.append("/../Resources");

    return true;
}

bool FileExists(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f)
    {
        fclose(f);
        return true;
    }

    return false;
}

bool ReadFileToString(const char* path, std::string& data)
{
    // Implementation adapted from base/file_util.cc
    FILE* file = fopen(path, "rb");
    if (!file)
        return false;

    char buf[1 << 16];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), file)) > 0)
        data.append(buf, len);
    fclose(file);

    return true;
}

bool LoadBinaryResource(const char* resource_name, std::string& resource_data)
{
    std::string path;
    if (!GetResourceDir(path))
        return false;

    path.append("/");
    path.append(resource_name);

    return ReadFileToString(path.c_str(), resource_data);
}

CefRefPtr<CefStreamReader> GetBinaryResourceReader(const char* resource_name)
{
    std::string path;
    if (!GetResourceDir(path))
        return NULL;

    path.append("/");
    path.append(resource_name);

    if (!FileExists(path.c_str()))
        return NULL;

    return CefStreamReader::CreateForFile(path);
}
