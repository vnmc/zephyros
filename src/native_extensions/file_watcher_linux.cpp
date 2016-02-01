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
 * Florian Müller, Vanamco AG
 *******************************************************************************/


#include <vector>
#include <set>

#include "util/string_util.h"
#include "native_extensions/file_watcher.h"

#include <sys/inotify.h>
#include <unordered_map>
#include <pthread.h>

#include <iostream>
#include <fstream>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN ( 1024 * ( EVENT_SIZE + 16) )

using namespace std;

//////////////////////////////////////////////////////////////////////////
// FileWatcher Implementation

struct FileWatchThreadData {
    Zephyros::FileWatcher *watcher;
    Zephyros::Path *path;
    std::vector<string> *extensions;
};

bool isFileEmpty(string directory, string filename) {
    return false;
}


void recurseDir(const char *name, int level, std::unordered_map<int, String> *watchMap, int m_fd)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            int wd = inotify_add_watch(m_fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);
            watchMap->insert(pair<int,String>(wd, String(path)));
            recurseDir(path, level + 1, watchMap, m_fd);
        }
    } while (entry = readdir(dir));
    closedir(dir);
}

void processEvent(char * buffer, int len, Zephyros::FileWatcher * watcher, std::unordered_map<int, String> watchMap, std::vector<string> *extensions) {

   int i = 0;
   std::vector<string> changes;

   while (i < len) {

        struct inotify_event *event = (struct inotify_event *) &buffer[i];
        unordered_map<int,String >::const_iterator item = watchMap.find(event->wd);
        string basePath = string(item->second);
        string fileName = string(event->name);

        bool isInExtensions = false;
        for (std::vector<string>::iterator it = extensions->begin() ; it != extensions->end(); ++it)
        {
                string ext = string(*it);
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
            String s = basePath;
            s.append("/");
            s.append(fileName);
            changes.insert(changes.end(), s);
        }

        i += EVENT_SIZE + event->len;
    }

    if (changes.size() > 0)
        watcher->FireFileChanged(changes);
}


void CleanupThreadData(void *arg)
{
    FileWatchThreadData *data = (FileWatchThreadData *) arg;
    delete data->path;
    delete data->extensions;
    delete data;
}


void *startWatchingThread(void *arg)
{
    int length, i = 0;
    char buffer[BUF_LEN];

    struct FileWatchThreadData *data = (struct FileWatchThreadData *) arg;

    int m_fd = inotify_init();
    std::unordered_map<int, String> watchMap;


    int wd = inotify_add_watch(m_fd, data->path->GetPath().c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    watchMap.insert(pair<int,String>(wd, data->path->GetPath()));

    // Recirsively add all directories below root dir to the watch
    recurseDir(data->path->GetPath().c_str(), 0, &watchMap, m_fd);

    struct timespec time_to_wait;
    time_to_wait.tv_sec = 60;
    time_to_wait.tv_nsec = 0;

    fd_set descriptors;

    FD_ZERO( &descriptors );
    FD_SET(m_fd, &descriptors);
    int ret_val;


    bool doContinue = true;
    pthread_cleanup_push(CleanupThreadData, arg);
    while( doContinue ) {

        ret_val = pselect (m_fd + 1, &descriptors, NULL, NULL, &time_to_wait, NULL);

        if( ret_val < 0) {
            //cout << "ERROR: " << ret_val << endl;
        }
        else if (!ret_val) {
            //cout << "Timeout occured." << endl;
        }
        else if (FD_ISSET( m_fd, &descriptors)) {
            int len = read(m_fd, buffer, BUF_LEN);
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
    // Prepare thread data
    struct FileWatchThreadData *data = new FileWatchThreadData;
    data->watcher = this;
    data->path = new Path(path);
    data->extensions = new std::vector<String>();

    for (String ext : fileExtensions)
        data->extensions->push_back(ext);

    // Start thread
    pthread_create(&m_thread, NULL, startWatchingThread, data);

}

void FileWatcher::Stop()
{
    // Cancel thread. The thread will clean up after itself.
    pthread_cancel(m_thread);
}

bool FileWatcher::ReadFile(String filePath, char** pBuf, size_t* pLen)
{
    ifstream fs;

    fs.open(filePath.c_str(), ios::in);

    if (fs.is_open()) {
        fs.seekg(0, ios::end);
        *pLen = (size_t) fs.tellg();
        *pBuf = new char [*pLen];
        fs.seekg(0, ios::beg);
        fs.read(*pBuf, *pLen);
        fs.close();
        return true;
    }
    else {
        return false;
    }
}

} // namespace Zephyros
