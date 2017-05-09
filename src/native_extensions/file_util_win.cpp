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
 * Florian Müller, Vanamco AG
 *******************************************************************************/


#include <Windows.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Shobjidl.h>
#include <stdio.h>
#include <tchar.h>
#include <sys/stat.h>

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/base64.h"
#include "util/string_util.h"

#include "native_extensions/file_util.h"
#include "native_extensions/error.h"
#include "native_extensions/image_util_win.h"


namespace Zephyros {
namespace FileUtil {

#define FOLDERSELECTION TEXT("Folder Selection.")
TCHAR* g_szDefaultFilter = TEXT("All Files\0*.*\0\0");


void CreateOpenFileNameStruct(JavaScript::Object options, OPENFILENAME* pOfn, TCHAR** pszFolderSelection)
{
    ZeroMemory(pOfn, sizeof(OPENFILENAME));

    pOfn->hwndOwner = GetActiveWindow();
    pOfn->lStructSize = sizeof(OPENFILENAME);
    pOfn->nMaxFile = MAX_PATH;

    // fill in the initial file name
    pOfn->lpstrFile = new TCHAR[MAX_PATH + 1];
    if (pszFolderSelection)
    {
        // combined (file/folder) dialog
        *pszFolderSelection = new TCHAR[MAX_PATH + 1];
        _tcscpy(*pszFolderSelection, options->HasKey(TEXT("folderSelectionText")) ? String(options->GetString("folderSelectionText")).c_str() : FOLDERSELECTION);

        // make sure the string ends with a dot
        if ((*pszFolderSelection)[_tcslen(*pszFolderSelection) - 1] != TEXT('.'))
            _tcscat(*pszFolderSelection, TEXT("."));

        _tcscpy(pOfn->lpstrFile, *pszFolderSelection);
    }
    else
    {
        if (options->HasKey(TEXT("initialFile")))
            _tcscpy(pOfn->lpstrFile, String(options->GetString(TEXT("initialFile"))).c_str());
        else
            pOfn->lpstrFile[0] = 0;
    }
    
    // fill in the title (or set to NULL if not provided)
    if (options->HasKey(TEXT("title")))
    {
        String strTitle(options->GetString(TEXT("title")));
        TCHAR* szTitle = new TCHAR[strTitle.length() + 1];
        _tcscpy(szTitle, strTitle.c_str());
        pOfn->lpstrTitle = szTitle;
    }
    else
        pOfn->lpstrTitle = NULL;
    
    // fill in the filters
    if (options->HasKey(TEXT("filters")))
    {
        JavaScript::Array filters = options->GetList(TEXT("filters"));
        int nNumItems = (int) filters->GetSize();
        StringStream ss;

        for (int i = 0; i < nNumItems; ++i)
        {
            JavaScript::Object filter = filters->GetDictionary(i);

            if (filter->HasKey(TEXT("description")))
                ss << String(filter->GetString(TEXT("description")));
            else
                ss << TEXT("");
            ss << TEXT('\0');

            if (filter->HasKey(TEXT("extensions")))
                ss << String(filter->GetString(TEXT("extensions")));
            else
                ss << TEXT("*.*");
            ss << TEXT('\0');
        }

        ss << TEXT('\0');

        String strFilter = ss.str();
        int nLen = strFilter.length() + 1;
        TCHAR* szFilter = new TCHAR[nLen];
        memcpy(szFilter, strFilter.c_str(), nLen * sizeof(TCHAR));
        pOfn->lpstrFilter = szFilter;
    }
    else
        pOfn->lpstrFilter = g_szDefaultFilter;
    
    // fill in the initial directory (or set to NULL if not provided)
    if (options->HasKey(TEXT("initialDirectory")))
    {
        String strInitialDir(options->GetString(TEXT("initialDirectory")));
        TCHAR* szInitialDir = new TCHAR[strInitialDir.length() + 1];
        _tcscpy(szInitialDir, strInitialDir.c_str());
        pOfn->lpstrInitialDir = szInitialDir;
    }
    else
        pOfn->lpstrInitialDir = NULL;
}

void DestroyOpenFileNameStruct(OPENFILENAME* pOfn)
{
    delete[] pOfn->lpstrFile;
    if (pOfn->lpstrTitle)
        delete[] pOfn->lpstrTitle;
    if (pOfn->lpstrFilter && pOfn->lpstrFilter != g_szDefaultFilter)
        delete[] pOfn->lpstrFilter;
    if (pOfn->lpstrInitialDir)
        delete[] pOfn->lpstrInitialDir;
}

bool ShowOpenFileDialog(JavaScript::Object options, Path& path)
{
    OPENFILENAME ofn;
    CreateOpenFileNameStruct(options, &ofn, NULL);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    bool ret = false;
    if (GetOpenFileName(&ofn))
    {
        path = Path(ofn.lpstrFile);
        ret = true;
    }

    DestroyOpenFileNameStruct(&ofn);
    return ret;
}

bool ShowSaveFileDialog(JavaScript::Object options, Path& path)
{
    OPENFILENAME ofn;
    CreateOpenFileNameStruct(options, &ofn, NULL);
    ofn.Flags = OFN_EXPLORER;

    bool ret = false;
    if (GetSaveFileName(&ofn))
    {
        path = Path(ofn.lpstrFile);
        ret = true;
    }

    DestroyOpenFileNameStruct(&ofn);
    return ret;
}

bool ShowOpenDirectoryDialog(JavaScript::Object options, Path& path)
{
    bool pathSelected = false;

    // check current OS version
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    IFileDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        // configure the dialog to Select Folders only
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_DONTADDTORECENT);

