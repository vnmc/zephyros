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


#include <map>

#include "base/app.h"
#include "base/types.h"

#ifdef USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#endif

#include "native_extensions/browser.h"
#include "native_extensions/custom_url_manager.h"
#include "native_extensions/file_util.h"
#include "native_extensions/file_watcher.h"
#include "native_extensions/network_util.h"
#include "native_extensions/os_util.h"
#include "native_extensions/pageimage.h"
#include "native_extensions/path.h"

#ifdef OS_MACOSX
#include "native_extensions/image_util_mac.h"
#endif

#ifndef APPSTORE
#include "native_extensions/updater.h"
#endif

#ifdef OS_WIN
#include <Windows.h>
#include <minmax.h>
#include <objidl.h>
#include <gdiplus.h>
#endif


namespace Zephyros {

NativeExtensions::NativeExtensions()
    : m_bIsNativeExtensionsAdded(false)
{
}

void NativeExtensions::SetClientExtensionHandler(ClientExtensionHandlerPtr e)
{
    m_e = e;
}


DefaultNativeExtensions::DefaultNativeExtensions()
    : m_pBrowsers(NULL)
{
    m_fileWatcher = new FileWatcher();
    m_customURLManager = new CustomURLManager();
}

DefaultNativeExtensions::~DefaultNativeExtensions()
{
    delete m_fileWatcher;
    delete m_customURLManager;

    if (m_pBrowsers)
        BrowserUtil::CleanUp(m_pBrowsers);
}

void DefaultNativeExtensions::SetClientExtensionHandler(ClientExtensionHandlerPtr e)
{
    NativeExtensions::SetClientExtensionHandler(e);
}

/**
 *
 * function implementation
 *
 * there are the following "built-in" variables:
 *
 * - handler: a CefRefPtr to the client handler
 * - browser: a CefRefPtr to the browser
 * - args:    the arguments passed to the function, a CefRefPtr<CefListValue>
 * - ret:     the arguments that will be passed to the callback function (if any)
 */
void DefaultNativeExtensions::AddNativeExtensions(NativeJavaScriptFunctionAdder* e)
{
    //////////////////////////////////////////////////////////////////////
    // Events

    // onAddURL: (callback: (URLs: string[]) => void) => void
    e->AddNativeJavaScriptCallback(
        TEXT("onAddURLs"),
        FUNC({
            // only register callback
            return NO_ERROR;
        }
    ));

    // onMenuCommand: (callback: (commandId: string) => void) => void
    e->AddNativeJavaScriptCallback(
        TEXT("onMenuCommand"),
        FUNC({
            // only register callback
            return NO_ERROR;
        }
    ));

    // onTouchBarAction: (callback: (id: string) => void) => void
    e->AddNativeJavaScriptCallback(
        TEXT("onTouchBarAction"),
        FUNC({
            // only register callback
            return NO_ERROR;
        }
    ));

    // onFileChanged: (callback: (paths: string[]) => void) => void
    e->AddNativeJavaScriptCallback(
        TEXT("onFileChanged"),
        FUNC({
            // only register callback
            return NO_ERROR;
        }
    ));

    // onCustomURL: (callback: (url: string) => void) => void
    e->AddNativeJavaScriptCallback(
        TEXT("onCustomURL"),
        FUNC({
            // only register callback
            return NO_ERROR;
        }
    ));

    // onAppTerminating(callback: () => void) => void
    NativeFunction* fnxOnAppTerminating = FUNC({ return NO_ERROR; });

#ifdef USE_CEF
    fnxOnAppTerminating->SetAllCallbacksCompletedHandler(CALLBACK_HANDLER({
        if (retVal)
            App::CloseWindow();
    }));
#endif

    e->AddNativeJavaScriptCallback(TEXT("onAppTerminating"), fnxOnAppTerminating);


    //////////////////////////////////////////////////////////////////////
    // App Life Cycle

    e->AddNativeJavaScriptProcedure(
        TEXT("quit"),
        FUNC({
            App::Quit();
            return NO_ERROR;
        }
    ));


    //////////////////////////////////////////////////////////////////////
    // UI

    // createMenu: (menu: IMenuItem[]) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("createMenu"),
        FUNC({
            OSUtil::CreateMenu(args->GetList(0));
            return NO_ERROR;
        },
        ARG(VTYPE_LIST, "menu")
    ));

