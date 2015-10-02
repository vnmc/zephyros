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


#ifndef NativeExtensions_h
#define NativeExtensions_h

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

#include "zephyros.h"
#include "jsbridge.h"


//////////////////////////////////////////////////////////////////////////
// Constants


#ifndef OS_WIN // NO_ERROR is defined on windows
static const int NO_ERROR                   = 0;
#endif

static const int ERR_UNKNOWN                = 1;
static const int ERR_INVALID_PARAM_NUM      = 2;
static const int ERR_INVALID_PARAM_TYPES    = 3;
static const int RET_DELAYED_CALLBACK       = -1;


#define END_MARKER -999


//////////////////////////////////////////////////////////////////////////
// Types

namespace Zephyros {
class ClientHandler;
class ClientExtensionHandler;
}

#ifdef USE_CEF

class CefBrowser;
class CefProcessMessage;

typedef int CallbackId;
typedef CefRefPtr<Zephyros::ClientExtensionHandler> ClientExtensionHandlerPtr;


// interface for process message delegates
class ProcessMessageDelegate : public virtual CefBase
{
public:
    // Called when a process message is received. Return true if the message was
    // handled and should not be passed on to other handlers.
    // ProcessMessageDelegates should check for unique message names to avoid
    // interfering with each other.
    virtual bool OnProcessMessageReceived(
        CefRefPtr<Zephyros::ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message);
        
    virtual void ReleaseCefObjects() {}
};

#endif


#ifdef USE_WEBVIEW

typedef Zephyros::ClientExtensionHandler* ClientExtensionHandlerPtr;
typedef JSObjectRef CallbackId;

#endif


#ifdef USE_CEF

#define FUNC(code, ...) new Zephyros::NativeFunction( \
    [](CefRefPtr<Zephyros::ClientHandler> handler, CefRefPtr<CefBrowser> browser, CefRefPtr<CefListValue> args, CefRefPtr<CefListValue> ret, CallbackId callback) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) [](CefRefPtr<Zephyros::ClientHandler> handler, CefRefPtr<CefBrowser> browser) code

#endif


#ifdef USE_WEBVIEW

#define FUNC(code, ...) new Zephyros::NativeFunction( \
    [](Zephyros::JavaScript::Array args, Zephyros::JavaScript::Array ret, CallbackId callback) -> int \
    code __VA_ARGS__, END_MARKER)

#define PROC(code) []() code

#endif


#define ARG(type, name) ,type,TEXT(name)


namespace Zephyros {

#ifdef USE_CEF

typedef int (*Function)(
    CefRefPtr<ClientHandler> handler,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefListValue> args,
    CefRefPtr<CefListValue> ret,
	CallbackId callbackId
);

typedef void (*CallbacksCompleteHandler)(
    CefRefPtr<ClientHandler> handler,
    CefRefPtr<CefBrowser> browser
);

#endif

    
#ifdef USE_WEBVIEW

typedef int (*Function)(Zephyros::JavaScript::Array args, Zephyros::JavaScript::Array ret, JSObjectRef callback);
typedef void (*CallbacksCompleteHandler)();

#endif

} // namespace Zephyros


//////////////////////////////////////////////////////////////////////////
// Zephyros Classes

namespace Zephyros {

// Native Extensions

class ClientCallback;
class FileWatcher;
class CustomURLManager;
class Browser;
    
class Path
{
public:
    Path()
    : m_path(TEXT("")), m_isDirectory(false), m_urlWithSecurityAccessData(TEXT("")), m_hasSecurityAccessData(false)
    {
    }
        
    Path(String path);
        
    Path(String path, String urlWithSecurityAccessData, bool hasSecurityAccessData);
        
    Path(JavaScript::Object jsonObj);
        
    Path& operator=(const Path& path)
    {
        m_path = path.GetPath();
        m_isDirectory = path.IsDirectory();
        m_urlWithSecurityAccessData = path.GetURLWithSecurityAccessData();
        m_hasSecurityAccessData = path.HasSecurityAccessData();
        
        return *this;
    }
        
    inline String GetPath() const
    {
        return m_path;
    }
        
    inline bool IsDirectory() const
    {
        return m_isDirectory;
    }
        
    inline String GetURLWithSecurityAccessData() const
    {
        return m_urlWithSecurityAccessData;
    }
        
    inline bool HasSecurityAccessData() const
    {
        return m_hasSecurityAccessData;
    }
        
