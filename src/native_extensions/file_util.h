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


#ifndef Zephyros_FileUtil_h
#define Zephyros_FileUtil_h
#pragma once


#include <vector>

#include "base/types.h"

#include "native_extensions/error.h"
#include "native_extensions/path.h"


namespace Zephyros {
namespace FileUtil {
    
typedef struct
{
    bool isFile;
    bool isDirectory;
    uint64_t fileSize;
    uint64_t creationDate;
    uint64_t modificationDate;
} StatInfo;

#ifdef OS_MACOSX
void ShowOpenFileDialog(JavaScript::Object options, CallbackId callback);
void ShowSaveFileDialog(JavaScript::Object options, CallbackId callback);
void ShowOpenDirectoryDialog(JavaScript::Object options, CallbackId callback);
void ShowOpenFileOrDirectoryDialog(JavaScript::Object options, CallbackId callback);
#else
bool ShowOpenFileDialog(JavaScript::Object options, Path& path);
bool ShowSaveFileDialog(JavaScript::Object options, Path& path);
bool ShowOpenDirectoryDialog(JavaScript::Object options, Path& path);
bool ShowOpenFileOrDirectoryDialog(JavaScript::Object options, Path& path);
#endif

void ShowInFileManager(String path);

bool ExistsFile(String filename);
bool IsDirectory(String path);
bool Stat(String path, StatInfo* stat);

bool MakeDirectory(String path, bool recursive, Error& err);
bool ReadDirectory(String path, std::vector<String>& files, Error& err);

/**
 * Returns true if the path exists.
 * If path is a directory, it isn't modified; otherwise it is modified to the
 * directory containing the file path.
 */
bool GetDirectory(String& path);

bool ReadFileBinary(String filename, uint8_t** ppData, int& size, Error& err);
bool ReadFile(String filename, JavaScript::Object options, String& result, Error& err);
bool WriteFile(String filename, String contents, JavaScript::Object options, Error& err);
bool MoveFile(String oldFilename, String newFilename, Error& err);
bool CopyFile(String source, String destination, Error& err);
bool DeleteFiles(String filenames, Error& err);

void LoadPreferences(String key, String& data);
void StorePreferences(String key, String data);

bool StartAccessingPath(Path& path, Error& err);
void StopAccessingPath(Path& path);

void GetTempDir(Path& path);
String GetApplicationPath();
void GetApplicationResourcesPath(Path& path);

} // namespace FileUtil
} // namespace Zephyros


#endif // Zephyros_FileUtil_h