    e->AddNativeJavaScriptProcedure(
        TEXT("removeMenuItem"),
        FUNC({
            OSUtil::RemoveMenuItem(args->GetString(0));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "commandId")
    ));

    // setMenuItemStatuses: (items: IMenuItemFlags) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("setMenuItemStatuses"),
        FUNC({
            JavaScript::Object items = args->GetDictionary(0);
            App::MenuItemStatuses statuses;

            JavaScript::KeyList keys;
            items->GetKeys(keys);
            for (JavaScript::KeyType commandId : keys)
                statuses[commandId] = items->GetInt(commandId);

            App::SetMenuItemStatuses(statuses);
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "items")
    ));

    // createContextMenu: (menuItems: IMenuItem[], callback: (menuHandle: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("createContextMenu"),
        FUNC({
            ret->SetString(0, std::to_string(OSUtil::CreateContextMenu(args->GetList(0))));
            return NO_ERROR;
        },
        ARG(VTYPE_LIST, "menuItems")
    ));

    // showContextMenu: (menuHandle: string, x: number, y: number, callback: (menuCommandId: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("showContextMenu"),
        FUNC({
            ret->SetString(0, OSUtil::ShowContextMenu(_wtoi64(String(args->GetString(0)).c_str()), (int) args->GetDouble(1), (int) args->GetDouble(2)));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "menuHandle")
        ARG(VTYPE_INT, "x")
        ARG(VTYPE_INT, "y")
    ));
    
    // createTouchBar: (touchBarItems: ITouchBarItem[]) => void;
    e->AddNativeJavaScriptProcedure(
        TEXT("createTouchBar"),
        FUNC({
            OSUtil::CreateTouchBar(args->GetList(0));
            return NO_ERROR;
        },
        ARG(VTYPE_LIST, "items")
    ));


    //////////////////////////////////////////////////////////////////////
    // OS Browsers

    // getBrowsers: (callback: (browsers: IBrowser[]) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getBrowsers"),
        FUNC({
            JavaScript::Array browsers = JavaScript::CreateArray();
            int i = 0;
            Zephyros::BrowserUtil::FindBrowsers(&((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers);
            for (Browser* pBrowser : *(((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers))
                browsers->SetDictionary(i++, pBrowser->CreateJSRepresentation());
            ret->SetList(0, browsers);
            return NO_ERROR;
        }
    ));

    // getDefaultBrowser: (callback: (browser: IBrowser) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getDefaultBrowser"),
        FUNC({
            Browser* defaultBrowser = Zephyros::BrowserUtil::GetDefaultBrowser(((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers);
            if (defaultBrowser != NULL)
                ret->SetDictionary(0, defaultBrowser->CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        }
    ));

    // getBrowserForUserAgent: (userAgent: IUserAgent, callback: (browser: IBrowser) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getBrowserForUserAgent"),
        FUNC({
            Browser* b = Zephyros::BrowserUtil::GetBrowserForUserAgent(((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers, args->GetDictionary(0));
            if (b != NULL)
                ret->SetDictionary(0, b->CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "userAgent")
    ));

    // openURL: (url: string) => void
    e->AddNativeJavaScriptFunction(
        TEXT("openURL"),
        FUNC({
            ret->SetBool(0, Zephyros::BrowserUtil::OpenURLInBrowser(args->GetString(0), NULL));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "url")
    ));

    // openURLWithBrowser: (url: string, browser: IBrowser) => void
    e->AddNativeJavaScriptFunction(
        TEXT("openURLWithBrowser"),
        FUNC({
            Browser* b = Zephyros::BrowserUtil::GetBrowserFromJSRepresentation(((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_pBrowsers, args->GetDictionary(1));
            if (b != NULL)
                ret->SetBool(0, Zephyros::BrowserUtil::OpenURLInBrowser(args->GetString(0), b));
            else
                ret->SetBool(0, false);
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "url")
        ARG(VTYPE_DICTIONARY, "browser")
    ));


    //////////////////////////////////////////////////////////////////////
    // File System

    // getComputerName: (callback: (computerName: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getComputerName"),
        FUNC({
            ret->SetString(0, OSUtil::GetComputerName());
            return NO_ERROR;
        }
    ));

    // getHomeDirectory: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getHomeDirectory"),
        FUNC({
            Path path(OSUtil::GetHomeDirectory());
            ret->SetDictionary(0, path.CreateJSRepresentation());
            return NO_ERROR;
        }
    ));

    // getTemporaryDirectory: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getTemporaryDirectory"),
        FUNC({
            Path path;
            FileUtil::GetTempDir(path);
            ret->SetDictionary(0, path.CreateJSRepresentation());
            return NO_ERROR;
        }
    ));

    // showSaveFileDialog: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("showSaveFileDialog"),
#ifdef OS_MACOSX
        FUNC({
            Zephyros::FileUtil::ShowSaveFileDialog(callback);
            return RET_DELAYED_CALLBACK;
        })
#else
        FUNC({
            Path path;
            if (FileUtil::ShowOpenFileDialog(path))
                ret->SetDictionary(0, path.CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        })
#endif
    );

    // showOpenFileDialog: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("showOpenFileDialog"),
#ifdef OS_MACOSX
        FUNC({
            Zephyros::FileUtil::ShowOpenFileDialog(callback);
            return RET_DELAYED_CALLBACK;
        })
#else
        FUNC({
            Path path;
            if (FileUtil::ShowOpenFileDialog(path))
                ret->SetDictionary(0, path.CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        })
#endif
    );

    // showOpenDirectoryDialog: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("showOpenDirectoryDialog"),
#ifdef OS_MACOSX
        FUNC({
            Zephyros::FileUtil::ShowOpenDirectoryDialog(callback);
            return RET_DELAYED_CALLBACK;
        })
#else
        FUNC({
            Path path;
            if (FileUtil::ShowOpenDirectoryDialog(path))
                ret->SetDictionary(0, path.CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        })
#endif
    );

    // showOpenFileOrDirectoryDialog: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("showOpenFileOrDirectoryDialog"),
#ifdef OS_MACOSX
        FUNC({
            Zephyros::FileUtil::ShowOpenFileOrDirectoryDialog(callback);
            return RET_DELAYED_CALLBACK;
        })
#else
        FUNC({
            Path path;
            if (FileUtil::ShowOpenFileOrDirectoryDialog(path))
                ret->SetDictionary(0, path.CreateJSRepresentation());
            else
                ret->SetNull(0);
            return NO_ERROR;
        })
#endif
    );

    // showInFileManager: (path: IPath) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("showInFileManager"),
        FUNC({
            JavaScript::Object path = args->GetDictionary(0);
            Zephyros::FileUtil::ShowInFileManager(path->GetString("path"));
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));

    // getDroppedURLs: (callback: (droppedURLs: IPath[]) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getDroppedURLs"),
        FUNC({
            JavaScript::Array urls = JavaScript::CreateArray();
            int i = 0;
            for (Path droppedURL : Zephyros::GetNativeExtensions()->GetDroppedURLs())
                urls->SetDictionary(i++, droppedURL.CreateJSRepresentation());
            ret->SetList(0, urls);
            Zephyros::GetNativeExtensions()->GetDroppedURLs().clear();
            return NO_ERROR;
        }
    ));

    // getCustomURLs: () => void
    e->AddNativeJavaScriptProcedure(
        TEXT("getCustomURLs"),
        FUNC({
            Zephyros::GetNativeExtensions()->GetCustomURLManager()->FireCustomURLs();
            return NO_ERROR;
        }
    ));

    // startAccessingPath: (path: IPath, callback: (err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("startAccessingPath"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error err;
        
            if (FileUtil::StartAccessingPath(path, err))
                ret->SetNull(0);
            else
                ret->SetDictionary(0, err.CreateJSRepresentation());

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));

    // stopAccessingPath: (path: IPath) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("stopAccessingPath"),
        FUNC({
            Path path(args->GetDictionary(0));
            FileUtil::StopAccessingPath(path);
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));

    // readFile: (path: IPath, options: IReadFileOptions, callback: (err: Error, contents: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("readFile"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error errStartAccessingPath;
        
            if (FileUtil::StartAccessingPath(path, errStartAccessingPath))
            {
                String result;
                Error errReadFile;
                
                if (FileUtil::ReadFile(path.GetPath(), args->GetDictionary(1), result, errReadFile))
                {
                    ret->SetNull(0);
                    ret->SetString(1, result);
                }
                else
                {
                    ret->SetDictionary(0, errReadFile.CreateJSRepresentation());
                    ret->SetNull(1);
                }

                FileUtil::StopAccessingPath(path);
            }
            else
            {
                ret->SetDictionary(0, errStartAccessingPath.CreateJSRepresentation());
                ret->SetNull(1);
            }

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
        ARG(VTYPE_DICTIONARY, "options")
    ));

    // writeFile: (path: IPath, contents: String, callback(err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("writeFile"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error errStartAccessingPath;

            if (FileUtil::StartAccessingPath(path, errStartAccessingPath))
            {
                Error errWriteFile;
                
                if (FileUtil::WriteFile(path.GetPath(), args->GetString(1), args->GetDictionary(2), errWriteFile))
                    ret->SetNull(0);
                else
                    ret->SetDictionary(0, errWriteFile.CreateJSRepresentation());

                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetDictionary(0, errStartAccessingPath.CreateJSRepresentation());

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
        ARG(VTYPE_STRING, "contents")
        ARG(VTYPE_DICTIONARY, "options")
    ));

    // existsFile: (path: IPath, callback(exists: boolean) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("existsFile"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error err;
        
            if (FileUtil::StartAccessingPath(path, err))
            {
                ret->SetBool(0, FileUtil::ExistsFile(path.GetPath()));
                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetBool(0, false);

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));

    // moveFile: (oldPath: IPath, newPath: IPath, callback: (err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("moveFile"),
        FUNC({
            Path oldPath(args->GetDictionary(0));
            Path newPath(args->GetDictionary(1));
            Error err;
        
            // TODO: macOS: sandboxing stuff; need access to the directory the file is moved to
            // e.g., cf. http://stackoverflow.com/questions/13950476/application-sandbox-renaming-a-file-doesnt-work
        
            if (FileUtil::MoveFile(oldPath.GetPath(), newPath.GetPath(), err))
                ret->SetNull(0);
            else
                ret->SetDictionary(0, err.CreateJSRepresentation());

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "oldPath")
        ARG(VTYPE_DICTIONARY, "newPath")
    ));
    
    // copyFile: (source: IPath, destination: IPath, callback: (err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("copyFile"),
        FUNC({
            Path source(args->GetDictionary(0));
            Path destination(args->GetDictionary(1));
            Error err;
        
            // TODO: macOS: sandboxing stuff; need access to the directory the file is moved to
            // e.g., cf. http://stackoverflow.com/questions/13950476/application-sandbox-renaming-a-file-doesnt-work
        
            if (FileUtil::CopyFile(source.GetPath(), destination.GetPath(), err))
                ret->SetNull(0);
            else
                ret->SetDictionary(0, err.CreateJSRepresentation());
        
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "source")
        ARG(VTYPE_DICTIONARY, "destination")
    ));

    // deleteFiles: (path: IPath, relativeFilenames: string, cb: (err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("deleteFiles"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error errStartAccessingPath;
        
            if (FileUtil::StartAccessingPath(path, errStartAccessingPath))
            {
                String strPath(path.GetPath());
                String strRelative = args->GetString(1);
                
                if (strRelative.size() > 0)
                {
                    if (strPath[strPath.length() - 1] != PATH_SEPARATOR)
                        strPath += PATH_SEPARATOR;
                    strPath += strRelative;
                }
                
                Error errDeleteFiles;
                                           
                if (FileUtil::DeleteFiles(strPath, errDeleteFiles))
                    ret->SetNull(0);
                else
                    ret->SetDictionary(0, errDeleteFiles.CreateJSRepresentation());

                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetDictionary(0, errStartAccessingPath.CreateJSRepresentation());
                                       
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
        ARG(VTYPE_STRING, "relativeFilenames")
    ));

    // isDirectory: (path: IPath, callback(isDir: boolean) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("isDirectory"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error err;

            if (FileUtil::StartAccessingPath(path, err))
            {
                ret->SetBool(0, path.IsDirectory());
                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetBool(0, false);

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));
    
    // stat: (path: IPath, callback(info: IStat) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("stat"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error err;

            if (FileUtil::StartAccessingPath(path, err))
            {
                FileUtil::StatInfo stat = { 0 };
                FileUtil::Stat(path.GetPath(), &stat);
                
                JavaScript::Object info = JavaScript::CreateObject();
                info->SetBool(TEXT("isFile"), stat.isFile);
                info->SetBool(TEXT("isDictionary"), stat.isDirectory);
                info->SetDouble(TEXT("fileSize"), stat.fileSize);
                info->SetDouble(TEXT("creationDate"), stat.creationDate);
                info->SetDouble(TEXT("modificationDate"), stat.modificationDate);
                
                ret->SetDictionary(0, info);

                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetNull(0);

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")),
        true, false,
        TEXT("return stat(path, function(info) { info.creationDate = new Date(info.creationDate); info.modificationDate = new Date(info.modificationDate); callback(info); });")
    );

    // makeDirectory: (path: IPath, callback(err: Error) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("makeDirectory"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error errStartAccessingPath;
        
            if (FileUtil::StartAccessingPath(path, errStartAccessingPath))
            {
                Error errMakeDirectory;

                if (FileUtil::MakeDirectory(path.GetPath(), true, errMakeDirectory))
                    ret->SetNull(0);
                else
                    ret->SetDictionary(0, errMakeDirectory.CreateJSRepresentation());

                FileUtil::StopAccessingPath(path);
            }
            else
                ret->SetDictionary(0, errStartAccessingPath.CreateJSRepresentation());

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));
    
    // readDirectory: (path: IPath, callback(err: Error, files: IPath[]) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("readDirectory"),
        FUNC({
            Path path(args->GetDictionary(0));
            Error errStartAccessingPath;
        
            if (FileUtil::StartAccessingPath(path, errStartAccessingPath))
            {
                std::vector<String> files;
                Error errReadDirectory;

                if (FileUtil::ReadDirectory(path.GetPath(), files, errReadDirectory))
                {
                    JavaScript::Array listFiles = JavaScript::CreateArray();
                    int i = 0;

                    for (String file : files)
                    {
                        Path p(file, path.GetURLWithSecurityAccessData(), path.HasSecurityAccessData());
                        listFiles->SetDictionary(i, p.CreateJSRepresentation());
                        ++i;
                    }

                    ret->SetNull(0);
                    ret->SetList(1, listFiles);
                }
                else
                {
                    ret->SetDictionary(0, errReadDirectory.CreateJSRepresentation());
                    ret->SetNull(1);
                }

                FileUtil::StopAccessingPath(path);
            }
            else
            {
                ret->SetDictionary(0, errStartAccessingPath.CreateJSRepresentation());
                ret->SetNull(1);
            }

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
    ));


    // startWatchingFiles: (path: string, fileExtensions: string[]) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("startWatchingFiles"),
        FUNC({
            std::vector<String> fileExtensions;
            JavaScript::Array listFileExtensions = args->GetList(1);
            for (size_t i = 0; i < listFileExtensions->GetSize(); ++i)
                fileExtensions.push_back(listFileExtensions->GetString((int) i));
            Path path(args->GetDictionary(0));
            ((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_fileWatcher->Start(path, fileExtensions);
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "path")
        ARG(VTYPE_LIST, "fileExtensions")
    ));

    // stopWatchingFiles: () => void
    e->AddNativeJavaScriptProcedure(
        TEXT("stopWatchingFiles"),
        FUNC({
            ((DefaultNativeExtensions*) Zephyros::GetNativeExtensions())->m_fileWatcher->Stop();
            return NO_ERROR;
        }
    ));

    // getApplicationResourcesDirectory: (callback: (path: IPath) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getApplicationResourcesDirectory"),
        FUNC({
            Path path;
            FileUtil::GetApplicationResourcesPath(path);
            ret->SetDictionary(0, path.CreateJSRepresentation());
            return NO_ERROR;
        }
    ));

    // startProcess: (executablePath: string, arguments: string[], cwd: string, callback: (exitCode: number, output: IOutputStreamData[]) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("startProcess"),
        FUNC({
            std::vector<String> arguments;
            JavaScript::Array listArgs = args->GetList(1);
            for (size_t i = 0; i < listArgs->GetSize(); ++i)
                arguments.push_back(listArgs->GetString((int) i));
            OSUtil::StartProcess(callback, args->GetString(0), arguments, args->GetString(2));
            return RET_DELAYED_CALLBACK;
        }
        ARG(VTYPE_STRING, "executablePath")
        ARG(VTYPE_LIST, "arguments")
        ARG(VTYPE_STRING, "cwd")
    ));


    //////////////////////////////////////////////////////////////////////
    // Preferences

    // loadPreferences: (callback: (key: string, data: any) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("loadPreferences"),
        FUNC({
            String data;
            FileUtil::LoadPreferences(args->GetString(0), data);
            ret->SetString(0, data);
            return NO_ERROR;
        }
        ARG(VTYPE_STRING, "key")),
        true, false,
        TEXT("return loadPreferences(key, function(datastr) { callback(datastr === '' ? {} : JSON.parse(datastr)); });")
    );

    // savePreferences: (key: string, data: any) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("storePreferences"),
        FUNC({
            FileUtil::StorePreferences(args->GetString(0), args->GetString(1));
            return NO_ERROR;
        }
        ARG(VTYPE_STRING, "key")
        ARG(VTYPE_STRING, "data")), // the native function ultimately receives a string
        TEXT("return storePreferences(key, JSON.stringify(data));")
    );