            if (SUCCEEDED(pfd->Show(GetActiveWindow())))
            {
                IShellItem *psi;
                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    LPWSTR lpwszName = NULL;
                    if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, (LPWSTR*) &lpwszName)))
                    {
                        // Add directory path to the result
                        //ConvertToUnixPath(pathName);
                        path = Path(lpwszName);
                        pathSelected = true;

                        ::CoTaskMemFree(lpwszName);
                    }

                    psi->Release();
                }
            }
        }

        pfd->Release();
    }

    return pathSelected;
}

bool ShowOpenFileOrDirectoryDialog(JavaScript::Object options, Path& path)
{
    OPENFILENAME ofn;
    TCHAR* szFolderSelection;
    CreateOpenFileNameStruct(options, &ofn, &szFolderSelection);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOTESTFILECREATE | OFN_EXPLORER;

    bool ret = false;
    if (GetOpenFileName(&ofn))
    {
        TCHAR* szFile = ofn.lpstrFile;
        if (_tcscmp(szFile + _tcslen(szFile) - _tcslen(szFolderSelection), szFolderSelection) == 0)
            PathRemoveFileSpec(szFile);

        path = Path(szFile);
        ret = true;
    }

    DestroyOpenFileNameStruct(&ofn);
    delete[] szFolderSelection;
    return ret;
}

