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


#ifndef Zephyros_FileWatcher_h
#define Zephyros_FileWatcher_h
#pragma once


#ifdef OS_MACOSX
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#else
#include <objc/objc.h>
#endif
#endif

#include <vector>
#include <map>
#include <string>

#include "base/types.h"
#include "native_extensions/path.h"
#include "util/MurmurHash3.h"


//////////////////////////////////////////////////////////////////////////
// Constants

// The latency with which to check for file changes
#define FILE_WATCH_LATENCY 0.5

// The wait period when a non-empty file has changed until the reload event is broadcast.
// This value has to be greater than FILE_WATCH_LATENCY to be effective.
#define WAIT_FOR_NONEMPTY_FILES_TIMEOUT_SECONDS 0.6

// The timeout until a reload event is broadcast after a file has become empty
// (If the file "regains" contents within the timeout period, a reload will be broadcast then.)
#define WAIT_FOR_EMPTY_FILES_TIMEOUT_SECONDS 3


//////////////////////////////////////////////////////////////////////////
// Forward Declarations

namespace Zephyros {

class FileWatcher;

}


//////////////////////////////////////////////////////////////////////////
// Helpers

#ifdef OS_MACOSX
#ifdef __OBJC__

@interface TimerDelegate : NSObject
{
    Zephyros::FileWatcher* m_fileWatcher;
}

- (void) onNonemptyFileTimeout: (NSTimer*) timer;

@end

typedef TimerDelegate* TimerDelegateRef;
typedef NSTimer* NSTimerRef;

#else

typedef id TimerDelegateRef;
typedef id FSEventStreamRef;
typedef id NSTimerRef;

#endif // __OBJC__
#endif // OS_MACOSX


typedef struct {
    size_t length;
    uint32_t value[4];
} Hash;


//////////////////////////////////////////////////////////////////////////
// FileWatcher Definition

namespace Zephyros {

class FileWatcher
{
    //////////////////////////////////////////////////////////////////////
    // Public Methods

public:
    FileWatcher();
    ~FileWatcher();
    
    void Start(Path& path, std::vector<String>& fileExtensions);
    void Stop();
    
    void FireFileChanged(std::vector<String>& files);
    
#ifdef OS_MACOSX
    void ScheduleNonEmptyFileCheck(std::vector<String>& filenames);
    void ScheduleEmptyFileCheck(std::vector<String>& filenames);
#endif
    
    
    inline bool HasFileChanged(String filePath)
    {
        size_t len = 0;
        char* buf = NULL;
        
        if (!ReadFile(filePath, &buf, &len))
            return true;
            
        bool hasChanged = HasFileChanged(filePath, buf, len);
        delete[] buf;
        
        return hasChanged;
    }

    
private:
    bool ReadFile(String filePath, char** pBuf, size_t* pLen);
    
    bool HasFileChanged(String filePath, char* pData, size_t len)
    {
        Hash oldHash;
        bool hasChanged = false;
        
        // retrieve the old hash if there is one
        if (m_fileHashes.find(filePath) != m_fileHashes.end())
            oldHash = m_fileHashes[filePath];
        else
            hasChanged = true;
        
        // compute the new hash
        Hash newHash;
        newHash.length = len;
#if defined(_WIN64) || defined(OS_MACOSX)
        MurmurHash3_x64_128(pData, (int) len, 8005, newHash.value);
#else
        MurmurHash3_x86_128(pData, (int) len, 8005, newHash.value);
#endif
        
        // compare against the old hash
        if (!hasChanged && newHash.length != oldHash.length)
            hasChanged = true;
        if (!hasChanged)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (newHash.value[i] != oldHash.value[i])
                {
                    hasChanged = true;
                    break;
                }
            }
        }
        
        // set the new hash if the hash has changed
        if (hasChanged)
            m_fileHashes[filePath] = newHash;
        
        return hasChanged;
    }
    
    
    //////////////////////////////////////////////////////////////////////
    // Member Variables

public:
    Path m_path;
    std::vector<String> m_fileExtensions;
    std::map<String, Hash> m_fileHashes;
    
#ifdef OS_MACOSX
    FSEventStreamRef m_stream;
    TimerDelegateRef m_timerDelegate;
    NSTimerRef m_nonemptyFileTimeout;
    BOOL m_emptyFileTimeoutCanceled;
    id m_activity;
#endif
#ifdef OS_WIN
	HANDLE m_hDirectory;
	OVERLAPPED m_overlapped;

	HANDLE m_hFileWatcherThread;

	HANDLE m_hEventTerminate;
#endif
};
    
} // namespace Zephyros


#endif // Zephyros_FileWatcher_h
