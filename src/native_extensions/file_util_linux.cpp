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


#include <fstream>
#include <sstream>

#include <glob.h>
#include <pwd.h>
#include <libgen.h>

#include <sys/stat.h>
#include <gtk/gtk.h>

#include "base/app.h"
#include "util/string_util.h"
#include "native_extensions/image_util_linux.h"
#include "native_extensions/os_util.h"
#include "native_extensions/file_util.h"
#include "zephyros_strings.h"


bool OpenFileDlg(GtkFileChooserAction action, int titleId, int okId, Zephyros::Path& path)
{
    bool retVal = false;

    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        Zephyros::GetString(titleId).c_str(),
        NULL,
        action,
        Zephyros::GetString(ZS_DIALOG_FILE_CANCEL_BUTTON).c_str(),
        GTK_RESPONSE_CANCEL,
        Zephyros::GetString(okId).c_str(),
        GTK_RESPONSE_ACCEPT,
        NULL
    );

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        path = Zephyros::Path(filename);
        retVal = true;
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
    return retVal;
}

/**
 * Check if a file is binary base on its mime type as reported by 'file'.
 * Stores the binary flag in result[0], the image flag in result[1],
 * and returns the mime type for further processing.
 */
String CheckForBinaryAndImage(String filename, bool& isBinary, bool& isImage)
{
    isBinary = false;
    isImage = false;

    String strCmd = "file -bi ";
    strCmd.append(filename);

    String guess = Zephyros::OSUtil::Exec(strCmd);
    String mimeType = guess.substr(0, guess.find(";"));

    size_t foundBinaryCharset = guess.find("charset=binary");
    if (foundBinaryCharset == String::npos)
        return mimeType;

    isBinary = true;
    size_t foundImageMimeType = guess.find("image/");
    if(foundImageMimeType == String::npos)
        return mimeType;

    isImage = true;
    return mimeType;
}

/**
 * Get the filename for the preferences file, to use in Load/StorePreferences.
 */
String GetPreferencesFile()
{
    String strFilename(Zephyros::OSUtil::GetConfigDirectory());
	strFilename.append(TEXT("/"));
	strFilename.append("preferences.ini");
    return strFilename;
}


namespace Zephyros {
namespace FileUtil {

bool ShowOpenFileDialog(Path& path)
{
    return OpenFileDlg(GTK_FILE_CHOOSER_ACTION_OPEN, ZS_DIALOG_OPEN_FILE, ZS_DIALOG_FILE_OPEN_BUTTON, path);
}

bool ShowSaveFileDialog(Path& path)
{
    return OpenFileDlg(GTK_FILE_CHOOSER_ACTION_SAVE, ZS_DIALOG_SAVE_FILE, ZS_DIALOG_FILE_SAVE_BUTTON, path);
}

bool ShowOpenDirectoryDialog(Path& path)
{
    return OpenFileDlg(GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ZS_DIALOG_OPEN_FOLDER, ZS_DIALOG_FILE_OPEN_BUTTON, path);
}

bool ShowOpenFileOrDirectoryDialog(Path& path)
{
    // TODO: not available in GTK? Seems only either file or folder is possible.
	return false;
}

void ShowInFileManager(String path)
{
	String strWhichCmd = TEXT("which xdg-open");
    String strCommand = OSUtil::Exec(strWhichCmd);
    Trim(strCommand);

    if (strCommand.length() > 0)
    {
        strCommand.append(" ");
        strCommand.append(path);
        strCommand.append(" &");
        system(strCommand.c_str());
    }
}

bool ExistsFile(String filename)
{
	struct stat buffer;
	return stat(filename.c_str(), &buffer) == 0;
}

bool IsDirectory(String path)
{
    struct stat buffer;
    stat(path.c_str(), &buffer);
    return S_ISDIR(buffer.st_mode);
}

// cf. http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
int MakeDirectoryInternal(const char* path, mode_t mode)
{
    struct stat st;
    int status = 0;

    if (stat(path, &st) != 0)
    {
        // directory does not exist. EEXIST for race condition
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return status;
}

bool MakeDirectory(String path, bool recursive)
{
    int status = 0;
    char* pathTmp = strdup(path.c_str());
    char* pp = pathTmp;
    char* sp;

    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            // neither root nor double slash in path
            *sp = '\0';
            status = MakeDirectoryInternal(pathTmp, 0777);
            *sp = '/';
        }

        pp = sp + 1;
    }

