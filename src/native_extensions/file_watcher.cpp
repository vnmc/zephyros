#ifdef USE_CEF
//#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
#endif

#ifdef USE_WEBVIEW
#include "base/webview/webview_extension.h"
#endif

#include "native_extensions/file_watcher.h"
#include "native_extensions/file_util.h"


namespace Zephyros {

void FileWatcher::FireFileChanged(std::vector<String>& files)
{
    JavaScript::Array listFiles = JavaScript::CreateArray();
    int i = 0;
    
    for (String file : files)
    {
        JavaScript::Object entry = JavaScript::CreateObject();
        
        entry->SetString(TEXT("path"), file);
        entry->SetInt(TEXT("action"), FileUtil::ExistsFile(file) ? 0 : 1);
        
        listFiles->SetDictionary(i++, entry);
    }

    JavaScript::Array args = JavaScript::CreateArray();
    args->SetList(0, listFiles);
    m_clientExtensionHandler->InvokeCallbacks(TEXT("onFileChanged"), args);
}

} // namespace Zephyros
