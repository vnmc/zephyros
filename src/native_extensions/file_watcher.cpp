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
 *******************************************************************************/


#ifdef USE_CEF
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"
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
