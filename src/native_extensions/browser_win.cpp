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


#include <iomanip>
#include <tchar.h>

#include <Windows.h>

#include "base/app.h"
#include "base/logging.h"

#include "util/string_util.h"

#include "native_extensions/browser.h"
#include "native_extensions/image_util_win.h"


#define MAX_LEN 2048


namespace Zephyros {
namespace BrowserUtil {

/**
 * Returns the name of the default browser
 */
String GetDefaultBrowserKey()
{
	bool bDefaultBrowserKeyFound = false;
	String strDefaultBrowserKey;

	DWORD len;
	DWORD type;
	BYTE data[MAX_LEN + 1];

	// Windows 10
	// cf. http://stackoverflow.com/questions/32354861/how-to-get-default-browser-on-window-10

	HKEY hKeyHttpUserChoice, hKeyDefaultHttpApp;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\URLAssociations\\http\\UserChoice"), 0, KEY_READ, &hKeyHttpUserChoice) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKeyHttpUserChoice, TEXT("ProgId"), NULL, &type, data, &len) == ERROR_SUCCESS)
		{
			String appNameKey = ToString(data, len) + TEXT("\\Application");
			
			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, appNameKey.c_str(), 0, KEY_READ, &hKeyDefaultHttpApp) == ERROR_SUCCESS)
			{
				if (RegQueryValueEx(hKeyDefaultHttpApp, TEXT("ApplicationName"), NULL, &type, data, &len) == ERROR_SUCCESS)
				{
					if (type == REG_SZ && len > 0)
					{
						strDefaultBrowserKey = ToString(data, len);
						bDefaultBrowserKeyFound = true;
					}
				}

				RegCloseKey(hKeyDefaultHttpApp);
			}
		}

		RegCloseKey(hKeyHttpUserChoice);
	}

	if (bDefaultBrowserKeyFound)
		return strDefaultBrowserKey;


	// older Windows versions

	// first look under HKEY_CURRENT_USER
	HKEY hKey1, hKey2;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Clients\\StartMenuInternet"), 0, KEY_READ, &hKey1) == ERROR_SUCCESS)
	{
		DEBUG_LOG(TEXT("Opened HKCU\\SOFTWARE\\Clients\\StartMenuInternet"));

		len = MAX_LEN;
		if (RegQueryValueEx(hKey1, TEXT(""), NULL, &type, data, &len) == ERROR_SUCCESS)
		{
			DEBUG_LOG(TEXT("Queried value (1), len=") + TO_STRING(len));

			if (type == REG_SZ && len > 0)
			{
				strDefaultBrowserKey = ToString(data, len);
				bDefaultBrowserKeyFound = true;

				DEBUG_LOG(TEXT("defaultBrowserKey (1) =") + strDefaultBrowserKey);
			}
		}

		if (!bDefaultBrowserKeyFound)
		{
			DEBUG_LOG(TEXT("default browser not found in HKCU"));

			// if no StartMenuInternet entry found in current user, look in HKEY_LOCAL_MACHINE
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Clients\\StartMenuInternet"), 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
			{
				DEBUG_LOG(TEXT("Opened HKLM\\SOFTWARE\\Clients\\StartMenuInternet"));

				len = MAX_LEN;
				if (RegQueryValueEx(hKey2, TEXT(""), NULL, &type, data, &len) == ERROR_SUCCESS)
				{
					DEBUG_LOG(TEXT("Queried value (2), len=") + TO_STRING(len));

					if (type == REG_SZ && len > 0)
					{
						strDefaultBrowserKey = ToString(data, len);
						bDefaultBrowserKeyFound = true;

						DEBUG_LOG(TEXT("defaultBrowserKey (2) =") + strDefaultBrowserKey);
					}
				}

				RegCloseKey(hKey2);
				DEBUG_LOG(TEXT("Closed key (2)"));
			}
		}

		RegCloseKey(hKey1);
		DEBUG_LOG(TEXT("Closed key (1)"));
	}

	return strDefaultBrowserKey;
}

HICON LoadLibIcon(TCHAR* szLibIcon)
{
	// find the icon ID in szLibIcon (after the comma)
	// start at the second-to-last character (there must be something after the comma...)
	int pos = (int) (_tcslen(szLibIcon)) - 2;
	if (pos <= 0)
		return NULL;

	int id = INT_MIN;
	for ( ; pos >= 0; --pos)
	{
		if (szLibIcon[pos] == ',')
		{
			id = _tstoi(szLibIcon + pos + 1);
			szLibIcon[pos] = 0;
			break;
		}
	}

	if (id == INT_MIN)
		return NULL;

	if (szLibIcon[0] == TEXT('"'))
		szLibIcon++;
	if (pos >= 2 && szLibIcon[pos - 2] == TEXT('"'))
		szLibIcon[pos - 2] = 0;

	DEBUG_LOG(String(TEXT("lib icon=")) + szLibIcon);

	HICON hIcon = NULL;
	ExtractIconEx(szLibIcon, id, &hIcon, NULL, 1);

	return hIcon;
}