void ShowInFileManager(String path)
{
    ShellExecute(NULL, TEXT("open"), path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

bool ExistsFile(String filename)
{
    return PathFileExists(filename.c_str()) == TRUE;
}

bool IsDirectory(String path)
{
    if (path.substr(0, 7) == TEXT("http://") || path.substr(0, 8) == TEXT("https://"))
        return false;

    DWORD dwFileAttrs = GetFileAttributes(path.c_str());
    if (dwFileAttrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return (dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool Stat(String path, StatInfo* stat)
{
    if (path.substr(0, 7) == TEXT("http://") || path.substr(0, 8) == TEXT("https://"))
        return false;

    DWORD dwFileAttrs = GetFileAttributes(path.c_str());
    if (dwFileAttrs != INVALID_FILE_ATTRIBUTES)
    {
        stat->isFile = (dwFileAttrs & ~FILE_ATTRIBUTE_DIRECTORY & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_READONLY)) != 0;
        stat->isDirectory = (dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    else
    {
        stat->isFile = false;
        stat->isDirectory = false;
    }

    HANDLE hnd = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hnd == INVALID_HANDLE_VALUE)
    {
        stat->fileSize = 0;
        stat->creationDate = 0;
        stat->modificationDate = 0;

        return false;
    }

    LARGE_INTEGER size;
    GetFileSizeEx(hnd, &size);
    stat->fileSize = size.QuadPart;

    FILETIME createTime, writeTime;
    GetFileTime(hnd, &createTime, NULL, &writeTime);

    ULARGE_INTEGER ullCreateTime, ullWriteTime;
    ullCreateTime.LowPart = createTime.dwLowDateTime;
    ullCreateTime.HighPart = createTime.dwHighDateTime;
    ullWriteTime.LowPart = writeTime.dwLowDateTime;
    ullWriteTime.HighPart = writeTime.dwHighDateTime;

    stat->creationDate = static_cast<uint64_t>(ullCreateTime.QuadPart / 10000ULL - 11644473600000ULL);
    stat->modificationDate = static_cast<uint64_t>(ullWriteTime.QuadPart / 10000ULL - 11644473600000ULL);

    CloseHandle(hnd);

    return true;
}

bool MakeDirectory(String path, bool recursive, Error& err)
{
    // TODO: revise: according to MSDN:
    // "[SHCreateDirectory is available for use in the operating systems
    // specified in the Requirements section. It may be altered or unavailable
    // in subsequent versions.]"

    int ret = SHCreateDirectory(GetActiveWindow(), PCWSTR(path.c_str()));
    if (ret == ERROR_SUCCESS)
        return true;

    err.FromError(ret);
    return false;
}


bool GetDirectory(String& path)
{
    DWORD dwFileAttrs = GetFileAttributes(path.c_str());
    if (dwFileAttrs == INVALID_FILE_ATTRIBUTES)
        return false;

    // if path is not a directory, take its parent directory
    if ((dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        TCHAR* pszPath = new TCHAR[path.length() + 1];
        _tcscpy(pszPath, path.c_str());
        PathRemoveFileSpec(pszPath);
        path = String(pszPath);
        delete pszPath;
    }

    return true;
}

bool ReadDirectory(String path, std::vector<String>& files, Error& err)
{
    if (path.find_first_of(TEXT("*?")) == String::npos)
    {
        // no wildcards, add a "*" at the end
        if (path.back() != TEXT('/') && path.back() != TEXT('\\'))
            path.append(PATH_SEPARATOR_STRING);
        path.append(TEXT("*"));
    }

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(path.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        err.FromLastError();
        return false;
    }

    TCHAR* pBasePath = new TCHAR[path.length() + 1];
    _tcscpy(pBasePath, path.c_str());
    PathRemoveFileSpec(pBasePath);
    size_t idx = _tcslen(pBasePath);
    pBasePath[idx] = PATH_SEPARATOR;
    pBasePath[idx + 1] = TEXT('\0');

    do
    {
        if (_tcscmp(fd.cFileName, TEXT(".")) == 0 || _tcscmp(fd.cFileName, TEXT("..")) == 0)
            continue;

        String filename(pBasePath);
        filename.append(fd.cFileName);
        files.push_back(filename);
    } while (FindNextFile(hFind, &fd));

    FindClose(hFind);
    delete[] pBasePath;
    return true;
}

bool ReadFileBinary(String filename, uint8_t** ppData, int& size, Error& err)
{
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        err.FromLastError();
        return false;
    }

    DWORD numBytesRead = 0;
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        err.FromLastError();
        return false;
    }

    // we don't want to read too large files
    _ASSERT(fileSize.HighPart == 0);

    // allocate buffer and read file
    *ppData = new BYTE[fileSize.LowPart];
    if (!::ReadFile(hFile, (LPVOID) *ppData, (DWORD) fileSize.LowPart, &numBytesRead, NULL))
    {
        delete[] *ppData;
        CloseHandle(hFile);
        err.FromLastError();
        return false;
    }

    size = numBytesRead;
    CloseHandle(hFile);

    return true;
}

bool ReadFile(String filename, JavaScript::Object options, String& result, Error& err)
{
    String encoding = TEXT("");
    if (options->HasKey("encoding"))
        encoding = options->GetString("encoding");

    // text file
    if (encoding == TEXT("") || encoding == TEXT("utf-8") || encoding == TEXT("text/plain;utf-8"))
    {
        uint8_t* data = NULL;
        int numBytesRead = 0;

        if (ReadFileBinary(filename, &data, numBytesRead, err))
        {
            // convert the buffer
            int wcLen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) data, numBytesRead, NULL, 0);
            TCHAR* buf = new TCHAR[wcLen + 1];

            if (MultiByteToWideChar(CP_UTF8, 0, (LPCCH) data, numBytesRead, buf, wcLen) == 0)
            {
                delete[] data;
                delete[] buf;
                err.FromLastError();
                return false;
            }

            buf[wcLen] = 0;
            result = String(buf, buf + wcLen);

            delete[] buf;
            delete[] data;

            return true;
        }

        // reading the file failed
        return false;
    }

    // image file; return as base64-encoded PNG
    if (encoding == TEXT("image/png;base64"))
    {
        BYTE* pData;
        DWORD length;
        if (!ImageUtil::ImageFileToPNG(filename, &pData, &length, err))
            return false;

        result = TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length);
        delete[] pData;
        return true;
    }

    // unknown encoding
    err.SetError(ERR_UNKNOWN_ENCODING, TEXT("The encoding \"") + encoding + TEXT("\" is not supported."));

    return false;
}

bool WriteFile(String filename, String contents, JavaScript::Object options, Error& err)
{
	String encoding(TEXT(""));
	if (options->HasKey("encoding"))
		encoding = options->GetString("encoding");

    bool ret = false;

    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
		if (encoding == TEXT("base64"))
		{
			size_t len = 0;
			BYTE* pData = (BYTE*) NewBase64Decode(std::string(contents.begin(), contents.end()).c_str(), contents.length(), &len);
			DWORD dwBytesWritten = 0;
			ret = ::WriteFile(hFile, pData, len, &dwBytesWritten, NULL) == TRUE;
		}
		else
		{
			int mbLen = WideCharToMultiByte(CP_UTF8, 0, contents.c_str(), (int) contents.length(), NULL, 0, NULL, NULL);
			BYTE* buf = new BYTE[mbLen + 1];
			if (WideCharToMultiByte(CP_UTF8, 0, contents.c_str(), (int) contents.length(), (LPSTR) buf, mbLen, NULL, NULL))
			{
				buf[mbLen] = 0;

				DWORD dwBytesWritten = 0;
				ret = ::WriteFile(hFile, buf, mbLen, &dwBytesWritten, NULL) == TRUE;
				FlushFileBuffers(hFile);
			}

			delete[] buf;
		}

		CloseHandle(hFile);
    }

    if (!ret)
        err.FromLastError();

    return ret;
}

