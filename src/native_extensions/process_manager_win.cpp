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


#include <tchar.h>
#include <Shlwapi.h>
#include <algorithm>

#include "lib/CrashRpt/CrashRpt.h"

#include "base/app.h"
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "native_extensions/os_util.h"
#include "native_extensions/process_manager.h"


//////////////////////////////////////////////////////////////////////////
// Constants

#define BUF_SIZE 8192


//////////////////////////////////////////////////////////////////////////
// Worker Thread Functions

extern CefRefPtr<Zephyros::ClientHandler> g_handler;

DWORD WINAPI ReadOutput(LPVOID param)
{
	// install exception handlers for this thread
	const TCHAR* szCrashReportingURL = Zephyros::GetCrashReportingURL();
	if (szCrashReportingURL != NULL && szCrashReportingURL[0] != TCHAR('\0'))
		crInstallToCurrentThread2(0);

	Zephyros::PipeData* p = (Zephyros::PipeData*) param;

	DWORD numBytesRead;
	DWORD numBytesAvail;
	char buf[BUF_SIZE + 1];

	for ( ; ; )
	{
		PeekNamedPipe(p->hndRead, NULL, 0, NULL, &numBytesAvail, NULL);
		if (numBytesAvail == 0 && p->isTerminated)
			break;

		// read stdout of the process and process the output
		ReadFile(p->hndRead, buf, BUF_SIZE, &numBytesRead, NULL);

		if (numBytesRead > 0)
		{
			Zephyros::StreamDataEntry entry;
			entry.type = p->type;
			entry.text = String(buf, buf + numBytesRead);

			EnterCriticalSection(&(p->pData->cs));
			p->pData->stream.push_back(entry);
			LeaveCriticalSection(&(p->pData->cs));
		}
	}

	CloseHandle(p->hndRead);
	CloseHandle(p->hndWrite);

	// unset exception handlers before exiting the thread
	crUninstallFromCurrentThread();

	return 0;
}