#ifdef APPSTORE
    e->AddNativeJavaScriptFunction(TEXT("getUpdaterSettings"), FUNC({ return NO_ERROR; }));
    e->AddNativeJavaScriptProcedure(TEXT("setUpdaterSettings"), FUNC({ return NO_ERROR; }, ARG(VTYPE_DICTIONARY, "updaterSettings")));
#else
    // getUpdaterSettings: (callback: (settings: IUpdaterSettings) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getUpdaterSettings"),
        FUNC({
            ret->SetDictionary(0, UpdaterUtil::GetSettings());
            return NO_ERROR;
        })
    );

    // setUpdaterSettings: (settings: IUpdaterSettings) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("setUpdaterSettings"),
        FUNC({
            UpdaterUtil::SetSettings(args->GetDictionary(0));
            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "updaterSettings")
    ));
#endif


    //////////////////////////////////////////////////////////////////////
    // Networking

    // getNetworkIPs: (callback: (ips: string[]) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getNetworkIPs"),
        FUNC({
            JavaScript::Array ips = JavaScript::CreateArray();
            int idx = 0;
        
            for (String ip : NetworkUtil::GetNetworkIPs())
                ips->SetString(idx++, ip);
            
            ret->SetList(0, ips);
        
            return NO_ERROR;
        }
    ));

    // getMACAddress: (callback: (macAddr: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getMACAddress"),
        FUNC({
            ret->SetString(0, NetworkUtil::GetPrimaryMACAddress());
            return NO_ERROR;
        }
    ));

    // getProxyForURL: (url: string, callback: (proxyConfig: IProxyConfig) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("getProxyForURL"),
        FUNC({
            String proxyType;
            String host;
            String username;
            String password;
            int port;

            NetworkUtil::GetProxyForURL(args->GetString(0), proxyType, host, port, username, password);

            JavaScript::Object proxyConfig = JavaScript::CreateObject();
            proxyConfig->SetString(TEXT("type"), proxyType == TEXT("") ? TEXT("none") : proxyType);
            proxyConfig->SetString(TEXT("host"), host);
            proxyConfig->SetInt(TEXT("port"), port);
            proxyConfig->SetString(TEXT("username"), username);
            proxyConfig->SetString(TEXT("password"), password);

            ret->SetDictionary(0, proxyConfig);
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "url")
    ));

