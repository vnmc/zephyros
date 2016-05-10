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
#include <map>

#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <spawn.h> // see manpages-posix-dev
#include <poll.h>
#include <sys/wait.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "base/app.h"
#include "base/types.h"

#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "native_extensions/os_util.h"

#include <iostream>


typedef struct
{
	int type;
	String text;
} StreamDataEntry;

typedef struct
{
    CallbackId callback;
    String executableFileName;
    std::vector<String>* arguments;
    String cwd;
} StartProcessThreadData;


extern CefRefPtr<Zephyros::ClientHandler> g_handler;

extern int g_nMinWindowWidth;
extern int g_nMinWindowHeight;

extern GtkWidget* g_pMenuBar;


void CleanupStartProcessThreadData(void* arg)
{
    StartProcessThreadData* data = (StartProcessThreadData*) arg;
    delete data->arguments;
    delete data;
}


// cf. http://stackoverflow.com/questions/13893085/posix-spawnp-and-piping-child-output-to-a-string
void* startProcessThread(void* arg)
{
    StartProcessThreadData* data = (StartProcessThreadData*) arg;
    int exit_code;
    int cout_pipe[2];
    int cerr_pipe[2];
    posix_spawn_file_actions_t action;

    pthread_cleanup_push(CleanupStartProcessThreadData, arg);

    if (pipe(cout_pipe) || pipe(cerr_pipe))
        std::cout << "pipe returned an error.\n";

    // prepare spawning of pipes
    posix_spawn_file_actions_init(&action);
    posix_spawn_file_actions_addclose(&action, cout_pipe[0]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[0]);
    posix_spawn_file_actions_adddup2(&action, cout_pipe[1], 1);
    posix_spawn_file_actions_adddup2(&action, cerr_pipe[1], 2);

    posix_spawn_file_actions_addclose(&action, cout_pipe[1]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[1]);

    // build spawn arguments
    char** args = new char*[data->arguments->size() + 2];
    args[0] = new char[data->executableFileName.length() + data->cwd.length() + 1];
    strcpy(args[0], (data->executableFileName).c_str());
    int i = 1;

    for (String arg : *(data->arguments))
    {
        args[i] = new char[arg.length() + 1];
        strcpy(args[i], arg.c_str());
        ++i;
    }
    args[i] = 0;

    // change to indicated working directory and spawn process
    if (data->cwd.length() > 0)
        chdir(data->cwd.c_str());

    pid_t pid;
    if (posix_spawnp(&pid, args[0], &action, NULL, &args[0], NULL) != 0)
        std::cout << "posix_spawnp failed with error: " << strerror(errno) << "\n";

    close(cout_pipe[1]), close(cerr_pipe[1]); // close child-side of pipes

    // Read process output until terminated, build JS callback arguments on the go
    i = 0;
    Zephyros::JavaScript::Array callbackArgs = Zephyros::JavaScript::CreateArray();
    Zephyros::JavaScript::Array stream = Zephyros::JavaScript::CreateArray();
    String buffer(1024, TEXT(' '));

    std::vector<pollfd> plist = {
        { cout_pipe[0], POLLIN },
        { cerr_pipe[0], POLLIN }
    };

    for (int rval; (rval = poll(&plist[0], plist.size(), -1 /* timeout */)) > 0; )
    {
        Zephyros::JavaScript::Object streamEntry = Zephyros::JavaScript::CreateObject();

        if (plist[0].revents & POLLIN)
        {
            int bytes_read = read(cout_pipe[0], &buffer[0], buffer.length());
            streamEntry->SetString(TEXT("text"), buffer.substr(0, static_cast<size_t>(bytes_read)));
            streamEntry->SetInt(TEXT("fd"), 0);
            stream->SetDictionary(i++, streamEntry);
        }
        else if (plist[1].revents & POLLIN)
        {
            int bytes_read = read(cerr_pipe[0], &buffer[0], buffer.length());
            streamEntry->SetString(TEXT("text"), buffer.substr(0, static_cast<size_t>(bytes_read)));
            streamEntry->SetInt(TEXT("fd"), 1);
            stream->SetDictionary(i++, streamEntry);
        }
        else
        {
            // nothing left to read
            break;
        }
    }

    // wait for process to terminate
    waitpid(pid, &exit_code, 0);

    // fire JavaScript callback
    callbackArgs->SetInt(0, exit_code);
    callbackArgs->SetList(1, stream);
    g_handler->GetClientExtensionHandler()->InvokeCallback(data->callback, callbackArgs);

    // clean up
    posix_spawn_file_actions_destroy(&action);
    pthread_cleanup_pop(arg);
}


