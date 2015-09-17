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


#include "native_extensions/file_util.h"


namespace Zephyros {
namespace FileUtil {

bool ShowOpenFileDialog(Path& path)
{
	// TODO: implement
	return false;
}

bool ShowSaveFileDialog(Path& path)
{
	// TODO: implement
	return false;
}

bool ShowOpenDirectoryDialog(Path& path)
{
	// TODO: implement
	return false;
}

bool ShowOpenFileOrDirectoryDialog(Path& path)
{
	// TODO: implement
	return false;
}

void ShowInFileManager(String path)
{
	// TODO: implement
}

bool ExistsFile(String filename)
{
	// TODO: implement
	return false;
}

bool IsDirectory(String path)
{
    // TODO: implement
    return false;
}

bool MakeDirectory(String path, bool recursive)
{
	// TODO: implement
	return false;
}

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
    // TODO: implement
    return false;
}

bool WriteFile(String filename, String contents)
{
    // TODO: implement
    return false;
}

bool DeleteFiles(String filenames)
{
    // TODO: implement
    return false;
}

void LoadPreferences(String key, String& data)
{
    // TODO: implement
}

void StorePreferences(String key, String data)
{
    // TODO: implement
}

bool StartAccessingPath(Path& path)
{
	// not supported on Linux
	return true;
}

void StopAccessingPath(Path& path)
{
	// not supported on Linux
}

void GetTempDir(Path& path)
{
    // TODO: implement
}

void GetApplicationResourcesPath(Path& path)
{
    // TODO: implement
}

} // namespace FileUtil
} // namespace Zephyros
