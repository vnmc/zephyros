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
 * Florian Müller, Vanamco AG
 *******************************************************************************/


#include <vector>
#include <set>
#include <unordered_map>

#include <iostream>
#include <fstream>

#include <sys/inotify.h>
#include <pthread.h>

#include "util/string_util.h"
#include "native_extensions/file_watcher.h"


#define EVENT_SIZE (sizeof(inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))


//////////////////////////////////////////////////////////////////////////
// Type Definitions

typedef struct
{
    Zephyros::FileWatcher* watcher;
    Zephyros::Path* path;
    std::vector<String>* extensions;
} FileWatchThreadData;


//////////////////////////////////////////////////////////////////////////
// FileWatcher Implementation

bool isFileEmpty(String directory, String filename)
{
    // TODO: implement
    return false;
}


void recurseDir(const char *name, int level, std::unordered_map<int, String> *watchMap, int fd)
{
    DIR* dir = opendir(name);
    if (!dir)
        return;

    dirent* entry = readdir(dir);
    if (!entry)
        return;

    char path[1024];

    do
    {
        if (entry->d_type == DT_DIR)
        {
            int len = snprintf(path, sizeof(path) - 1, TEXT("%s/%s"), name, entry->d_name);
            path[len] = 0;

            if (strcmp(entry->d_name, TEXT(".")) == 0 || strcmp(entry->d_name, TEXT("..")) == 0)
                continue;

            int wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);
            watchMap->insert(std::pair<int, String>(wd, String(path)));
            recurseDir(path, level + 1, watchMap, fd);
        }
    } while (entry = readdir(dir));

    closedir(dir);
}

void processEvent(char* buffer, int len, Zephyros::FileWatcher* watcher, std::unordered_map<int, String> watchMap, std::vector<String>* extensions)
{
   int i = 0;
   std::vector<String> changes;

   while (i < len)
   {
        inotify_event* event = (inotify_event*) &buffer[i];
        std::unordered_map<int, String>::const_iterator item = watchMap.find(event->wd);
        String fileName = String(event->name);

        bool isInExtensions = false;
        for (std::vector<String>::iterator it = extensions->begin(); it != extensions->end(); ++it)
        {
            String ext = String(*it);
            int pos = fileName.find(ext);
            int requiredPos = fileName.length() - ext.length();

            if (requiredPos > 0 && pos == requiredPos)
            {
                isInExtensions = true;
                break;
            }
        }

        if (isInExtensions)
        {
            String s = item->second;
            s.append(TEXT("/"));
            s.append(fileName);
            changes.insert(changes.end(), s);
        }

        i += EVENT_SIZE + event->len;
    }

    if (changes.size() > 0)
        watcher->FireFileChanged(changes);
}


void CleanupThreadData(void* arg)
{
    FileWatchThreadData* data = (FileWatchThreadData*) arg;
    delete data->path;
    delete data->extensions;
    delete data;
}


void* startWatchingThread(void* arg)
{
    char buffer[BUF_LEN];
    FileWatchThreadData* data = (FileWatchThreadData*) arg;

    int fd = inotify_init();
    std::unordered_map<int, String> watchMap;

    int wd = inotify_add_watch(fd, data->path->GetPath().c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    watchMap.insert(std::pair<int, String>(wd, data->path->GetPath()));

    // recursively add all directories below root dir to the watch
    recurseDir(data->path->GetPath().c_str(), 0, &watchMap, fd);

    timespec waitTime;
    waitTime.tv_sec = 60;
    waitTime.tv_nsec = 0;

    fd_set descriptors;

    FD_ZERO(&descriptors);
    FD_SET(fd, &descriptors);
    int ret_val;

    bool doContinue = true;
    pthread_cleanup_push(CleanupThreadData, arg);

    while (doContinue)
    {
        ret_val = pselect(fd + 1, &descriptors, NULL, NULL, &waitTime, NULL);

        if( ret_val < 0)
        {
            //cout << "ERROR: " << ret_val << endl;
        }
        else if (!ret_val)
        {
            //cout << "Timeout occured." << endl;
        }
        else if (FD_ISSET(fd, &descriptors))
        {
            int len = read(fd, buffer, BUF_LEN);
            processEvent(buffer, len, data->watcher, watchMap, data->extensions);
        }
    }

    pthread_cleanup_pop(arg);
}


namespace Zephyros {

FileWatcher::FileWatcher()
{

}

FileWatcher::~FileWatcher()
{
	Stop();
}

void FileWatcher::Start(Path& path, std::vector<String>& fileExtensions)
{
    // prepare thread data
    FileWatchThreadData* data = new FileWatchThreadData;
    data->watcher = this;
    data->path = new Path(path);
    data->extensions = new std::vector<String>();

    for (String ext : fileExtensions)
        data->extensions->push_back(ext);

    // start thread
    pthread_create(&m_thread, NULL, startWatchingThread, data);

}

void FileWatcher::Stop()
{
    // cancel thread
    // the thread will clean up after itself
    pthread_cancel(m_thread);
}

bool FileWatcher::ReadFile(String filePath, char** pBuf, size_t* pLen)
{
    std::ifstream fs;
    fs.open(filePath.c_str(), std::ios::in);

    if (!fs.is_open())
        return false;

    fs.seekg(0, std::ios::end);
    *pLen = static_cast<size_t>(fs.tellg());
    *pBuf = new char[*pLen];
    fs.seekg(0, std::ios::beg);
    fs.read(*pBuf, *pLen);
    fs.close();
    return true;
}

} // namespace Zephyros
