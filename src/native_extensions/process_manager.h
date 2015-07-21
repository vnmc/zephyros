//
//  server_manager.h
//  Ghostlab
//
//  Created by Matthias Christen on 05.09.13.
//
//

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