namespace Zephyros {
namespace OSUtil {

String GetOSVersion()
{
    utsname info;
    uname(&info);

    StringStream ss;
    ss << info.sysname << TEXT("_") << info.version;
    String retVal = ss.str();
    size_t space = retVal.find(TEXT(" "));

    while (space != String::npos)
    {
        retVal = retVal.replace(space, 1, TEXT("_"));
        space = retVal.find(TEXT(" "));
    }

    return retVal;
}

String GetUserName()
{
    passwd pwd;
    passwd* result;

    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;

    char* buf = new char[bufsize];
    getpwuid_r(getuid(), &pwd, buf, bufsize, &result);

    String ret;
    if (result != NULL)
        ret = pwd.pw_name;

    delete[] buf;

	return ret;
}

String GetHomeDirectory()
{
    passwd pwd;
    passwd* result;

    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;

    TCHAR* buf = new TCHAR[bufsize];
    getpwuid_r(getuid(), &pwd, buf, bufsize, &result);

    String ret;
    if (result != NULL)
        ret = pwd.pw_dir;

    delete[] buf;

	return ret;
}

String GetConfigDirectory()
{
    StringStream ss;

    // construct a directory name of the form "~/.<normalized-app-name>"
    ss << OSUtil::GetHomeDirectory() << TEXT("/.");

    // normalize the app name
    const TCHAR* szAppName = Zephyros::GetAppName();
    size_t nLen = _tcslen(szAppName);
    for (size_t i = 0; i < nLen; ++i)
    {
        TCHAR c = szAppName[i];
        if ((TEXT('a') <= c && c <= TEXT('z')) || (TEXT('0') <= c && c <= TEXT('9')))
            ss << c;
        else if (TEXT('A') <= c && c <= TEXT('Z'))
            ss << (TCHAR) (c - TEXT('A') + TEXT('a'));
        else
            ss << TEXT('_');
    }

    String strConfigDir = ss.str();

    // make sure the directory exists
    DIR* dir = opendir(strConfigDir.c_str());
    if (!dir)
        mkdir(strConfigDir.c_str(), S_IRWXU | S_IXGRP | S_IXOTH);
    else
        closedir(dir);

    return strConfigDir;
}

String GetComputerName()
{
	return OSUtil::Exec(TEXT("hostname"));
}

void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
    StartProcessThreadData* data = new StartProcessThreadData;
    data->callback = callback;
    data->executableFileName = executableFileName;
    data->cwd = cwd;
    data->arguments = new std::vector<String>();

    for (String arg : arguments)
        data->arguments->push_back(arg);

    // start thread
    pthread_t thread;
    pthread_create(&thread, NULL, startProcessThread, data);
}

String Exec(String command)
{
    FILE* pipe = popen(command.c_str(), TEXT("r"));
    if (!pipe)
    	return TEXT("");

    char buffer[128];
    StringStream result;

    while (!feof(pipe))
    	if (fgets(buffer, 128, pipe) != NULL)
    		result << buffer;

    pclose(pipe);
    return result.str();
}

void CreateMenuRecursive(GtkWidget* pMenu, JavaScript::Array menuItems, bool bIsInDemoMode)
{
    bool bPrevItemWasSeparator = false;
    int nNumItems = (int) menuItems->GetSize();

	for (int i = 0; i < nNumItems; ++i)
	{
		JavaScript::Object item = menuItems->GetDictionary(i);

		GtkWidget* pMenuItem = NULL;

		String strCaption = item->GetString(TEXT("caption"));
		if (strCaption == TEXT("-"))
		{
            // this menu item is a separator

            if (bPrevItemWasSeparator)
                continue;

            pMenuItem = gtk_separator_menu_item_new();
            bPrevItemWasSeparator = true;
		}
		else
		{
            if (item->HasKey(TEXT("subMenuItems")))
            {
                pMenuItem = gtk_menu_item_new_with_mnemonic(strCaption.c_str());
                GtkWidget* pSubMenu = gtk_menu_new();
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pSubMenu);
                CreateMenuRecursive(pSubMenu, item->GetList(TEXT("subMenuItems")), bIsInDemoMode);
            }
            else
            {
                if (item->HasKey(TEXT("systemCommandId")))
                {
                }
                else if (item->HasKey(TEXT("menuCommandId")))
                {
                    String strCommandId = item->GetString(TEXT("menuCommandId"));

                    if (!bIsInDemoMode && (strCommandId == TEXT(MENUCOMMAND_ENTER_LICENSE) || strCommandId == TEXT(MENUCOMMAND_PURCHASE_LICENSE)))
                        continue;

                    pMenuItem = gtk_menu_item_new_with_mnemonic(strCaption.c_str());

                    // special command IDs

                    // TODO: implement handlers, icons, accels
                }
            }
		}

        gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
        bPrevItemWasSeparator = false;
    }
}