#ifdef USE_WEBVIEW
    // mimicks the Zepto ajax function
    // Syntax:
    // app.ajax(options),
    // options = {
    //     type: {String, opt}, HTTP method, e.g., "GET" or "POST"; default: "GET"
    //     url: {String}
    //     data: {String, opt}, POST data
    //     contentType: {String, opt}, the content type of "data"; default: "application/x-www-form-urlencoded" if "data" is set
    //     dataType: {String, opt}, response type to expect from the server ("json", "xml", "html", "text", "base64")
    //     success: {Function(data, contentType)}, callback called when request succeeds
    //     error: {Function()}, callback called if there is an error (timeout, parse error, or status code not in HTTP 2xx)
    // }
    e->AddNativeJavaScriptFunction(
        TEXT("ajax"),
        FUNC({
            JavaScript::Object options = args->GetDictionary(0);

            String httpMethod = options->GetString(TEXT("type"));
            if (httpMethod == "")
                httpMethod = TEXT("GET");

            String url = options->GetString(TEXT("url"));

            String postData = options->GetString(TEXT("data"));
            String postDataContentType = options->GetString(TEXT("contentType"));
            if (postData != "" && postDataContentType == "")
                postDataContentType = TEXT("application/x-www-form-urlencoded");

            String responseDataType = options->GetString(TEXT("dataType"));
            if (responseDataType == "")
                responseDataType = TEXT("text");

            NetworkUtil::MakeRequest(callback, httpMethod, url, postData, postDataContentType, responseDataType);

            return RET_DELAYED_CALLBACK;
        }
        ARG(VTYPE_DICTIONARY, "options")),
        true, false,
        TEXT("return ajax(options, function(data, contentType, status) { if (status) { if (options.success) options.success(options.dataType === 'json' ? JSON.parse(data) : data, contentType); } else if (options.error) options.error(); });")
    );
