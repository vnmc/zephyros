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

#include "file_util.h"
#include "image_util_win.h"


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

void ShowInFileManager(String path)
{
	ShellExecute(NULL, TEXT("open"), path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
	String encoding = TEXT("");
	if (options->HasKey("encoding"))
		options->GetString("encoding");

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

		result = ImageUtil::Base64Encode(pData, length);
		delete[] pData;
		return true;
	}

	return false;
}

} // namespace FileUtil