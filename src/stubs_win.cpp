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


#include "stdafx.h"
#include "lib/CrashRpt/CrashRpt.h"


extern "C"
{
#ifdef NO_SPARKLE
    // stubs for WinSparkle (if a Zephyros app isn't linked with WinSparkle)

    void __cdecl win_sparkle_init() {}
    void __cdecl win_sparkle_cleanup() {}
    void __cdecl win_sparkle_set_appcast_url(const char *url) {}
    void __cdecl win_sparkle_check_update_with_ui() {}
    void __cdecl win_sparkle_set_automatic_check_for_updates(int state) {}
    int __cdecl win_sparkle_get_automatic_check_for_updates() { return 0; }
    void __cdecl win_sparkle_set_update_check_interval(int interval) {}
    int __cdecl win_sparkle_get_update_check_interval() { return 0; }
#endif

#ifdef NO_CRASHRPT
    // stubs for CrashRpt (if a Zephyros app isn't linked with CrashRpt)

    int _stdcall crInstallA(PCR_INSTALL_INFOA pInfo) { return 0; }
    int _stdcall crInstallW(__in PCR_INSTALL_INFOW pInfo) { return 0; }
    int _stdcall crSetCrashCallbackW(PFNCRASHCALLBACKW pfnCallbackFunc, LPVOID lpParam) { return 0; }
    int _stdcall crUninstall() { return 0; }
    int _stdcall crAddFile2A(LPCSTR pszFile, LPCSTR pszDestFile, LPCSTR pszDesc, DWORD dwFlags) { return 0; }
    int _stdcall crAddFile2W(LPCWSTR pszFile, LPCWSTR pszDestFile, LPCWSTR pszDesc, DWORD dwFlags) { return 0; }
    int _stdcall crGetLastErrorMsgA(LPSTR pszBuffer, UINT uBuffSize) { return 0; }
    int _stdcall crGetLastErrorMsgW(LPWSTR pszBuffer, UINT uBuffSize) { return 0; }
    int _stdcall crInstallToCurrentThread2(DWORD dwFlags) { return 0; }
    int _stdcall crUninstallFromCurrentThread() { return 0; }
#endif
}