#endif

#ifdef OS_MACOSX
    // convertImage: (base64ImageData: string, callback: (base64PNG: string) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("convertImage"),
        FUNC({
            ret->SetString(0, Zephyros::ImageUtil::ConvertImageToBase64EncodedPNG(args->GetString(0)));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "imageData")
    ));
#endif

    e->AddNativeJavaScriptFunction(
        TEXT("getPageImageForURL"),
        FUNC({
            PageImage::GetPageImageForURL(callback, args->GetString(0), args->GetInt(1));
            return RET_DELAYED_CALLBACK;
        },
        ARG(VTYPE_STRING, "url")
        ARG(VTYPE_INT, "width")
    ));


    //////////////////////////////////////////////////////////////////////
    // Windows, Notifications

#ifdef OS_WIN
#   define SET_WINDOW_SIZE_RETVAL NO_ERROR
#else
#   define SET_WINDOW_SIZE_RETVAL RET_DELAYED_CALLBACK
#endif
    // setWindowSize: (size: ISize, callback?: (newSize: ISize) => void) => void
    e->AddNativeJavaScriptFunction(
        TEXT("setWindowSize"),
        FUNC({
            int width = 0;
            int height = 0;
            bool hasWidth = false;
            bool hasHeight = false;

            JavaScript::Object size = args->GetDictionary(0);

            if (size->HasKey(TEXT("width")))
            {
                width = size->GetInt(TEXT("width"));
                hasWidth = true;
            }

            if (size->HasKey(TEXT("height")))
            {
                height = size->GetInt(TEXT("height"));
                hasHeight = true;
            }

            int newWidth = 0;
            int newHeight = 0;
            OSUtil::SetWindowSize(callback, width, height, hasWidth, hasHeight, &newWidth, &newHeight);

            JavaScript::Object newSize = JavaScript::CreateObject();
            newSize->SetInt(TEXT("width"), newWidth);
            newSize->SetInt(TEXT("height"), newHeight);
            ret->SetDictionary(0, newSize);

            return SET_WINDOW_SIZE_RETVAL;
        },
        ARG(VTYPE_DICTIONARY, "size")
    ));

    // setMinimumWindowSize: (size: ISize) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("setMinimumWindowSize"),
        FUNC({
            int width = 0;
            int height = 0;

            JavaScript::Object size = args->GetDictionary(0);

            if (size->HasKey(TEXT("width")))
                width = size->GetInt(TEXT("width"));
            if (size->HasKey(TEXT("height")))
                height = size->GetInt(TEXT("height"));

            OSUtil::SetMinimumWindowSize(width, height);

            return NO_ERROR;
        },
        ARG(VTYPE_DICTIONARY, "size")
    ));

    // displayNotification: (title: string, details: string) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("displayNotification"),
        FUNC({
            OSUtil::DisplayNotification(args->GetString(0), args->GetString(1));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "title")
        ARG(VTYPE_STRING, "details")
    ));

    // requestUserAttention: () => void;
    e->AddNativeJavaScriptProcedure(
        TEXT("requestUserAttention"),
        FUNC({
            OSUtil::RequestUserAttention();
            return NO_ERROR;
        }
    ));

    // copyToClipboard: (text: string) => void
    e->AddNativeJavaScriptProcedure(
        TEXT("copyToClipboard"),
        FUNC({
            OSUtil::CopyToClipboard(args->GetString(0));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "text")
    ));
    
    // beginDragFile: (path: IPath, x: number, y: number) => void
    e->AddNativeJavaScriptFunction(
        TEXT("beginDragFile"),
#ifdef OS_MACOSX
        FUNC({
            Path path(args->GetDictionary(0));
            OSUtil::BeginDragFile(callback, path, args->GetInt(1), args->GetInt(2));
            return RET_DELAYED_CALLBACK;
        },
		ARG(VTYPE_DICTIONARY, "path")
		ARG(VTYPE_INT, "x")
		ARG(VTYPE_INT, "y"))
#else
		FUNC({
			Path path(args->GetDictionary(0));
			int result = 0;

			ret->SetBool(0, OSUtil::BeginDragFile(path, args->GetInt(1), args->GetInt(2), result));
			ret->SetInt(1, result);

			return NO_ERROR;
		}, 
        ARG(VTYPE_DICTIONARY, "path")
        ARG(VTYPE_INT, "x")
        ARG(VTYPE_INT, "y"))
#endif
	);


    //////////////////////////////////////////////////////////////////////
    // Licensing

