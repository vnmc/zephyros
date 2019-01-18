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


#import <sys/stat.h>

#import "base/logging.h"

#import "native_extensions/file_watcher.h"

#ifdef USE_CEF
#import "base/cef/client_handler.h"
#import "base/cef/extension_handler.h"
#endif

#import "native_extensions.h"


bool isFileEmpty(NSString* fileName)
{
    struct stat fileInfo;
    if (stat ([fileName fileSystemRepresentation], &fileInfo) < 0)
        return YES; // file doesn't exist
    return fileInfo.st_size == 0;
}

void fsevents_callback(
    ConstFSEventStreamRef streamRef, void *userData, size_t numEvents, void *eventPaths,
    const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[])
{
    //DEBUG_LOG(@"-- File changed --");
    Zephyros::FileWatcher *me = (Zephyros::FileWatcher*) userData;
    
    // process the events
    std::vector<std::string> changedFilenames;
    std::vector<std::string> checkAgainLaterFilenames;
    for (size_t i = 0; i < numEvents; i++)
    {
        NSString *fileName = (NSString*) CFArrayGetValueAtIndex((CFArrayRef) eventPaths, i);
        std::string strFileName = [fileName UTF8String];
        
        // if no file extensions have been defined, send the event to the delegate every time
        if (me->m_fileExtensions.empty())
        {
            if (isFileEmpty(fileName))
                checkAgainLaterFilenames.push_back(strFileName);
            else
                changedFilenames.push_back(strFileName);
        }
        else
        {
            // otherwise check for the file extension
            for (String extension : me->m_fileExtensions)
            {
                NSString *ext = [NSString stringWithUTF8String: extension.c_str()];
                if ([fileName hasSuffix: ext])
                {
                    if (isFileEmpty(fileName))
                    {
                        checkAgainLaterFilenames.push_back(strFileName);
                        //DEBUG_LOG(@"-- CheckAgainLater: %@", fileName);
                    }
                    else
                    {
                        changedFilenames.push_back(strFileName);
                        //DEBUG_LOG(@"-- Changed: %@", fileName);
                    }
                    
                    break;
                }
            }
        }
    }
    
    // send the event to the delegate if the a file with extension in _fileExtensions has changed
    std::vector<std::string> allChangedFilenames;
    allChangedFilenames.insert(allChangedFilenames.end(), checkAgainLaterFilenames.begin(), checkAgainLaterFilenames.end());
    allChangedFilenames.insert(allChangedFilenames.end(), changedFilenames.begin(), changedFilenames.end());
    
    if (checkAgainLaterFilenames.size() > 0)
        me->ScheduleEmptyFileCheck(allChangedFilenames);
    else if (changedFilenames.size() > 0)
        me->ScheduleNonEmptyFileCheck(allChangedFilenames);
}


@implementation TimerDelegate

- (id) initWithFileWatcher: (Zephyros::FileWatcher*) fileWatcher
{
    self = [super init];
    m_fileWatcher = fileWatcher;
    return self;
}

- (void) fireFileChangeEvents: (NSArray*) arrFilenames
{
    std::vector<std::string> filenames;
    for (NSString *filename in arrFilenames)
    {
        String strFilename([filename UTF8String]);
        if (m_fileWatcher->HasFileChanged(strFilename))
            filenames.push_back(strFilename);
    }
    
    if (filenames.size() > 0)
        m_fileWatcher->FireFileChanged(filenames);
}

- (void) onNonemptyFileTimeout: (NSTimer*) timer
{
    // invalidate the "empty" timeout if there was one and it hasn't fired yet
    //DEBUG_LOG(@"Non-empty timeout fired");
    
    m_fileWatcher->m_emptyFileTimeoutCanceled = YES;
    
    if (m_fileWatcher->m_nonemptyFileTimeout != nil)
        m_fileWatcher->m_nonemptyFileTimeout = nil;
    
    // end disabling app nap
    if (m_fileWatcher->m_activity != nil)
    {
        [[NSProcessInfo processInfo] endActivity: m_fileWatcher->m_activity];
        m_fileWatcher->m_activity = nil;
    }
    
    [self fireFileChangeEvents: (NSArray*) timer.userInfo];
}

@end


namespace Zephyros {

FileWatcher::FileWatcher()
  : m_stream(nil),
    m_nonemptyFileTimeout(nil),
    m_emptyFileTimeoutCanceled(NO),
    m_activity(nil)
{
    m_timerDelegate = [[TimerDelegate alloc] initWithFileWatcher: this];
}

FileWatcher::~FileWatcher()
{
}

void FileWatcher::Start(Path& path, std::vector<std::string>& fileExtensions, double delay)
{
    if (m_stream != nil)
        Stop();
    
    m_path = path;
    m_delay = delay;
    std::string localPrefix = "file://localhost";
    NSString *dir = [NSString stringWithUTF8String: (m_path.GetPath().substr(0, localPrefix.size()) == localPrefix) ? m_path.GetPath().substr(localPrefix.size()).c_str() : m_path.GetPath().c_str()];
    
    // check whether the directory exists
    BOOL isDirectory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath: dir isDirectory: &isDirectory])
        return;
    if (!isDirectory)
        return;
    
    m_fileExtensions.clear();
    m_fileExtensions.insert(m_fileExtensions.end(), fileExtensions.begin(), fileExtensions.end());
        
    // start watching...
    if (m_path.HasSecurityAccessData())
        [[NSURL URLWithString: [NSString stringWithUTF8String: m_path.GetURLWithSecurityAccessData().c_str()]] startAccessingSecurityScopedResource];
    
    NSArray *pathsToWatch = @[dir];
    FSEventStreamContext context = { 0, (void*) this, NULL, NULL, NULL };
    
    m_stream = FSEventStreamCreate(
        NULL,
        &fsevents_callback,
        &context,
        (__bridge CFArrayRef) (pathsToWatch),
        kFSEventStreamEventIdSinceNow,
        (CFAbsoluteTime) FILE_WATCH_LATENCY,
        kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagFileEvents
    );

    if (m_stream == nil)
        return;
    
    FSEventStreamScheduleWithRunLoop(m_stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(m_stream);
}

