// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

// This file is shared by cefclient and cef_unittests so don't include using
// a qualified path.

#include <string>

#include "lib/cef/include/cef_cookie.h"
#include "lib/cef/include/cef_process_message.h"
#include "lib/cef/include/cef_task.h"
#include "lib/cef/include/cef_v8.h"

#include "base/cef/client_app.h"
#include "base/cef/client_handler.h"
#include "base/cef/extension_handler.h"


namespace Zephyros {

ClientApp::ClientApp()
{
	m_pAppExtensionHandler = new AppExtensionHandler();
    m_renderDelegates.insert(m_pAppExtensionHandler.get());
}

ClientApp::~ClientApp()
{
	m_browserDelegates.clear();
	m_renderDelegates.clear();
}

void ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	// disable proxy resolution (we only want to contact the Ghostlab server from the UI)
	command_line->AppendSwitch(TEXT("no-proxy-server"));
}

void ClientApp::OnContextInitialized()
{
    for (CefRefPtr<BrowserDelegate> delegate : m_browserDelegates)
        delegate->OnContextInitialized(this);
}

void ClientApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
    for (CefRefPtr<BrowserDelegate> delegate : m_browserDelegates)
        delegate->OnBeforeChildProcessLaunch(this, command_line);
}

void ClientApp::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info)
{
    for (CefRefPtr<BrowserDelegate> delegate : m_browserDelegates)
        delegate->OnRenderProcessThreadCreated(this, extra_info);
}

void ClientApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnRenderThreadCreated(this, extra_info);
}

void ClientApp::OnWebKitInitialized()
{
	// generate the JavaScript code for the extension and register it
    Zephyros::GetNativeExtensions()->AddNativeExtensions(m_pAppExtensionHandler.get());
	CefRegisterExtension("v8/app", m_pAppExtensionHandler->GetJavaScriptCode(), m_pAppExtensionHandler.get());

    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnWebKitInitialized(this);
}

void ClientApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnBrowserCreated(this, browser);
}

void ClientApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnBrowserDestroyed(this, browser);
}

CefRefPtr<CefLoadHandler> ClientApp::GetLoadHandler()
{
	CefRefPtr<CefLoadHandler> loadHandler;
	for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
	{
		loadHandler = delegate->GetLoadHandler(this);
		if (loadHandler.get())
			return loadHandler;
	}

	return NULL;
}

bool ClientApp::OnBeforeNavigation(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
    NavigationType navigation_type, bool is_redirect)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        if (delegate->OnBeforeNavigation(this, browser, frame, request, navigation_type, is_redirect))
            return true;

    return false;
}

void ClientApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnContextCreated(this, browser, frame, context);
}

void ClientApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnContextReleased(this, browser, frame, context);
}

void ClientApp::OnUncaughtException(
	CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context,
	CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
{
	for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
		delegate->OnUncaughtException(this, browser, frame, context, exception, stackTrace);
}

void ClientApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node)
{
    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        delegate->OnFocusedNodeChanged(this, browser, frame, node);
}

bool ClientApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	DCHECK_EQ(source_process, PID_BROWSER);

    for (CefRefPtr<RenderDelegate> delegate : m_renderDelegates)
        if (delegate->OnProcessMessageReceived(this, browser, source_process, message))
            return true;
    
    return false;
}

} // namespace Zephyros