String ExtractFilename(String strCommand)
{
	if (strCommand[0] != TEXT('"'))
		return strCommand;

	size_t pos = strCommand.find(TEXT('"'), 1);
	if (pos == String::npos)
		return strCommand;

	return strCommand.substr(1, pos - 1);
}

String GetExeVersion(String strCommand)
{
	String strResult(TEXT(""));
	String strFilename = ExtractFilename(strCommand);
	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(strFilename.c_str(), &dwHandle);

    if (dwSize > 0)
    {
        BYTE* pbBuf = new BYTE[dwSize];
		if (GetFileVersionInfo(strFilename.c_str(), dwHandle, dwSize, pbBuf))
        {
            UINT uiSize = 0;
            BYTE* lpb = NULL;

            if (VerQueryValue(pbBuf, TEXT("\\VarFileInfo\\Translation"), (void**) &lpb, &uiSize))
            {
                WORD* lpw = (WORD*) lpb;
				StringStream ssQuery;
				ssQuery << TEXT("\\StringFileInfo\\");
				ssQuery << std::setfill(TEXT('0')) << std::setw(4) << std::hex << lpw[0];
				ssQuery << std::setfill(TEXT('0')) << std::setw(4) << std::hex << lpw[1];
				ssQuery << TEXT("\\ProductVersion");

                if (VerQueryValue(pbBuf, ssQuery.str().c_str(), (void**) &lpb, &uiSize) && uiSize > 0)
                    strResult = (LPCTSTR) lpb;
            }
        }

		delete[] pbBuf;
    }

    return strResult;
}

/**
 * Returns an array of all browsers available on the system.
 */