DWORD WINAPI WaitForProcessProc(LPVOID param)
{
	Zephyros::ProcessManager* pMgr = (Zephyros::ProcessManager*) param;
	DWORD dwExitCode = 0;

	// wait until the process has terminated
	WaitForSingleObject(pMgr->m_procInfo.hProcess, INFINITE);
	GetExitCodeProcess(pMgr->m_procInfo.hProcess, &dwExitCode);

	// wait until the reading threads have termiated
	pMgr->m_in.isTerminated = true;
	pMgr->m_out.isTerminated = true;
	pMgr->m_err.isTerminated = true;
	HANDLE handles[] = { pMgr->m_hReadOutThread, pMgr->m_hReadErrThread };
	WaitForMultipleObjects(2, handles, TRUE, 1000);

	CloseHandle(pMgr->m_procInfo.hProcess);
	CloseHandle(pMgr->m_procInfo.hThread);

	// return the result and clean up
	pMgr->FireCallback(true, (int) dwExitCode);
	delete pMgr;

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// ProcessManager Implementation

namespace Zephyros {

ProcessManager::ProcessManager(CallbackId callbackId, String strExePath, std::vector<String> vecArgs, String strCWD)
  : m_callbackId(callbackId),
	m_strExePath(strExePath),
	m_vecArgs(vecArgs),
	m_strCWD(strCWD)
{
	InitializeCriticalSection(&m_data.cs);

	if (m_strCWD == TEXT("~"))
		m_strCWD = OSUtil::GetHomeDirectory();
}

ProcessManager::~ProcessManager()
{
	DeleteCriticalSection(&m_data.cs);
}

void ProcessManager::Start()
{
	m_data.stream.clear();

	m_in.isTerminated = false;
	m_out.isTerminated = false;
	m_err.isTerminated = false;

	m_in.type = TYPE_STDIN;
	m_out.type = TYPE_STDOUT;
	m_err.type = TYPE_STDERR;

	m_in.pData = &m_data;
	m_out.pData = &m_data;
	m_err.pData = &m_data;

	if (ProcessManager::CreateProcess(m_strExePath, m_vecArgs, m_strCWD, NULL, &m_procInfo,
		&m_in.hndRead, &m_in.hndWrite, &m_out.hndRead, &m_out.hndWrite, &m_err.hndRead, &m_err.hndWrite))
	{
		m_hReadOutThread = CreateThread(NULL, 0, ReadOutput, &m_out, 0, NULL);
		m_hReadErrThread = CreateThread(NULL, 0, ReadOutput, &m_err, 0, NULL);
		CreateThread(NULL, 0, WaitForProcessProc, this, 0, NULL);
	}
	else
	{
		FireCallback(false, -1);
		delete this;
	}
}

void ProcessManager::FireCallback(bool bSuccess, int exitCode)
{
	JavaScript::Array args = JavaScript::CreateArray();

	if (bSuccess)
	{
		JavaScript::Array stream = JavaScript::CreateArray();
		int i = 0;
		for (StreamDataEntry e : m_data.stream)
		{
			JavaScript::Object streamEntry = JavaScript::CreateObject();
			streamEntry->SetInt(TEXT("fd"), e.type);
			streamEntry->SetString(TEXT("text"), e.text);
			stream->SetDictionary(i++, streamEntry);
		}

		args->SetInt(0, exitCode);
		args->SetList(1, stream);
	}
	else
		args->SetNull(0);

	g_handler->GetClientExtensionHandler()->InvokeCallback(m_callbackId, args);
}

bool ProcessManager::CreateProcess(
	String strExePath,
	std::vector<String> vecArgs,
	String strCWD,
	LPVOID lpEnv,
	PROCESS_INFORMATION* pProcInfo,
	HANDLE* phStdinRead, HANDLE* phStdinWrite,
	HANDLE* phStdoutRead, HANDLE* phStdoutWrite,
	HANDLE* phStderrRead, HANDLE* phStderrWrite)
{
	// create pipes
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE h1, h2, h3, h4;
	if (phStdinRead == NULL)
		phStdinRead = &h1;
	if (phStdinWrite == NULL)
		phStdinWrite = &h2;
	if (phStdoutRead == NULL)
		phStdoutRead = &h3;
	if (phStdoutWrite == NULL)
		phStdoutWrite = &h4;
	if (phStderrRead == NULL)
		phStderrRead = phStdoutRead;
	if (phStderrWrite == NULL || phStderrRead == phStdoutRead)
		phStderrWrite = phStdoutWrite;

	CreatePipe(phStdinRead, phStdinWrite, &sa, 0);
	SetHandleInformation(*phStdinWrite, HANDLE_FLAG_INHERIT, 0);

	CreatePipe(phStdoutRead, phStdoutWrite, &sa, 0);
	SetHandleInformation(*phStdoutRead, HANDLE_FLAG_INHERIT, 0);

	if (phStderrRead != phStdoutRead)
	{
		CreatePipe(phStderrRead, phStderrWrite, &sa, 0);
		SetHandleInformation(*phStderrRead, HANDLE_FLAG_INHERIT, 0);
	}	

	// set up members of the PROCESS_INFORMATION structure
	ZeroMemory(pProcInfo, sizeof(PROCESS_INFORMATION));
 
	// set up members of the STARTUPINFO structure
	// this structure specifies the STDIN and STDOUT handles for redirection
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdInput = *phStdinRead;
	startupInfo.hStdOutput = *phStdoutWrite;
	startupInfo.hStdError = *phStderrWrite;
	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// prepare the command line
	String cmdLine;
	size_t pos = strExePath.find_last_of(TEXT('.'));
	String extension = pos == String::npos ? TEXT("") : strExePath.substr(pos);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension != TEXT(".exe"))
	{
		// if this is no exe file, use "cmd.exe /C <file> <args>"
		String cmd = ProcessManager::GetEnvVar(TEXT("ComSpec"));
		
		bool hasSpaces = strExePath.find(TEXT(" ")) != String::npos;
		cmdLine = cmd + TEXT(" /C ");
		if (hasSpaces)
			cmdLine.append(TEXT("\""));
		cmdLine.append(strExePath);
		if (hasSpaces)
			cmdLine.append(TEXT("\""));

		strExePath = cmd;
	}
	else
	{
		// resolve the path if needed
		if (strExePath.find(TEXT(":\\")) == String::npos)
			ProcessManager::FindInPath(strExePath);

		cmdLine = TEXT("\"") + strExePath + TEXT("\"");	
	}

	// all arguments are wrapped in quotes, and quotes are escaped with double quotes
	// escape '\"' by '\\""'
	for (String arg : vecArgs)
	{
		bool hasSpacesOrQuotationMarks = (arg.find(TEXT(" ")) != String::npos) || (arg.find(TEXT("\"")) != String::npos);

		if (hasSpacesOrQuotationMarks)
		{
			cmdLine.append(TEXT(" \""));
			cmdLine.append(StringReplace(StringReplace(arg, TEXT("\""), TEXT("\"\"")), TEXT("\\\"\""), TEXT("\\\\\"\"")));
			cmdLine.append(TEXT("\""));
		}
		else
		{
			cmdLine.append(TEXT(" "));
			cmdLine.append(arg);
		}
	}

#ifndef NDEBUG
	App::Log(cmdLine);
#endif

	// start the child process
	TCHAR* szCmdLine = new TCHAR[cmdLine.length() + 1];
	_tcscpy(szCmdLine, cmdLine.c_str());

	BOOL bSuccess = ::CreateProcess(
		strExePath.c_str(),
		szCmdLine,
		NULL,
		NULL,
		TRUE,
		0,
		lpEnv,
		strCWD == TEXT("") ? NULL : strCWD.c_str(),
		&startupInfo,
		pProcInfo
	);

	delete[] szCmdLine;

	if (!bSuccess)
		App::ShowErrorMessage();

	return bSuccess == TRUE;
}

void ProcessManager::FindInPath(String& strFile)
{
	bool HasExtension = strFile.find(TEXT(".")) != String::npos;

	StringStream ssPath(ProcessManager::GetEnvVar(TEXT("PATH")));
	String path;

	while (std::getline(ssPath, path, TEXT(';')))
	{
		String strFileWithPath = path;
		if (path.at(path.length() - 1) != TEXT('\\'))
			strFileWithPath += TEXT("\\");
		strFileWithPath += strFile;

		if (HasExtension && PathFileExists(strFileWithPath.c_str()))
		{
			strFile = strFileWithPath;
			break;
		}

		String strFileWithPathAndExtension = strFileWithPath + TEXT(".exe");
		if (PathFileExists(strFileWithPathAndExtension.c_str()))
		{
			strFile = strFileWithPathAndExtension;
			break;
		}

		strFileWithPathAndExtension = strFileWithPath + TEXT(".bat");
		if (PathFileExists(strFileWithPathAndExtension.c_str()))
		{
			strFile = strFileWithPathAndExtension;
			break;
		}
	}
}

String ProcessManager::GetEnvVar(LPCTSTR strVarName)
{
	DWORD dwLen = GetEnvironmentVariable(strVarName, NULL, 0);
	TCHAR* pBuf = new TCHAR[dwLen + 1];

	GetEnvironmentVariable(strVarName, pBuf, dwLen);

	String str(pBuf);
	delete[] pBuf;

	return str;
}

} // namespace Zephyros
