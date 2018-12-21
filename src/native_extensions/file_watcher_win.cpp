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


#include <vector>
#include <set>

#include "lib/CrashRpt/CrashRpt.h"

#include "base/app.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "native_extensions/file_watcher.h"



//////////////////////////////////////////////////////////////////////////
// Constants

#define MAX_BUF_LEN 16384


//////////////////////////////////////////////////////////////////////////
// Worker Thread Helper Functions

//
// Determines whether the file named filename is empty.
//
bool isFileEmpty(String directory, String filename)
{
    String path = directory;
    path.append(TEXT("\\"));
    path.append(filename);

    HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return true;

    DWORD sizeHi;
    DWORD sizeLo = GetFileSize(hFile, &sizeHi);
    bool isEmpty = sizeLo == INVALID_FILE_SIZE || (sizeLo == 0 && sizeHi == 0);
    CloseHandle(hFile);

    return isEmpty;
}

String GetAbsoluteFilename(Zephyros::FileWatcher* pWatcher, String filename)
{
    String path = pWatcher->m_path.GetPath();
    String ret = StringReplace(pWatcher->m_path.GetPath() + TEXT("\\") + filename, TEXT("\\\\"), TEXT("\\"));
    if (path.substr(0, 2) == TEXT("\\\\"))
        ret = TEXT("\\") + ret;
    return ret;
}

//
// Check the changed files. Add them to the change set if the file extension matches
// one of the entries in the file watcher's extensions vector or if the vector is empty
// (which means that all files are to be watched).
// For non-empty files, fire a change event only if during a certain period of time
// (WAIT_FOR_NONEMPTY_FILES_TIMEOUT_SECONDS) no further changes have occurred.
// For empty files, fire a change event after a fixed amout of time
// (WAIT_FOR_EMPTY_FILES_TIMEOUT_SECONDS) if no change event has been fired yet during
// this wait time.
//
void ProcessChangedFiles(Zephyros::FileWatcher* pWatcher, BYTE* buf, std::set<String>& allChangedFilenames, HANDLE hTimerEmptyFile, HANDLE hTimerNonemptyFile)
{
    DWORD numBytesReturned;
    std::vector<String> changedFilenames;
    std::vector<String> checkAgainLaterFilenames;

    if (!GetOverlappedResult(pWatcher->m_hDirectory, &(pWatcher->m_overlapped), &numBytesReturned, FALSE))
        return;
    if (numBytesReturned == 0)
        return;

    for (FILE_NOTIFY_INFORMATION* pInfo = (FILE_NOTIFY_INFORMATION*) buf; ; )
    {
        if (pInfo->Action != 0)
        {
            String filename = String(pInfo->FileName, (String::size_type) pInfo->FileNameLength / sizeof(TCHAR));

            if (pWatcher->m_fileExtensions.empty())
            {
                // if no file extensions have been defined, send the event to the delegate every time
                if (isFileEmpty(pWatcher->m_path.GetPath(), filename))
                    checkAgainLaterFilenames.push_back(filename);
                else
                    changedFilenames.push_back(filename);
            }
            else
            {
                // otherwise check for the file extension
                for (String fileext : pWatcher->m_fileExtensions)
                {
                    if (StringEndsWith(filename, fileext))
                    {
                        if (isFileEmpty(pWatcher->m_path.GetPath(), filename))
                            checkAgainLaterFilenames.push_back(filename);
                        else
                            changedFilenames.push_back(filename);

                        break;
                    }
                }
            }
        }

        if (pInfo->NextEntryOffset == 0)
            break;
        pInfo = (FILE_NOTIFY_INFORMATION*) (((BYTE*) pInfo) + pInfo->NextEntryOffset);
    }

    // create the vector containing all the modified files
    allChangedFilenames.insert(checkAgainLaterFilenames.begin(), checkAgainLaterFilenames.end());
    allChangedFilenames.insert(changedFilenames.begin(), changedFilenames.end());
    
    // find the right timer to start, depending on the file changes7
    HANDLE hTimer = INVALID_HANDLE_VALUE;
    int waitTimeMilliseconds = 0;
    if (checkAgainLaterFilenames.size() > 0)
    {
        hTimer = hTimerEmptyFile;
        waitTimeMilliseconds = (int) (WAIT_FOR_EMPTY_FILES_TIMEOUT_SECONDS * 1000);
    }
    else if (changedFilenames.size() > 0)
    {
        if (pWatcher->m_delay == 0.0)
        {
            if (hTimerEmptyFile != INVALID_HANDLE_VALUE)
                CancelWaitableTimer(hTimerEmptyFile);
            if (hTimerNonemptyFile != INVALID_HANDLE_VALUE)
                CancelWaitableTimer(hTimerNonemptyFile);

            hTimerEmptyFile = INVALID_HANDLE_VALUE;
            hTimerNonemptyFile = INVALID_HANDLE_VALUE;

            std::vector<String> changedFilenamesCopy;
            for (String filename : changedFilenames)
            {
                String absoluteFilename = GetAbsoluteFilename(pWatcher, filename);
                if (pWatcher->HasFileChanged(absoluteFilename))
                    changedFilenamesCopy.push_back(absoluteFilename);
            }
            
            if (changedFilenamesCopy.size() > 0)
                pWatcher->FireFileChanged(changedFilenamesCopy);
        }
        else
        {
            hTimer = hTimerNonemptyFile;
            waitTimeMilliseconds = (int) (pWatcher->m_delay * 1000);
        }
    }

    // start the timer
    if (hTimer != INVALID_HANDLE_VALUE)
    {
        // cancel both existing timers (if they are running)
        CancelWaitableTimer(hTimerEmptyFile);
        CancelWaitableTimer(hTimerNonemptyFile);

        // set the time
        SYSTEMTIME time;
        GetSystemTime(&time);
        time.wMilliseconds += waitTimeMilliseconds;
        if (time.wMilliseconds > 1000)
        {
            time.wSecond += time.wMilliseconds / 1000;
            time.wMilliseconds %= 1000;
        }

        FILETIME ftime;
        SystemTimeToFileTime(&time, &ftime);

        // start the timer
        SetWaitableTimer(hTimer, reinterpret_cast<LARGE_INTEGER*>(&ftime), 0L, NULL, NULL, FALSE);
    }
}