bool MoveFile(String oldFilename, String newFilename, Error& err)
{
    if (!::MoveFile(oldFilename.c_str(), newFilename.c_str()))
    {
        err.FromLastError();
        return false;
    }

    return true;
}

bool CopyFile(String source, String destination, Error& err)
{
	if (!::CopyFile(source.c_str(), destination.c_str(), FALSE))
	{
		err.FromLastError();
		return false;
	}

	return true;
}

bool DeleteFiles(String filenames, Error& err)
{
    // no wildcards in the filename; simply delete the file
    if (filenames.find_first_of(TEXT("*?")) == String::npos)
    {
        if (!DeleteFile(filenames.c_str()))
        {
            err.FromLastError();
            return false;
        }

        return true;
    }
        

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(filenames.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        err.FromLastError();
        return false;
    }

    TCHAR* pDir = new TCHAR[filenames.length() + 1];
    _tcscpy(pDir, filenames.c_str());
    PathRemoveFileSpec(pDir);
    size_t idx = _tcslen(pDir);
    pDir[idx] = PATH_SEPARATOR;
    pDir[idx + 1] = TEXT('\0');
    
    do
    {
        if (_tcscmp(fd.cFileName, TEXT(".")) == 0 || _tcscmp(fd.cFileName, TEXT("..")) == 0)
            continue;

        String filename(pDir);
        filename.append(fd.cFileName);
        if (!DeleteFile(filename.c_str()))
        {
            FindClose(hFind);
            delete[] pDir;
            err.FromLastError();
            return false;
        }
    } while (FindNextFile(hFind, &fd));

    FindClose(hFind);
    delete[] pDir;
    return true;
}

void LoadPreferences(String key, String& data)
{
    data = TEXT("");

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, Zephyros::GetWindowsInfo().szRegistryKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD type = 0;
        DWORD len = 0;
        if (RegQueryValueEx(hKey, key.c_str(), NULL, &type, NULL, &len) == ERROR_SUCCESS)
        {
            BYTE* buf = new BYTE[len];
            RegQueryValueEx(hKey, key.c_str(), NULL, &type, buf, &len);
            data = ToString(buf, len);

            delete[] buf;
        }

        RegCloseKey(hKey);
    }
}

void StorePreferences(String key, String data)
{
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, Zephyros::GetWindowsInfo().szRegistryKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, key.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (DWORD) ((data.length() + 1) * sizeof(TCHAR)));
        RegCloseKey(hKey);
    }
}

bool StartAccessingPath(Path& path, Error& err)
{
    // not supported on Windows
    return true;
}

void StopAccessingPath(Path& path)
{
    // not supported on Windows
}

void GetTempDir(Path& path)
{
    TCHAR* pTempPath = new TCHAR[MAX_PATH + 1];
    GetTempPath(MAX_PATH, pTempPath);

    path = Path(pTempPath);
    delete[] pTempPath;
}

String GetApplicationPath()
{
    TCHAR* pModuleFilename = new TCHAR[MAX_PATH + 1];
    GetModuleFileName(NULL, pModuleFilename, MAX_PATH);
    TCHAR* pLastSep = _tcsrchr(pModuleFilename, TEXT('\\'));
    String path(pModuleFilename, pLastSep);
    delete[] pModuleFilename;

    return path;
}

void GetApplicationResourcesPath(Path& path)
{
    path = Path(FileUtil::GetApplicationPath() + TEXT("\\res"));
}

} // namespace FileUtil
} // namespace Zephyros