void CreateMenu(JavaScript::Array menuItems)
{
    // TODO: empty existing menu bar
    // Round 1
    CreateMenuRecursive(g_pMenuBar, menuItems, Zephyros::GetLicenseManager() != NULL && Zephyros::GetLicenseManager()->IsInDemoMode());
}

void RemoveMenuItem(String strCommandId)
{
   /* GList *children = gtk_container_get_children(GTK_CONTAINER(g_pMenuBar));
    GList *l;
    for (l = children; l != NULL; l = l->next)
    {
        std::cout << "Child " << l << std::endl;
    }*/
    // Round 1
}

MenuHandle CreateContextMenu(JavaScript::Array menuItems)
{
    // TODO: implement
    // Round 2
	return (MenuHandle) 0;
}

String ShowContextMenu(MenuHandle nMenuHandle, int x, int y)
{
    // TODO: implement
    // Round 2
    return TEXT("");
}


int lastOriginX = INT_MIN;
int lastOriginY = INT_MIN;
int lastX = -1;
int lastY = -1;
int lastWidth = -1;
int lastHeight = -1;

void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
    // TODO: fullscreen not handled yet
    GdkScreen *screen = gdk_screen_get_default();
    GtkWindow *window = GTK_WINDOW( gtk_widget_get_ancestor (App::GetMainHwnd(), GTK_TYPE_WINDOW) );
    gint screenWidth = gdk_screen_get_width(screen);
    gint screenHeight = gdk_screen_get_height(screen);
    gint posX;
    gint posY;
    gtk_window_get_position(window, &posX, &posY);

    gint newPosX = posX;
    gint newPosY = posY;

    bool windowMoved = posX != lastX || posY != lastY;

    if (hasWidth)
    {
        // do we need to restore the lastOriginX?
        if ( lastWidth > width  && lastOriginX != INT_MIN)
        {
            if (!windowMoved)
                newPosX = lastOriginX;
            lastOriginX = INT_MIN;
        }

        // adjust the windows x position if, after resizing, the window would fall off the screen
        if ( (newPosX + width) > screenWidth)
        {
            newPosX = screenWidth - width;
            if (newPosX < 0)
                newPosX = 0;
        }

        lastWidth = width;
    }
    else
        lastWidth = -1;

    if (hasHeight)
    {
        // do we need to restore the lastOriginY?
        if (lastHeight > height && lastOriginY != INT_MIN)
        {
            if (!windowMoved)
                newPosY = lastOriginY;
            lastOriginY = INT_MIN;
        }

        // adjust the windows y position if, after resizing, the window would fall off the screen
        if (newPosY + height > screenHeight)
        {
            newPosY = screenHeight - height;
            if (newPosY < 0)
                newPosY = 0;
        }

        lastHeight = height;
    }
    else
        lastHeight = -1;

    gtk_window_move(window, newPosX, newPosY);
    gtk_window_resize(window, width, height);
}


void SetMinimumWindowSize(int width, int height)
{
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_ancestor(App::GetMainHwnd(), GTK_TYPE_WINDOW));

    GdkGeometry hints;
    hints.min_width = width;
    hints.min_height = height;

    gtk_window_set_geometry_hints(window, NULL, &hints, GDK_HINT_MIN_SIZE);
}


void DisplayNotification(String title, String details)
{
    // TODO: implement
    // Round 2
}

void RequestUserAttention()
{
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_ancestor(App::GetMainHwnd(), GTK_TYPE_WINDOW));
    gtk_window_set_urgency_hint (window, true);
}

void CopyToClipboard(String text)
{
    GtkClipboard *cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, text.c_str(), -1);
}

void CleanUp()
{
}

} // namespace OSUtil
} // namespace Zephyros