//////////////////////////////////////////////////////////////////////////
// Worker Thread

//
// Worker thread watching for directory changes.
//
DWORD WINAPI WatchDirectory(LPVOID param)
{
    // install exception handlers for this thread
    const TCHAR* szCrashReportingURL = Zephyros::GetCrashReportingURL();
    if (szCrashReportingURL != NULL && szCrashReportingURL[0] != TCHAR('\0'))
        crInstallToCurrentThread2(0);

    Zephyros::FileWatcher* pWatcher = (Zephyros::FileWatcher*) param;

    // create timers to schedule firing file changes
    HANDLE hTimerEmptyFile = CreateWaitableTimer(NULL, FALSE, NULL);
    HANDLE hTimerNonemptyFile = CreateWaitableTimer(NULL, FALSE, NULL);

    HANDLE handles[] = { pWatcher->m_overlapped.hEvent, hTimerEmptyFile, hTimerNonemptyFile, pWatcher->m_hEventTerminate };
    std::set<String> changedFilenames;
    std::vector<String> changedFilenamesCopy;

    // monitor the first directory/file change
    BYTE* buf = new BYTE[MAX_BUF_LEN];
    ZeroMemory(buf, MAX_BUF_LEN);

    DWORD numBytesReturned;
    BOOL success = false;

    success = ReadDirectoryChangesW(
        pWatcher->m_hDirectory,
        buf,
        MAX_BUF_LEN,
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
        &numBytesReturned,
        &(pWatcher->m_overlapped),
        NULL
    );

    while (true)
    {
        // wait for a change to occur or for the termination event
        DWORD ret = WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
        switch (ret)
        {
        case WAIT_OBJECT_0:
            // the overlapped event has signaled
            if (success)
                ProcessChangedFiles(pWatcher, buf, changedFilenames, hTimerEmptyFile, hTimerNonemptyFile);

            // monitor the next directory/file change
            success = ReadDirectoryChangesW(
                pWatcher->m_hDirectory,
                buf,
                MAX_BUF_LEN,
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &numBytesReturned,
                &(pWatcher->m_overlapped),
                NULL
            );
            break;

        case WAIT_OBJECT_0 + 1:
            // the empty file timer has signaled
            {
                changedFilenamesCopy.clear();

                for (String filename : changedFilenames)
                {
                    String absoluteFilename = GetAbsoluteFilename(pWatcher, filename);
                    if (pWatcher->HasFileChanged(absoluteFilename))
                        changedFilenamesCopy.push_back(absoluteFilename);
                }
                
                changedFilenames.clear();

                if (changedFilenamesCopy.size() > 0)
                    pWatcher->FireFileChanged(changedFilenamesCopy);
            }
            break;

        case WAIT_OBJECT_0 + 2:
            // the non-empty file timer has signaled
            {
                CancelWaitableTimer(hTimerEmptyFile);
                changedFilenamesCopy.clear();

                for (String filename : changedFilenames)
                {
                    String absoluteFilename = GetAbsoluteFilename(pWatcher, filename);
                    if (pWatcher->HasFileChanged(absoluteFilename))
                        changedFilenamesCopy.push_back(absoluteFilename);
                }

                changedFilenames.clear();

                if (changedFilenamesCopy.size() > 0)
                    pWatcher->FireFileChanged(changedFilenamesCopy);
            }
            break;

        case WAIT_OBJECT_0 + 3:
            // the terminate event was signaled; terminate this thread
            CloseHandle(hTimerEmptyFile);
            CloseHandle(hTimerNonemptyFile);
            delete[] buf;

            // unset exception handlers before exiting the thread
            crUninstallFromCurrentThread();

            ExitThread(0);
            return 0;

        case WAIT_FAILED:
            Zephyros::App::ShowErrorMessage();
            break;
        }
    }

    // unset exception handlers before exiting the thread
    crUninstallFromCurrentThread();

    return 0;
}


