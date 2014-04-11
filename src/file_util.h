//
//  FileUtilMac.h
//  Zephyros
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
String ShowOpenFileDialog();
String ShowOpenDirectoryDialog();
#endif

void ShowInFileManager(String path);

bool ReadFile(String filename, JavaScript::Object options, String& result);

} // namespace FileUtil