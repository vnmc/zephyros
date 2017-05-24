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


#ifndef Zephyros_ProcessManager_h
#define Zephyros_ProcessManager_h
#pragma once


#include <vector>
#include "base/types.h"

#define TYPE_STDOUT 0
#define TYPE_STDIN 1
#define TYPE_STDERR 2


namespace Zephyros {

typedef struct
{
    int type;
    String text;
} StreamDataEntry;

typedef struct
{
    std::vector<StreamDataEntry> stream;
    CRITICAL_SECTION cs;
} StreamData;

typedef struct
{
    HANDLE hndRead;
    HANDLE hndWrite;
    int type;
    StreamData* pData;
    bool isTerminated;
} PipeData;


class ProcessManager
{
public:
    ProcessManager(CallbackId callbackId, String strExePath, std::vector<String> vecArgs, String strCWD = TEXT(""));
    ~ProcessManager();

    void Start();
    void FireCallback(bool bSuccess, int exitCode);

    static bool CreateProcess(String strExePath, std::vector<String> vecArgs, String strCWD, LPVOID lpEnv,
        PROCESS_INFORMATION* pProcInfo,
        HANDLE* phStdinRead, HANDLE* phStdinWrite, HANDLE* phStdoutRead, HANDLE* phStdoutWrite, HANDLE* phStderrRead, HANDLE* phStderrWrite);

    static void FindInPath(String& strFile);
    static String GetEnvVar(LPCTSTR strVarName);

public:
    PROCESS_INFORMATION m_procInfo;
    HANDLE m_hReadOutThread;
    HANDLE m_hReadErrThread;

    PipeData m_in;
    PipeData m_out;
    PipeData m_err;

private:
    StreamData m_data;

    CallbackId m_callbackId;

    String m_strExePath;
    std::vector<String> m_vecArgs;
    String m_strCWD;
};

} // namespace Zephyros


# endif // Zephyros_ProcessManager_h
