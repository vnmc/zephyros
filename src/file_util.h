//
//  FileUtilMac.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#include "types.h"


namespace FileUtil {

#ifdef USE_WEBVIEW
void ShowOpenFileDialog(JSObjectRef callback);
void ShowOpenDirectoryDialog(JSObjectRef callback);
#else
bool ShowOpenFileDialog(String path);
bool ShowOpenDirectoryDialog(String path);
#endif

void ShowInFileManager(String path);

} // namespace FileUtil