#ifdef APPSTORE
    e->AddNativeJavaScriptFunction(TEXT("getLicenseData"), FUNC({ return NO_ERROR; }));
    e->AddNativeJavaScriptProcedure(TEXT("deactivateLicense"), FUNC({ return NO_ERROR; }));
#else

    // getLicenseData: (callback: (data: ILicenseData) => void) => void;
    typedef std::map<String, String> StringMap;
    e->AddNativeJavaScriptFunction(
        TEXT("getLicenseData"),
        FUNC({
            AbstractLicenseManager* pMgr = Zephyros::GetLicenseManager();
            if (pMgr)
            {
                StringMap info(pMgr->GetLicenseInformation());
                JavaScript::Object obj = JavaScript::CreateObject();

                for (std::map<String, String>::iterator it = info.begin(); it != info.end(); ++it)
                    obj->SetString(it->first, it->second);

                ret->SetDictionary(0, obj);
            }
            else
                ret->SetNull(0);

            return NO_ERROR;
        }
    ));

    // deactivateLicense: () => void;
    e->AddNativeJavaScriptProcedure(
        TEXT("deactivateLicense"),
        FUNC({
            AbstractLicenseManager* pMgr = Zephyros::GetLicenseManager();
            if (pMgr)
                pMgr->Deactivate();
            return NO_ERROR;
        }
    ));
#endif
}

} // namespace Zephyros
