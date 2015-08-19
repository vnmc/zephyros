#include <windows.h>
#include <CommDlg.h>
#include <Psapi.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Shobjidl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <tchar.h>

#include "base/zephyros_impl.h"
#include "base/licensing.h"

#include "native_extensions/file_util.h"
#include "native_extensions/image_util_win.h"


namespace Zephyros {
namespace FileUtil {

bool ShowOpenFileDialog(Path& path)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];
	szFile[0] = 0;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.hwndOwner = GetActiveWindow();
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = NULL;
	ofn.lpstrFilter = TEXT("All Files\0*.*\0Web Files\0*.js;*.css;*.htm;*.html\0\0");
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileName(&ofn))
	{
		path = Path(szFile);
		return true;
	}

	return false;
}

bool ShowOpenDirectoryDialog(Path& path)
{
	bool pathSelected = false;

	// check current OS version
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (GetVersionEx(&osvi) && (osvi.dwMajorVersion >= 6))
	{
		// for Vista or later, use the MSDN-preferred implementation of the Open File dialog in pick folders mode
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
	}
	else
	{
		// for XP, use the old-styled SHBrowseForFolder() implementation
		BROWSEINFO bi = {0};
		bi.hwndOwner = GetActiveWindow();
		bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if (pidl != 0)
		{
			TCHAR szFile[MAX_PATH];
			szFile[0] = 0;

			if (SHGetPathFromIDList(pidl, szFile))
			{
				// Add directory path to the result
				//ConvertToUnixPath(pathName);
				path = Path(szFile);
				pathSelected = true;
			}

			IMalloc* pMalloc = NULL;
			SHGetMalloc(&pMalloc);
			if (pMalloc)
			{
				pMalloc->Free(pidl);
				pMalloc->Release();
			}
		}
	}

	return pathSelected;
}

#define FOLDERSELECTION TEXT("Folder Selection.")

bool ShowOpenFileOrDirectoryDialog(Path& path)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];
	_tcscpy(szFile, FOLDERSELECTION);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.hwndOwner = GetActiveWindow();
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = NULL;
	ofn.lpstrFilter = TEXT("All Files\0*.*\0Web Files\0*.js;*.css;*.htm;*.html\0\0");
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOTESTFILECREATE | OFN_EXPLORER;

    if (GetOpenFileName(&ofn))
	{
		if (_tcscmp(szFile + _tcslen(szFile) - _tcslen(FOLDERSELECTION), FOLDERSELECTION) == 0)
			PathRemoveFileSpec(szFile);

		path = Path(szFile);
		return true;
	}

	return false;
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

bool MakeDirectory(String path, bool recursive)
{
	// TODO: revise: according to MSDN:
	// "[SHCreateDirectory is available for use in the operating systems
	// specified in the Requirements section. It may be altered or unavailable
	// in subsequent versions.]"

	int r = SHCreateDirectory(GetActiveWindow(), PCWSTR(path.c_str()));
	if (r == ERROR_SUCCESS)
		return true;
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

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
	String encoding = TEXT("");
	if (options->HasKey("encoding"))
		encoding = options->GetString("encoding");

	// text file
	if (encoding == TEXT("") || encoding == TEXT("utf-8") || encoding == TEXT("text/plain;utf-8"))
	{
		HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD numBytesRead = 0;
			LARGE_INTEGER fileSize;
			GetFileSizeEx(hFile, &fileSize);

			// we don't want to read too large files
			_ASSERT(fileSize.HighPart == 0);

			// allocate buffer and read file
			BYTE* data = new BYTE[fileSize.LowPart];
			::ReadFile(hFile, (LPVOID) data, (DWORD) fileSize.LowPart, &numBytesRead, NULL);

			// convert the buffer
			int wcLen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) data, numBytesRead, NULL, 0);
			TCHAR* buf = new TCHAR[wcLen + 1];
			MultiByteToWideChar(CP_UTF8, 0, (LPCCH) data, numBytesRead, buf, wcLen);
			buf[wcLen] = 0;
			result = String(buf, buf + wcLen);
			delete[] buf;
			delete[] data;
		}

		CloseHandle(hFile);
		return true;
	}
	
	// image file; return as base64-encoded PNG
	if (encoding == TEXT("image/png;base64"))
	{
		BYTE* pData;
		DWORD length;
		if (!ImageUtil::ImageFileToPNG(filename, &pData, &length))
			return false;

		result = TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length);
		delete[] pData;
		return true;
	}

	return false;
}

void LoadPreferences(String key, String& data)
{
	data = TEXT("");

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, Zephyros::GetRegistryKey(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD type = 0;
		DWORD len = 0;
		if (RegQueryValueEx(hKey, key.c_str(), NULL, &type, NULL, &len) == ERROR_SUCCESS)
		{
			BYTE* buf = new BYTE[len];
			RegQueryValueEx(hKey, key.c_str(), NULL, &type, buf, &len);

			// convert to string; RegQueryValueEx includes the terminating \0, so the last char is one
			// char before that, hence "- sizeof(TCHAR)"
			data = String(reinterpret_cast<TCHAR*>(buf), reinterpret_cast<TCHAR*>(buf + len - sizeof(TCHAR)));

			delete[] buf;
		}

		RegCloseKey(hKey);
	}
}

void StorePreferences(String key, String data)
{
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, Zephyros::GetRegistryKey(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, key.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (DWORD) ((data.length() + 1) * sizeof(TCHAR)));
		RegCloseKey(hKey);
	}
}

bool StartAccessingPath(Path& path)
{
	// not supported on Windows
	return true;
}

void StopAccessingPath(Path& path)
{
	// not supported on Windows
}

void GetApplicationResourcesPath(Path& path)
{
	TCHAR* pModuleFilename = new TCHAR[MAX_PATH + 1];
	GetModuleFileName(NULL, pModuleFilename, MAX_PATH);
	TCHAR* pLastSep = _tcsrchr(pModuleFilename, TEXT('\\'));
	String strModulePath(pModuleFilename, pLastSep);
	delete[] pModuleFilename;

	path = Path(strModulePath + TEXT("\\res"));
}

} // namespace FileUtil
} // namespace Zephyros