    if (status == 0)
        status = MakeDirectoryInternal(path.c_str(), 0777);

    free(pathTmp);

    return status == 0;
}

bool ReadDirectory(String path, std::vector<String>& files)
{
    if (path.find_first_of(TEXT("*?")) == String::npos)
    {
        if (path.back() == TEXT('/'))
            path.append(TEXT("*"));
        else
            path.append(TEXT("/*"));
    }

    glob_t glob_result;
    glob(path.c_str(), GLOB_TILDE, NULL, &glob_result);

    // no match
    if (glob_result.gl_pathc == 0)
        return true;

    for (unsigned int i = 0; i < glob_result.gl_pathc; i++)
        files.push_back(String(glob_result.gl_pathv[i]));

    return true;
}

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
    bool isBinary;
    bool isImage;
    String mimeType = CheckForBinaryAndImage(filename, isBinary, isImage);

    std::ifstream fs;
    fs.open(filename, std::ios::in);
    if (fs.is_open())
    {
        fs.seekg(0, std::ios::end);
        long size = fs.tellg();
        char* contents = new char[size];
        fs.seekg(0, std::ios::beg);
        fs.read(contents, size);

        if (isImage)
        {
            // TODO: convert .ico to .png
            // http://fossies.org/linux/xpaint/util/ico2png/create_icon.c ?
            result = "data:";
            result.append(mimeType);
            result.append(";base64,");
            result.append(ImageUtil::Base64Encode(contents, size));
        }
        else
            result = String(contents);

        delete[] contents;
        fs.close();
        return true;
    }

    return false;
}

bool WriteFile(String filename, String contents)
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);

    if (!file.is_open())
        return false;

    file << contents;
    file.close();

    return true;
}

bool MoveFile(String oldFilename, String newFilename)
{
    return rename(oldFilename.c_str(), newFilename.c_str()) != 0;
}

bool DeleteFiles(String filenames)
{
    glob_t glob_result;

    glob(filenames.c_str(), GLOB_TILDE, NULL, &glob_result);

    // no match
    if (glob_result.gl_pathc == 0)
        return false;

    for (unsigned int i = 0; i < glob_result.gl_pathc; i++)
        if (unlink(glob_result.gl_pathv[i]) != 0)
            return false;

    // there was a match, and all matching files were deleted successfully
    return true;
}

void LoadPreferences(String key, String& data)
{
    std::ifstream fs(GetPreferencesFile());
    if (!fs.is_open())
        return;

    String line;
    while (getline(fs, line))
    {
        size_t idx = line.find(key);
        if (idx == 0)
        {
            data = line.substr(key.size() + 1);
            break;
        }
    }
}

void StorePreferences(String key, String data)
{
    String prefFile = GetPreferencesFile();
    std::ifstream fsIn(prefFile);

    StringStream newContents;
    bool alreadyFound = false;

    if (fsIn.is_open())
    {
        String line;
        while (getline(fsIn, line))
        {
            size_t idx = line.find(key + "=");
            if (idx == 0)
            {
                alreadyFound = true;
                newContents << key << "=" << data << std::endl;
            }
            else
                newContents << line << std::endl;
        }

        fsIn.close();
        if (!alreadyFound)
            newContents << key << "=" << data << std::endl;
    }
    else
        newContents << key << "=" << data << std::endl;

    WriteFile(prefFile, newContents.str());
}

bool StartAccessingPath(Path& path)
{
	// not supported on Linux
	return true;
}

void StopAccessingPath(Path& path)
{
	// not supported on Linux
}

void GetTempDir(Path& path)
{
    path = Path(P_tmpdir);
}

String GetApplicationPath()
{
    char path[PATH_MAX];
    if (readlink("/proc/self/exe", path, PATH_MAX) != -1)
    {
        dirname(path);
        strcat(path, "/");
        return String(path);
    }

    return "";
}

void GetApplicationResourcesPath(Path& path)
{
	path = Path(FileUtil::GetApplicationPath() + TEXT("/res"));
}

} // namespace FileUtil
} // namespace Zephyros
