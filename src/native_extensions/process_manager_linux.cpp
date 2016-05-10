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


#include <algorithm>

#include "base/app.h"
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"

#include "util/string_util.h"

#include "native_extensions/os_util.h"
#include "native_extensions/process_manager.h"


extern CefRefPtr<Zephyros::ClientHandler> g_handler;


//////////////////////////////////////////////////////////////////////////
// ProcessManager Implementation

namespace Zephyros {

ProcessManager::ProcessManager(CallbackId callbackId, String strExePath, std::vector<String> vecArgs, String strCWD)
  : m_callbackId(callbackId),
	m_strExePath(strExePath),
	m_vecArgs(vecArgs),
	m_strCWD(strCWD)
{
	if (m_strCWD == TEXT("~"))
		m_strCWD = OSUtil::GetHomeDirectory();
}

ProcessManager::~ProcessManager()
{
}

void ProcessManager::Start()
{
	m_data.stream.clear();

    // TODO: implement
}

void ProcessManager::FireCallback(bool bSuccess, int exitCode)
{
	JavaScript::Array args = JavaScript::CreateArray();

	if (bSuccess)
	{
		JavaScript::Array stream = JavaScript::CreateArray();
		int i = 0;
		for (StreamDataEntry e : m_data.stream)
		{
			JavaScript::Object streamEntry = JavaScript::CreateObject();
			streamEntry->SetInt(TEXT("fd"), e.type);
			streamEntry->SetString(TEXT("text"), e.text);
			stream->SetDictionary(i++, streamEntry);
		}

		args->SetInt(0, exitCode);
		args->SetList(1, stream);
	}
	else
		args->SetNull(0);

	g_handler->GetClientExtensionHandler()->InvokeCallback(m_callbackId, args);
}

} // namespace Zephyros