void FileWatcher::Stop()
{
    if (m_stream == nil)
        return;
    
    if (m_path.HasSecurityAccessData())
        [[NSURL URLWithString: [NSString stringWithUTF8String: m_path.GetURLWithSecurityAccessData().c_str()]] stopAccessingSecurityScopedResource];

    FSEventStreamStop(m_stream);
    FSEventStreamInvalidate(m_stream);
    FSEventStreamRelease(m_stream);
    m_stream = nil;
    
    m_fileHashes.clear();
}

void FileWatcher::ScheduleNonEmptyFileCheck(std::vector<std::string>& filenames)
{
    if (m_nonemptyFileTimeout == nil || !m_nonemptyFileTimeout.isValid)
    {
        // create a new timer
        //DEBUG_LOG(@"Creating new timer for non-empty file");
        
        NSMutableArray *arrFilenames = [[NSMutableArray alloc] init];
        for (String filename : filenames)
            [arrFilenames addObject: [NSString stringWithUTF8String: filename.c_str()]];

        if (m_delay == 0.0)
        {
            // fire the events immediately if no delay was configured
            [m_timerDelegate fireFileChangeEvents: arrFilenames];
        }
        else
        {
            // disable app nap until the timer has been invoked
            if (m_activity == nil && [[NSProcessInfo processInfo] respondsToSelector: @selector(beginActivityWithOptions:reason:)])
                m_activity = [[NSProcessInfo processInfo] beginActivityWithOptions: NSActivityUserInitiated reason: @"File watching"];
            
            
            m_nonemptyFileTimeout = [NSTimer scheduledTimerWithTimeInterval: m_delay
                                                                     target: m_timerDelegate
                                                                   selector: @selector(onNonemptyFileTimeout:)
                                                                   userInfo: arrFilenames
                                                                    repeats: NO];
        }
    }
    else
    {
        //DEBUG_LOG(@"Postponing firing");
        
        // add additional filenames
        NSMutableArray *arrFilenames = (NSMutableArray*) m_nonemptyFileTimeout.userInfo;
        for (String filename : filenames)
            [arrFilenames addObject: [NSString stringWithUTF8String: filename.c_str()]];
        
        // postpone firing
        m_nonemptyFileTimeout.fireDate = [NSDate dateWithTimeIntervalSinceNow: m_delay];
    }
}

void FileWatcher::ScheduleEmptyFileCheck(std::vector<std::string>& filenames)
{
    //DEBUG_LOG(@"Creating new timer for empty file");
    
    m_emptyFileTimeoutCanceled = NO;
    FileWatcher *me = this;
    
    // copy the filenames to the heap
    size_t len = filenames.size();
    char** pFilenames = new char*[len];
    int i = 0;
    for (std::string filename : filenames)
    {
        pFilenames[i] = new char[filename.length() + 1];
        strcpy(pFilenames[i], filename.c_str());
        i++;
    }
    
    // disable app nap until the delayed code has been executed
    if (m_activity == nil && [[NSProcessInfo processInfo] respondsToSelector: @selector(beginActivityWithOptions:reason:)])
        m_activity = [[NSProcessInfo processInfo] beginActivityWithOptions: NSActivityUserInitiated reason: @"File watching"];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, fmax(m_delay, WAIT_FOR_EMPTY_FILES_TIMEOUT_SECONDS) * NSEC_PER_SEC), dispatch_get_main_queue(),
    ^{
        //DEBUG_LOG(@"Empty file timout, canceled=%d", _emptyFileTimeoutCanceled);
        
        if (!me->m_emptyFileTimeoutCanceled)
        {
            std::vector<String> f;
            for (unsigned int i = 0; i < len; ++i)
                if (HasFileChanged(pFilenames[i]))
                    f.push_back(pFilenames[i]);
            
            if (f.size() > 0)
                me->FireFileChanged(f);
        }
        
        // delete the filenames from the heap
        for (unsigned int i = 0; i < len; ++i)
            delete[] pFilenames[i];
        delete[] pFilenames;
        
        if (me->m_activity != nil)
        {
            [[NSProcessInfo processInfo] endActivity: me->m_activity];
            me->m_activity = nil;
        }
    });
}

bool FileWatcher::ReadFile(String filePath, char** pBuf, size_t* pLen)
{
    NSData *data = [[NSData alloc] initWithContentsOfFile: [NSString stringWithUTF8String: filePath.c_str()]];
    if (data == nil)
        return false;

    *pLen = data.length;
    *pBuf = new char[data.length];
    
    [data getBytes: *pBuf length: data.length];
    return true;
}
    
} // namespace Zephyros
