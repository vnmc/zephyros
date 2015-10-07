/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_FileUtil_h
#define Zephyros_FileUtil_h
#pragma once


#include "base/types.h"
#include "native_extensions/path.h"


namespace Zephyros {
namespace FileUtil {

#ifdef OS_MACOSX
void ShowOpenFileDialog(CallbackId callback);
void ShowSaveFileDialog(CallbackId callback);
void ShowOpenDirectoryDialog(CallbackId callback);
void ShowOpenFileOrDirectoryDialog(CallbackId callback);
#else
bool ShowOpenFileDialog(Path& path);
bool ShowSaveFileDialog(Path& path);
bool ShowOpenDirectoryDialog(Path& path);
bool ShowOpenFileOrDirectoryDialog(Path& path);
#endif

void ShowInFileManager(String path);

bool ExistsFile(String filename);
bool IsDirectory(String path);

bool MakeDirectory(String path, bool recursive);

/**
 * Returns true if the path exists.
 * If path is a directory, it isn't modified; otherwise it is modified to the
 * directory containing the file path.
 */
bool GetDirectory(String& path);

bool ReadFile(String filename, JavaScript::Object options, String& result);
bool WriteFile(String filename, String contents);
bool DeleteFiles(String filenames);
    
void LoadPreferences(String key, String& data);
void StorePreferences(String key, String data);

bool StartAccessingPath(Path& path);
void StopAccessingPath(Path& path);
    
void GetTempDir(Path& path);
void GetApplicationResourcesPath(Path& path);

} // namespace FileUtil
} // namespace Zephyros


#endif // Zephyros_FileUtil_h