//////////////////////////////////////////////////////////////////////////
// FileWatcher Implementation

namespace Zephyros {

FileWatcher::FileWatcher()
  : m_hDirectory(INVALID_HANDLE_VALUE),
    m_hFileWatcherThread(INVALID_HANDLE_VALUE)
{
    m_hEventTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
}

FileWatcher::~FileWatcher()
{
    Stop();
    CloseHandle(m_hEventTerminate);
}

void FileWatcher::Start(Path& path, std::vector<String>& fileExtensions, double delay)
{
    // if watching is already running, turn it off first
    if (m_hDirectory != INVALID_HANDLE_VALUE)
        Stop();

    // set new configuration
    m_path = path;
    m_delay = delay;
    m_fileExtensions.clear();
    m_fileExtensions.insert(m_fileExtensions.end(), fileExtensions.begin(), fileExtensions.end());
        
    m_hDirectory = CreateFile(
        m_path.GetPath().c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (m_hDirectory == INVALID_HANDLE_VALUE)
    {
        String msg = TEXT("Could not start watching the directory ");
        msg.append(m_path.GetPath());

        DWORD errCode = GetLastError();
        bool hasOwnErrorMsg = true;
        if (errCode == ERROR_PATH_NOT_FOUND)
            msg.append(TEXT(" because it doesn't exist."));
        else if (errCode == ERROR_ACCESS_DENIED)
            msg.append(TEXT(" because the access to it was denied."));
        else
        {
            hasOwnErrorMsg = false;
            App::ShowErrorMessage();
        }

        if (hasOwnErrorMsg)
            App::Alert(TEXT("File Watching Error"), msg, App::AlertStyle::AlertWarning);

        return;
    }

    // start watching
    ZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    m_overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // create the thread waiting for changes
    m_hFileWatcherThread = CreateThread(NULL, 0, WatchDirectory, this, 0, NULL);
}

void FileWatcher::Stop()
{
    if (m_hDirectory == INVALID_HANDLE_VALUE)
        return;

    SetEvent(m_hEventTerminate);
    WaitForSingleObject(m_hFileWatcherThread, INFINITE);

    CloseHandle(m_hDirectory);
    m_hDirectory = INVALID_HANDLE_VALUE;

    CloseHandle(m_overlapped.hEvent);
}

bool FileWatcher::ReadFile(String filePath, char** pBuf, size_t* pLen)
{
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD numBytesRead = 0;
    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    // we don't want to read too large files
    _ASSERT(fileSize.HighPart == 0);

    // allocate buffer and read file
    *pLen = fileSize.LowPart;
    *pBuf = new char[fileSize.LowPart];
    ::ReadFile(hFile, (LPVOID) *pBuf, (DWORD) fileSize.LowPart, &numBytesRead, NULL);

    CloseHandle(hFile);
    return true;
}

} // namespace Zephyros
