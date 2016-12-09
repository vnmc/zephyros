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


#ifndef Zephyros_OSUtil_h
#define Zephyros_OSUtil_h
#pragma once


#include "base/types.h"
#include "jsbridge.h"
#ifdef USE_WEBVIEW
#include "native_extensions.h"
#endif


#ifdef OS_WIN
typedef ULONG_PTR MenuHandle;
#else
typedef unsigned long MenuHandle;
#endif


#define CONTEXT_MENU_STARTID 90000


namespace Zephyros {
namespace OSUtil {

String GetOSVersion();
String GetComputerName();
String GetUserName();
String GetHomeDirectory();

void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd);

#ifdef OS_LINUX
String GetConfigDirectory();
String Exec(String command);
#endif

void CreateMenu(JavaScript::Array menuItems);
MenuHandle CreateContextMenu(JavaScript::Array menuItems);
String ShowContextMenu(MenuHandle nMenuHandle, int x, int y);
void RemoveMenuItem(String strCommandId);

void CreateTouchBar(JavaScript::Array touchBarItems);
    
#ifdef OS_WIN
void GetWindowBorderSize(POINT* pPtBorder);
#endif

void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight);
void SetMinimumWindowSize(int width, int height);

void DisplayNotification(String title, String details);

void RequestUserAttention();

void CopyToClipboard(String text);

void CleanUp();

#ifdef OS_WIN
BOOL IsWin7OrLater();
BOOL IsWin8OrLater();
BOOL IsWinVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion);
#endif

} // namespace OSUtil
} // namespace Zephyros


#endif // Zephyros_OSUtil_h
