//
//  FileUtilMac.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#include "base/types.h"
#include "native_extensions/path.h"


namespace Zephyros {
namespace FileUtil {
    
#ifdef USE_CEF
bool ShowOpenFileDialog(Path& path);
bool ShowOpenDirectoryDialog(Path& path);
bool ShowOpenFileOrDirectoryDialog(Path& path);
#endif

#ifdef USE_WEBVIEW
void ShowOpenFileDialog(JSObjectRef callback);
void ShowOpenDirectoryDialog(JSObjectRef callback);
void ShowOpenFileOrDirectoryDialog(JSObjectRef callback);
#endif

void ShowInFileManager(String path);

bool ExistsFile(String filename);
bool IsDirectory(String path);

bool MakeDirectory(String path, bool recursive);

/**
 * Returns true if the path exists.
 * If path is a directory, it isn't modified; otherwise it is modified to the
 * directory containing the file path.
 */
bool GetDirectory(String& path);

bool ReadFile(String filename, JavaScript::Object options, String& result);

void LoadPreferences(String key, String& data);
void StorePreferences(String key, String data);

bool StartAccessingPath(Path& path);
void StopAccessingPath(Path& path);
    
void GetApplicationResourcesPath(Path& path);

} // namespace FileUtil
} // namespace Zephyros
