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


#include "native_extensions/file_util.h"

#include "util/string_util.h"
#include "native_extensions/image_util_linux.h"
#include "native_extensions/os_util.h"
#include <sys/stat.h>
#include <glob.h>
#include <fstream>
#include <sstream>
#include "base/app.h"
#include <pwd.h>
#include <sstream>;

#include "zephyros_strings.h"


#include <gtk/gtk.h>

using namespace std;


/*
 * Check if a file is binary base on its mime type as reported by 'file'.
 * Stores the binary flag in result[0], the image flag in result[1],
 * and returns the mime type for further processing.
 */
String checkForBinaryAndImage(String filename, bool* result)
{
    String strCmd = "file -bi ";
    strCmd.append(filename);
    String guess = Zephyros::OSUtil::Exec(strCmd);

    String mimeType = String(guess.substr(0, guess.find(";")));
    size_t foundBinaryCharset = guess.find("charset=binary");
    if (foundBinaryCharset == string::npos)
        return mimeType;

    result[0] = true;

    size_t foundImageMimeType = guess.find("image/");
    if(foundImageMimeType == string::npos)
        return mimeType;

    result[1] = true;
    return mimeType;
}


// Get the filename for the preferences file, to use in Load/StorePreferences
string getPreferencesFile()
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
    GtkWidget *dialog;

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;
	bool retVal = false;

	dialog = gtk_file_chooser_dialog_new(Zephyros::GetString(ZS_DIALOG_OPEN_FILE).c_str(), NULL, action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        path = Path(filename);
        retVal = true;
        g_free(filename);
    }

    gtk_widget_destroy(dialog);

	return retVal;
}

bool ShowSaveFileDialog(Path& path)
{
	GtkWidget *dialog;

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	gint res;
	bool retVal = false;

	dialog = gtk_file_chooser_dialog_new(Zephyros::GetString(ZS_DIALOG_SAVE_FILE).c_str(), NULL, action, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        path = Path(filename);
        retVal = true;
        g_free(filename);
    }

    gtk_widget_destroy(dialog);

	return retVal;
}

bool ShowOpenDirectoryDialog(Path& path)
{
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	gint res;
	bool retVal = false;

	dialog = gtk_file_chooser_dialog_new(Zephyros::GetString(ZS_DIALOG_OPEN_FOLDER).c_str(), NULL, action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        path = Path(filename);
        retVal = true;
        g_free(filename);
    }

    gtk_widget_destroy(dialog);

	return retVal;
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
    strCommand.append(" ");
    strCommand.append(path);
    strCommand.append(" &");
    system(strCommand.c_str());
}

bool ExistsFile(String filename)
{
	struct stat buffer;
	int status;
	status = stat(filename.c_str(), &buffer);
	return status == 0;
}

bool IsDirectory(String path)
{
    struct stat buffer;
    stat(path.c_str(), &buffer);
    return S_ISDIR(buffer.st_mode);
}

bool MakeDirectory(String path, bool recursive)
{
	String strWhichCmd = TEXT("which mkdir");
    String strCommand = OSUtil::Exec(strWhichCmd);
    Trim(strCommand);
    if (recursive)
        strCommand.append(" -p");
    strCommand.append(" ");
    strCommand.append(path);

    int exitCode = system(strCommand.c_str());
    return exitCode == 0;
}

bool ReadFile(String filename, JavaScript::Object options, String& result)
{
    ifstream fs;
    bool fileFlags[2] = { 0 };
    String mimeType = checkForBinaryAndImage(filename, fileFlags);


    if (fileFlags[0])
    {
        if (fileFlags[1])
        {
            // TODO: convert .ico to .png
            // http://fossies.org/linux/xpaint/util/ico2png/create_icon.c ?
            fs.open(filename, ios::in);
            if (fs.is_open()) {
                fs.seekg(0, ios::end);
                long size = fs.tellg();
                char *contents = new char [size];
                fs.seekg(0, ios::beg);
                fs.read(contents, size);
                String imageData = ImageUtil::Base64Encode(contents, size);
                result = "data:"; result.append(mimeType); result.append(";base64,"); result.append(imageData);
                delete [] contents;
                fs.close();
            }
            return true;
        }
        else
        {
            fs.open(filename, ios::in);
            if (fs.is_open()) {
                fs.seekg(0, ios::end);
                long size = fs.tellg();
                char *contents = new char [size];
                fs.seekg(0, ios::beg);
                fs.read(contents, size);
                String imageData = ImageUtil::Base64Encode(contents, size);
                result = "data:"; result.append(mimeType); result.append(";base64,"); result.append(imageData);
                delete [] contents;
                fs.close();
            }
            return true;
        }
    }
	else
	{
        fs.open(filename, ios::in);
        if (fs.is_open()) {
            fs.seekg(0, ios::end);
            long size = fs.tellg();
            char *contents = new char [size];
            fs.seekg(0, ios::beg);
            fs.read(contents, size);
            result = String(contents);
            delete [] contents;
            fs.close();
            return true;
        }
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
}

bool DeleteFiles(String filenames)
{
    glob_t glob_result;

    glob(filenames.c_str(), GLOB_TILDE, NULL, &glob_result);

    // No match
    if (glob_result.gl_pathc == 0)
        return false;

    for (unsigned int i = 0; i < glob_result.gl_pathc; i++)
    {
        if(unlink(glob_result.gl_pathv[i]) != 0)
            return false;
    }
    // Had match, and successfully deleted all matching
    return true;
}


void LoadPreferences(String key, String& data)
{
    ifstream fs(getPreferencesFile());

    if (fs.is_open())
    {
        string line;
        while (getline(fs, line))
        {
            istringstream iss(line);
            size_t idx = line.find(key);
            if (idx == 0)
            {
                data = line.substr(key.size() + 1);
                break;
            }

        }
    }

}

void StorePreferences(String key, String data)
{
    string prefFile = getPreferencesFile();
    ifstream fsIn(prefFile);

    StringStream newContents;
    bool alreadyFound = false;

    if (fsIn.is_open())
    {
        string line;
        while (getline(fsIn, line))
        {
            istringstream iss(line);
            size_t idx = line.find(key + "=");
            if (idx == 0)
            {
                alreadyFound = true;
                newContents << key << "=" << data << endl;
            }
            else
                newContents << line << endl;
        }
        fsIn.close();
        if (!alreadyFound)
            newContents << key << "=" << data << endl;
    }
    else
        newContents << key << "=" << data << endl;

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

void GetApplicationResourcesPath(Path& path)
{
    // TODO: implement
}

} // namespace FileUtil
} // namespace Zephyros
