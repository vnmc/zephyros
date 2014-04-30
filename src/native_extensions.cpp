//
// Copyright (C) 2013-2014 Vanamco AG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#include "app.h"

#ifndef USE_WEBVIEW
#include "extension_handler.h"
#else
#include "webview_extension.h"
#endif

#include "native_extensions.h"

#include "file_util.h"
#include "network_util.h"

#ifdef OS_WIN
#include <minmax.h>
#endif


//////////////////////////////////////////////////////////////////////
// Native Extensions

//
//
// function implementation
//
// there are the following "built-in" variables:
//
// - handler: a CefRefPtr to the client handler
// - browser: a CefRefPtr to the browser
// - state:   a CefRefPtr to the extension state, an object in which stateful
//            information and objects can be put
// - args:    the arguments passed to the function, a CefRefPtr<CefListValue>
// - ret:     the arguments that will be passed to the callback function (if any)
//
void AddNativeExtensions(NativeJavaScriptFunctionAdder* e)
{
    //////////////////////////////////////////////////////////////////////
    // Events

	// void onMenuCommand(function(string commandId))
	e->AddNativeJavaScriptCallback(
		TEXT("onMenuCommand"),
		FUNC({
			// only register callback
			return NO_ERROR;
		}
	));

    // void onAppTerminating(function())
    NativeFunction* fnxOnAppTerminating = FUNC({ return NO_ERROR; });
    fnxOnAppTerminating->SetAllCallbacksCompletedHandler(PROC({ App::QuitMessageLoop(); }));
    e->AddNativeJavaScriptCallback(TEXT("onAppTerminating"), fnxOnAppTerminating);

    
    //////////////////////////////////////////////////////////////////////
    // My Own Native Functions
    
    // In this example, we want to create the function (in C code):
    //     int myFunction(int firstNumber, int secondNumber) {
    //         return firstNumber + secondNumber;
    //     }
    //
    // The function "myFunction" will be available in your app's
    // JavaScript in the "app" object
    // (i.e., call app.myFunction(arg1, ..., argN, callback);
    // the callback function will be called once the result is ready from the
    // native layer)
    //
    e->AddNativeJavaScriptFunction(
        // function name
        TEXT("myFunction"),
                                   
        FUNC({
            // your native code here
        
            // you can access the arguments to myFunction using the
            // 'built-in' "args" array variable and set return values using
            // "ret" (which are in fact arguments passed to the callback function)
        
            int arg0 = args->GetInt(0);
            int arg1 = args->GetInt(1);
        
            // myFunction computes arg0 + arg1
            ret->SetInt(0, arg0 + arg1);
        
            // everything went OK
            return NO_ERROR;
        },
        
        // declare the argument types and names
        ARG(VTYPE_INT, "firstNumber")
        ARG(VTYPE_INT, "secondNumber")
    ));


    //////////////////////////////////////////////////////////////////////
    // Some useful functions: File System

    // void showOpenFileDialog(function(string path))
    e->AddNativeJavaScriptFunction(
        TEXT("showOpenFileDialog"),
#ifndef USE_WEBVIEW
        FUNC({
            String file = FileUtil::ShowOpenFileDialog();
			if (file.length() > 0)
                ret->SetString(0, file);
            else
                ret->SetNull(0);
            return NO_ERROR;
		})
#else
		FUNC({
            FileUtil::ShowOpenFileDialog(callback);
            return RET_DELAYED_CALLBACK;
		})
#endif
    );

    // void showOpenDirectoryDialog(function(string path))
    e->AddNativeJavaScriptFunction(
        TEXT("showOpenDirectoryDialog"),
#ifndef USE_WEBVIEW
        FUNC({
            String path = FileUtil::ShowOpenDirectoryDialog();
            if (path.length() > 0)
                ret->SetString(0, path);
            else
                ret->SetNull(0);
            return NO_ERROR;
		})
#else
		FUNC({
			FileUtil::ShowOpenDirectoryDialog(callback);
			return RET_DELAYED_CALLBACK;
        })
#endif
    );

    // void showInFileManager(string path)
    e->AddNativeJavaScriptProcedure(
        TEXT("showInFileManager"),
        FUNC({
            FileUtil::ShowInFileManager(args->GetString(0));
            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "path")
    ));

	// void readFile(string path, json<readFileOptions> options, function(string contents))
    e->AddNativeJavaScriptFunction(
        TEXT("readFile"),
        FUNC({
            String result;
            if (FileUtil::ReadFile(args->GetString(0), args->GetDictionary(1), result))
                ret->SetString(0, result);
            else
                ret->SetNull(0);

            return NO_ERROR;
        },
        ARG(VTYPE_STRING, "path")
        ARG(VTYPE_DICTIONARY, "options")
    ));
    

    //////////////////////////////////////////////////////////////////////
    // Networking
    
#ifdef USE_WEBVIEW
    // mimicks the Zepto ajax function
    // Syntax:
    // app.ajax(options),
    // options = {
    //     type: {String, opt}, HTTP method, e.g., "GET" or "POST"; default: "GET"
    //     url: {String}
    //     data: {String, opt}, POST data
    //     contentType: {String, opt}, the content type of "data"; default: "application/x-www-form-urlencoded" if "data" is set
    //     dataType: {String, opt}, response type to expect from the server ("json", "xml", "html", "text", "base64")
    //     success: {Function(data, contentType)}, callback called when request succeeds
    //     error: {Function()}, callback called if there is an error (timeout, parse error, or status code not in HTTP 2xx)
    // }
    e->AddNativeJavaScriptFunction(
        TEXT("ajax"),
        FUNC({
            JavaScript::Object options = args->GetDictionary(0);
        
            String httpMethod = options->GetString(TEXT("type"));
            if (httpMethod == "")
                httpMethod = TEXT("GET");
        
            String url = options->GetString(TEXT("url"));
        
            String postData = options->GetString(TEXT("data"));
            String postDataContentType = options->GetString(TEXT("contentType"));
            if (postData != "" && postDataContentType == "")
                postDataContentType = TEXT("application/x-www-form-urlencoded");
        
            String responseDataType = options->GetString(TEXT("dataType"));
            if (responseDataType == "")
                responseDataType = TEXT("text");
        
            NetworkUtil::MakeRequest(callback, httpMethod, url, postData, postDataContentType, responseDataType);
        
            return RET_DELAYED_CALLBACK;
        }
        ARG(VTYPE_DICTIONARY, "options")),
        true, false,
        TEXT("return ajax(options, function(data, contentType, status) { if (status) { if (options.success) options.success(options.dataType === 'json' ? JSON.parse(data) : data, contentType); } else if (options.error) options.error(); });")
    );
#endif
}


//////////////////////////////////////////////////////////////////////
// State Object for the Native Extensions

// Add any references to long lived objects used in the
// native extensions to this class.

ExtensionState::ExtensionState()
{
}
 
ExtensionState::~ExtensionState()
{
}

void ExtensionState::SetClientExtensionHandler(ClientExtensionHandlerPtr e)
{
    m_e = e;
}