void FindBrowsers(std::vector<Browser*>** ppBrowsers)
{
	if (*ppBrowsers != NULL)
		return;

	*ppBrowsers = new std::vector<Browser*>();

	String strDefaultBrowserKey = GetDefaultBrowserKey();

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Clients\\StartMenuInternet"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DEBUG_LOG(TEXT("Opened HKLM\\SOFTWARE\\Clients\\StartMenuInternet"));

		// enumerate the subkeys
		TCHAR szSubkeyName[MAX_LEN + 1];
		DWORD len;
		DWORD type;
		BYTE data[MAX_LEN + 1];

		for (DWORD idx = 0; ; ++idx)
		{
			DEBUG_LOG(TEXT("Trying to read key #") + TO_STRING(idx));

			len = MAX_LEN;
			LONG res = RegEnumKeyEx(hKey, idx, szSubkeyName, &len, NULL, NULL, NULL, NULL);
			
			if (res == ERROR_NO_MORE_ITEMS)
			{
				DEBUG_LOG(TEXT("No more items"));
				break;
			}
			if (res != ERROR_SUCCESS)
			{
				DEBUG_LOG(TEXT("Failed to enum key"));
				continue;
			}

			bool success = true;
			String strBrowserName;
			String strBrowserCommand;
			String strIcon;
			int nStatusCode = 0;
			//String strGrayscaleIcon;

			HKEY hKeyBrowser;
			if (RegOpenKeyEx(hKey, szSubkeyName, 0, KEY_READ, &hKeyBrowser) == ERROR_SUCCESS)
			{
				DEBUG_LOG(String(TEXT("Opened ")) + szSubkeyName);

				// get the default value of the subkey: this is the name of the browser
				len = MAX_LEN;
				if (RegQueryValueEx(hKeyBrowser, TEXT(""), NULL, &type, data, &len) == ERROR_SUCCESS)
				{
					DEBUG_LOG(TEXT("Queried value (3), len=") + TO_STRING(len));

					if (type == REG_SZ && len > 0)
					{
						strBrowserName = ToString(data, len);
						DEBUG_LOG(TEXT("Found browser ") + strBrowserName);
					}
					else
						success = false;
				}
				else
					success = false;

				// get the path to the browser program: this is contained in the default value of "shell\open\command"
				HKEY hKeyCommand;
				if (success && RegOpenKeyEx(hKeyBrowser, TEXT("shell\\open\\command"), 0, KEY_READ, &hKeyCommand) == ERROR_SUCCESS)
				{
					DEBUG_LOG(TEXT("Opened shell\\open\\command"));

					len = MAX_LEN;
					if (RegQueryValueEx(hKeyCommand, TEXT(""), NULL, &type, data, &len) == ERROR_SUCCESS)
					{
						DEBUG_LOG(TEXT("Queried value (4), len=") + TO_STRING(len));

						if (type == REG_SZ && len > 0)
						{
							strBrowserCommand = ToString(data, len);
							DEBUG_LOG(TEXT("Found browser command ") + strBrowserCommand);
						}
						else
							success = false;
					}
					else
						success = false;

					RegCloseKey(hKeyCommand);
					DEBUG_LOG(TEXT("Closed key (3)"));
				}
				else
					nStatusCode = BROWSER_CODE_NO_EXE_FOUND;

				// get the default icon: the path is contained in the default value of the "DefaultIcon" subkey
				HKEY hKeyIcon;
				if (success && RegOpenKeyEx(hKeyBrowser, TEXT("DefaultIcon"), 0, KEY_READ, &hKeyIcon) == ERROR_SUCCESS)
				{
					DEBUG_LOG(TEXT("Opened DefaultIcon"));

					len = MAX_LEN;
					if (RegQueryValueEx(hKeyIcon, TEXT(""), NULL, &type, data, &len) == ERROR_SUCCESS)
					{
						DEBUG_LOG(TEXT("Queried value (5), len=") + TO_STRING(len));

						if (type == REG_SZ && len > 0)
						{
							HICON hIcon = LoadLibIcon(reinterpret_cast<TCHAR*>(data));
							if (hIcon != NULL)
							{
								BYTE* pData = NULL;
								DWORD length;
								if (ImageUtil::IconToPNG(hIcon, &pData, &length))
								{
									strIcon = TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length);
									delete[] pData;
								}

								/*
								// currently not used
								if (ImageUtil::IconToGrayscalePNG(hIcon, &pData, &length))
								{
									strGrayscaleIcon = TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length);
									delete[] pData;
								}
								*/

								DestroyIcon(hIcon);
							}
						}
					}

					RegCloseKey(hKeyIcon);
					DEBUG_LOG(TEXT("Closed key (4)"));
				}

				RegCloseKey(hKeyBrowser);
				DEBUG_LOG(TEXT("Closed key (5)"));

				// add a new browser object to the list
				if (success)
				{
					bool isDefaultBrowser = strDefaultBrowserKey.length() > 0 ?
						_tcscmp(strDefaultBrowserKey.c_str(), szSubkeyName) == 0 :
						idx == 0;

					DEBUG_LOG(TEXT("Adding browser ") + strBrowserName);
					(*ppBrowsers)->push_back(new Browser(
						strBrowserName,
						GetExeVersion(strBrowserCommand),
						strBrowserCommand,
						strIcon,
						isDefaultBrowser,
						nStatusCode
					));
					DEBUG_LOG(TEXT("Added browser"));
				}
			}
		}

		RegCloseKey(hKey);
		DEBUG_LOG(TEXT("Closed key (6)"));
	}

	// add Microsoft Edge - they don't believe in the power of the registry...
	TCHAR szWindowsDir[MAX_PATH] = TEXT("");
	ExpandEnvironmentStrings(TEXT("%windir%"), szWindowsDir, MAX_PATH);
	String systemAppDir = szWindowsDir;
	systemAppDir.append(TEXT("\\SystemApps\\"));
	String edgeSearchString = systemAppDir;
	edgeSearchString.append(TEXT("Microsoft.MicrosoftEdge*"));
	
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(edgeSearchString.c_str(), &fd);

	if (INVALID_HANDLE_VALUE != hFind)
	{
		String edgeExe = systemAppDir;
		edgeExe.append(fd.cFileName);
		edgeExe.append(TEXT("\\MicrosoftEdge.exe"));
		
		HICON edgeIcon;
		String strIcon = TEXT("");
		int nIcons = ExtractIconEx(edgeExe.c_str(), 0, &edgeIcon, NULL, 1);

		if (edgeIcon != NULL)
		{
			BYTE* pData = NULL;
			DWORD length;

			if (ImageUtil::IconToPNG(edgeIcon, &pData, &length))
			{
				strIcon = TEXT("data:image/png;base64,") + ImageUtil::Base64Encode(pData, length);
				delete[] pData;
			}

			DestroyIcon(edgeIcon);
		}

		String search = TEXT("MicrosoftEdge");
		bool isEdgeDefault = strDefaultBrowserKey.length() > 0 ?
			strDefaultBrowserKey.find(search) != String::npos :
			false;

		(*ppBrowsers)->push_back(new Browser(
			TEXT("Microsoft Edge"),
			GetExeVersion(edgeExe),
			TEXT("microsoft-edge:"),
			strIcon,
			isEdgeDefault
		));
	}	
}

bool OpenURLInBrowser(String url, Browser* browser)
{
	SHELLEXECUTEINFO execInfo = { 0 };

	execInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	execInfo.fMask = SEE_MASK_DEFAULT;
	execInfo.lpVerb = TEXT("open");
	execInfo.nShow = SW_SHOW;

	if (!browser)
	{
		execInfo.lpFile = url.c_str();
		execInfo.lpParameters = NULL;
	}
	else
	{
		String id = browser->GetIdentifier();

		// in case we're launching Edge, we do this via microsoft-edge:url,
		// and microsoft-edge is the browser identifier in that case
		if (browser->GetName() == TEXT("Microsoft Edge"))
			id.append(url);

		execInfo.lpFile = id.c_str();
		execInfo.lpParameters = url.c_str();
	}

	return ShellExecuteEx(&execInfo) == TRUE;
}

void CleanUp(std::vector<Browser*>* pBrowsers)
{
	for (Browser* pBrowser : *pBrowsers)
		delete pBrowser;
}

} // namespace BrowserUtil
} // namespace Zephyros
