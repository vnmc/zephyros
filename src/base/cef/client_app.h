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
 *******************************************************************************/


#ifndef Zephyros_ClientApp_h
#define Zephyros_ClientApp_h
#pragma once


#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "lib/cef/include/cef_app.h"


namespace Zephyros {

class AppExtensionHandler;

class ClientApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler
{
public:

    // Interface for browser delegates. All BrowserDelegates must be returned via
    // CreateBrowserDelegates. Do not perform work in the BrowserDelegate
    // constructor. See CefBrowserProcessHandler for documentation.
    class BrowserDelegate : public virtual CefBase
    {
    public:
        virtual void OnContextInitialized(CefRefPtr<ClientApp> app) {}
        virtual void OnBeforeChildProcessLaunch(CefRefPtr<ClientApp> app, CefRefPtr<CefCommandLine> command_line) {}
        virtual void OnRenderProcessThreadCreated(CefRefPtr<ClientApp> app, CefRefPtr<CefListValue> extra_info) {}
    };

    typedef std::set<CefRefPtr<BrowserDelegate> > BrowserDelegateSet;

    // Interface for renderer delegates. All RenderDelegates must be returned via
    // CreateRenderDelegates. Do not perform work in the RenderDelegate
    // constructor. See CefRenderProcessHandler for documentation.
    class RenderDelegate : public virtual CefBase
    {
    public:
        virtual void OnRenderThreadCreated(CefRefPtr<ClientApp> app, CefRefPtr<CefListValue> extra_info) {}
        virtual void OnWebKitInitialized(CefRefPtr<ClientApp> app) {}
        virtual void OnBrowserCreated(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser) {}
        virtual void OnBrowserDestroyed(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser) {}

        virtual CefRefPtr<CefLoadHandler> GetLoadHandler(CefRefPtr<ClientApp> app)
        {
            return NULL;
        }

        virtual bool OnBeforeNavigation(
            CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
            CefRefPtr<CefRequest> request, cef_navigation_type_t navigation_type, bool is_redirect)
        {
            return false;
        }

        virtual void OnContextCreated(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {}
        virtual void OnContextReleased(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {}

        virtual void OnUncaughtException(
            CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
            CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
        {
        }

        virtual void OnFocusedNodeChanged(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node) {}

        // Called when a process message is received. Return true if the message was
        // handled and should not be passed on to other handlers. RenderDelegates
        // should check for unique message names to avoid interfering with each other.
        virtual bool OnProcessMessageReceived(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
        {
            return false;
        }
    };

    typedef std::set<CefRefPtr<RenderDelegate> > RenderDelegateSet;


    ClientApp();
    ~ClientApp();

private:
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE
    {
        return this;
    }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
    {
        return this;
    }

    virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) OVERRIDE;

    // CefBrowserProcessHandler methods
    virtual void OnContextInitialized() OVERRIDE;
    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) OVERRIDE;
    virtual void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;

    // CefRenderProcessHandler methods
    virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
    virtual void OnWebKitInitialized() OVERRIDE;
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
    virtual bool OnBeforeNavigation(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavigationType navigation_type, bool is_redirect) OVERRIDE;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    virtual void OnUncaughtException(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context,
        CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node) OVERRIDE;
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;


    // set of supported BrowserDelegates
    BrowserDelegateSet m_browserDelegates;

    // set of supported RenderDelegates
    RenderDelegateSet m_renderDelegates;

    CefRefPtr<AppExtensionHandler> m_pAppExtensionHandler;


    IMPLEMENT_REFCOUNTING(ClientApp);
};

} // namespace Zephyros


#endif // Zephyros_ClientApp_h