    JavaScript::Object CreateJSRepresentation();
    
private:
    String m_path;
    bool m_isDirectory;
    String m_urlWithSecurityAccessData;
    bool m_hasSecurityAccessData;
};


#ifdef USE_CEF

// ClientHandler implementation.
class ClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
                      public CefDragHandler,
                      public CefKeyboardHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler
{
public:

	typedef std::set<CefRefPtr<ProcessMessageDelegate> > ProcessMessageDelegateSet;


	ClientHandler();
	virtual ~ClientHandler();

	void ReleaseCefObjects();


	// CefClient methods
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) OVERRIDE;
	virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) OVERRIDE;

	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE;

	// CefDragHandler methods
	virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask) OVERRIDE;

	// CefKeyboardHandler methods
	virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut) OVERRIDE;
	virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event) OVERRIDE;

	// CefLifeSpanHandler methods
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefLoadHandler methods
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) OVERRIDE;

	// CefRequestHandler methods
	virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request) OVERRIDE;

	virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;

	void SetMainHwnd(ClientWindowHandle handle);
	ClientWindowHandle GetMainHwnd() const;

#if OS_WIN
	void SetAccelTable(HACCEL hAccelTable)
	{
		m_hAccelTable = hAccelTable;
	}
#endif

	CefRefPtr<ClientExtensionHandler> GetClientExtensionHandler();

	CefRefPtr<CefBrowser> GetBrowser() const;
	int GetBrowserId() const;

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	// Returns true if the main browser window is currently closing. Used in
	// combination with DoClose() and the OS close notification to properly handle
	// 'onbeforeunload' JavaScript events during window close.
	bool IsClosing() const;

	void ShowDevTools(CefRefPtr<CefBrowser> browser, const CefPoint& inspect_element_at);
	void CloseDevTools(CefRefPtr<CefBrowser> browser);

private:
	void InitializeMIMETypes();

private:
	// START THREAD SAFE MEMBERS
	// The following members are thread-safe because they're initialized during
	// object construction and not changed thereafter.

    // Registered delegates.
    ProcessMessageDelegateSet m_processMessageDelegates;

	CefRefPtr<ClientExtensionHandler> m_clientExtensionHandler;

	std::map<String, String> m_mimeTypes;

	// END THREAD SAFE MEMBERS


	// Lock used to protect members accessed on multiple threads. Make it mutable
	// so that it can be used from const methods.
	mutable base::Lock m_lock;


	// START LOCK PROTECTED MEMBERS
	// The following members are accessed on multiple threads and must be protected by |lock_|.

	// The child browser window.
	CefRefPtr<CefBrowser> m_browser;

	// The child browser id.
	int m_nBrowserId;

	// True if the main browser window is currently closing.
	bool m_bIsClosing;

	// END LOCK PROTECTED MEMBERS


	// START UI THREAD ACCESS ONLY MEMBERS
	// The following members will only be accessed on the CEF UI thread.

	// List of any popup browser windows.
	std::list<CefRefPtr<CefBrowser> > m_popupBrowsers;

	// The main frame window handle.
	ClientWindowHandle m_mainHwnd;

#if OS_WIN
	HACCEL m_hAccelTable;
#endif

	// Number of currently existing browser windows. The application will exit
	// when the number of windows reaches 0.
	static int m_nBrowserCount;

	// END UI THREAD ACCESS ONLY MEMBERS

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
};


class NativeFunction
{
public:
    NativeFunction(Function fnx, ...);
    ~NativeFunction();
    
    int Call(
        CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefListValue> args, CefRefPtr<CefListValue> ret, int messageId);
    void AddCallback(int messageId, CefBrowser* browser);
    String GetArgList();
    
    int GetNumArgs()
    {
        return (int) m_argNames.size();
    }
    
    void SetAllCallbacksCompletedHandler(CallbacksCompleteHandler fnxAllCallbacksCompleted)
    {
        m_fnxAllCallbacksCompleted = fnxAllCallbacksCompleted;
    }

private:
    // A function pointer to the native implementation
    Function m_fnx;
    
    std::vector<int> m_argTypes;
    std::vector<String> m_argNames;
    
public:
    String m_name;
    bool m_hasPersistentCallback;
    std::vector<ClientCallback*> m_callbacks;

    // Function to invoke when all JavaScript callbacks have completed
    CallbacksCompleteHandler m_fnxAllCallbacksCompleted;
};

#endif


#ifdef USE_WEBVIEW

class NativeFunction
{
public:
    NativeFunction(Function fnx, ...);
    ~NativeFunction();
    
    int Call(JavaScript::Array args);
    void AddCallback(JSObjectRef objCallback);
    
    int GetNumArgs()
    {
        return (int) m_argNames.size();
    }
    
    String GetArgName(int index)
    {
        return m_argNames.at(index);
    }
    
    void SetAllCallbacksCompletedHandler(CallbacksCompleteHandler fnxAllCallbacksCompleted)
    {
        m_fnxAllCallbacksCompleted = fnxAllCallbacksCompleted;
    }
    
    void SetParamTransform(JSObjectRef paramTransform);
    
private:
    // A function pointer to the native implementation
    Function m_fnx;
    
    // The argument transformation function (based on the custom JS implementation)
    JSObjectRef m_paramTransform;
    
    std::vector<int> m_argTypes;
    std::vector<String> m_argNames;
    
public:
    String m_name;
    bool m_hasPersistentCallback;
    std::vector<JSObjectRef> m_callbacks;
    
    // Function to invoke when all JavaScript callbacks have completed
    CallbacksCompleteHandler m_fnxAllCallbacksCompleted;
};

#endif


class NativeJavaScriptFunctionAdder
{
public:
    inline void AddNativeJavaScriptProcedure(String name, NativeFunction* fnx, String customJavaScriptImplementation = TEXT(""))
    {
        AddNativeJavaScriptFunction(name, fnx, false, false, customJavaScriptImplementation);
    }
    
    inline void AddNativeJavaScriptCallback(String name, NativeFunction* fnx, String customJavaScriptImplementation = TEXT(""))
    {
        AddNativeJavaScriptFunction(name, fnx, false, true, customJavaScriptImplementation);
    }
    
    virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT("")) = 0;

#ifdef USE_CEF
protected:
	String CreateArgList(NativeFunction* fnx, bool hasReturnValue, bool hasPersistentCallback)
	{
		String argList = fnx->GetArgList();
		if (hasReturnValue || hasPersistentCallback)
		{
			if (fnx->GetNumArgs() > 0)
				argList.append(TEXT(","));
			argList.append(TEXT("callback"));
		}

		return argList;
	}
#endif
};


#ifdef USE_CEF

class ClientExtensionHandler : public NativeJavaScriptFunctionAdder, public ProcessMessageDelegate
{
public:
    ClientExtensionHandler();
    virtual ~ClientExtensionHandler();
    
    virtual void ReleaseCefObjects() override;
    
	virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT("")) override;

	bool InvokeCallbacks(String functionName, CefRefPtr<CefListValue> args);
	bool InvokeCallback(CallbackId callbackId, CefRefPtr<CefListValue> args);
    
    
    // ProcessMessageDelegate Implementation
    
    virtual bool OnProcessMessageReceived(CefRefPtr<ClientHandler> handler, CefRefPtr<CefBrowser> browser,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;
    
private:
    std::map<String, NativeFunction*> m_mapFunctions;
	std::map<CallbackId, ClientCallback*> m_mapDelayedCallbacks;
    
    IMPLEMENT_REFCOUNTING(ClientExtensionHandler);
};

#endif


#ifdef USE_WEBVIEW

class ClientExtensionHandler : public NativeJavaScriptFunctionAdder
{
public:
    ClientExtensionHandler();
    virtual ~ClientExtensionHandler();
    
    virtual void AddNativeJavaScriptFunction(String name, NativeFunction* fnx, bool hasReturnValue = true, bool hasPersistentCallback = false, String customJavaScriptImplementation = TEXT(""));
    
    bool InvokeFunction(String functionName, Zephyros::JavaScript::Array args);
    bool InvokeCallbacks(String functionName, Zephyros::JavaScript::Array args);
    void ThrowJavaScriptException(String functionName, int retval);
    
private:
    std::map<String, NativeFunction*> m_mapFunctions;
};

#endif


class NativeExtensions
{
public:
    virtual ~NativeExtensions() {}
    
    virtual void AddNativeExtensions(NativeJavaScriptFunctionAdder* extensionHandler) = 0;
    virtual void SetClientExtensionHandler(ClientExtensionHandlerPtr e);
    virtual ClientExtensionHandlerPtr GetClientExtensionHandler() { return m_e; }
    
    inline std::vector<Zephyros::Path>& GetDroppedURLs() { return m_droppedURLs; }
    inline CustomURLManager* GetCustomURLManager() { return m_customURLManager; }

protected:
    ClientExtensionHandlerPtr m_e;
    std::vector<Zephyros::Path> m_droppedURLs;
    CustomURLManager* m_customURLManager;
};

class DefaultNativeExtensions : public NativeExtensions
{
public:
    DefaultNativeExtensions();
    virtual ~DefaultNativeExtensions();
    
    virtual void AddNativeExtensions(NativeJavaScriptFunctionAdder* extensionHandler);
    virtual void SetClientExtensionHandler(ClientExtensionHandlerPtr e);
    
public:
    Zephyros::FileWatcher* m_fileWatcher;
    std::vector<Zephyros::Browser*>* m_pBrowsers;
};

} // namespace Zephyros


#endif // NativeExtensions_h
