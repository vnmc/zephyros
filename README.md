Zephyros
========

**A Multi-platform HTML5 application wrapper.**

## Synopsis

Zephyros is a native application shell for HTML5 desktop apps.

Zephyros enables you to create your GUI in HTML5/CSS and write your application logic in JavaScript. For operating system functionality which can't be accessed normally by JavaScript, Zephyros provides an easily extensible native layer.

Currently, Mac and Windows are supported.
On Windows, the Chromium browser is used via the [Chromium Embedded Framework](https://code.google.com/p/chromiumembedded/) (CEF); on Mac the native WebView is used, which is based on Safari. (The Chromium Embedded Framework could also be used on Mac, but it wouldn't allow you to submit your App to the Mac AppStore.)

This project is very similar to Adobe's [brackets-shell](https://github.com/adobe/brackets-shell). The differences are that Zephyros uses WebView on Mac (for compatibility with the AppStore), and that Zephyros provides (in our opinion) a more easily extensible JavaScript extension ("native") layer.

## Project Structure

The project's root directory contains the project files, which can be opened in XCode and Visual Studio, respectively.

All the files for the HTML5 app are contained in the _app_ folder. When you clone the project, you'll find a simple example application in there, which you'll replace with your own app.
You may structure the contents of this folder as you like.

The source code for the native application shell and the native layer is contained in the _src_ folder.

The folder _lib_ contains the CEF DLL wrapper sources and DLLs.

### Using the JavaScript Native Extension

The native extension is exposed to the JavaScript through the ```app``` object.
For instance, the native layer provides a function ```showOpenFileDialog```, which will display an "open file" dialog. In your JavaScript, you can use it like so:

    app.showOpenFileDialog(function(path) { /* do something with path */ });
    
As in the example above, the functions in the native extension don't have any return values. Instead, the result(s) are passed to a callback function.

Currently, Zephyros only comes with a handful of exemplary native extension functions and events:

* ```app.showOpenFileDialog(function(path /*string*/) {})```
* ```app.showOpenDirectoryDialog(function(path /*string*/) {})```
* ```app.showInFileManager(path /*string*/)```
* ```app.readFile(path /*string*/, options /*object*/, function(fileContents /*string*/) {})```

* ```app.onMenuCommand(function(cmdId /*string*/) {})```
* ```app.onAppTerminating(function() {})```

The ```options``` object currently only supports the ```encoding``` property, which can be set to "utf-8" or "text/plain;utf-8". To support more encodings, or if you need more options, see the section on extending the native layer below.

The ```cmdId``` menu command IDs are explained in the section "Adding Menu Commands" below.

If you need more native functionality, see the section on extending the native layer below. Also feel free to send a pull request if you've added something you want to share to the implementation :-)

### Building on Windows

Before you build the project in Visual Studio on Windows, you'll need to download the CEF binaries from the [Chromium Embedded Framework](https://code.google.com/p/chromiumembedded/) project page.
Currently, Zephyros is based on CEF 3.1650.1562.

Extract the CEF archive, and copy the folders

* Debug
* Release
* Resources

into the Zephyros project folder _lib/Libcef/DLL/libcef/x64_ if you're building a 64-bit executable and you downloaded the 64-bit version of CEF, or into _lib/Libcef/DLL/libcef/Win32_ if you're building a 32-bit executable.

**Note:** CEF's _Debug_ folder contains the debug version of Chromium and CEF. It is a lot slower than the release version. Typically, you don't need to debug the CEF layer, even if you debug your native Zephyros layer. You can thus safely copy the contents of the _Release_ folder into the _Debug_ folder if you experience performance problems or even crashes when using the CEF debug version.

### Developing and Debugging

You can develop your app as you would develop a regular webapp, i.e., in a browser. You'll only need to support Chrome and Safari, though.

However, in a browser you won't have access to the native layer, thus it makes sense to have a mock implementation in JavaScript of the native layer; you'll find an example in _app/js/mock-app.js_.

When you're ready to test the app in its native shell, you can still debug (inspect the DOM and debug your JavaScript code) when you're building in debug mode with CEF (i.e., on Windows): open a Chrome browser window and navigate to _localhost:19384_, which will give you a "remote" debugger.

Unfortunately, this isn't supported when using the WebView version (i.e., on Mac).

## Extending the JavaScript Native Extension Layer

All the native extension functions are defined in the file _src/native_extensions.cpp_.

There are the following methods to add different flavors of native functions:

* ```AddNativeJavaScriptFunction(String name, NativeFunction* fnx)``` adds a function (i.e., a function whose last argument is expected to be a callback function to which the result(s) are passed);
* ```AddNativeJavaScriptProcedure(String name, NativeFunction* fnx)``` adds a procedure (i.e., a function which doesn't have a callback function argument);
* ```AddNativeJavaScriptCallback(String name, NativeFunction* fnx)``` only registers a callback. To invoke the registered callback functions, use the ```InvokeCallbacks``` method of the ```ClientExtensionHandler``` object.

A ```NativeFunction``` object basically stores a function pointer and argument declarators (the argument types passed in from JavaScript are checked against this specification). Zephyros provides a macro, ```FUNC```, which facilitates creating a ```NativeFunction```, wrapping your code into a C++11 lambda. Here's a simple example showing this:

Say, we want to create a native function which adds two numbers provided as arguments, i.e., something like

```
int myFunction(int firstNumber, int secondNumber) {
    return firstNumber + secondNumber;
}
```

We want to make ```myFunction``` available to JavaScript so it can be invoked like so:

```
var num1 = ...;
var num2 = ...;
app.myFunction(num1, num2, function(result) {
    alert(result);
});
```

Here's the C++ code implementing this function natively:

```
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
```

You'll also find this example in _src/native_extensions.cpp_.

### Adding Menu Commands

First, you'll need to register a event handler in your JavaScript app:

```
app.onMenuCommand(function(cmdId) {
    switch(cmdId) {
    case 'show_about':
        // show the about box
    	break;
    case 'show_help':
        // open your app's help
        break;
    }
});
```

(These two menu commands, "show_about" and "show_help" actually exist in the Zephyros boilerplate.)

#### Mac
* To add a new menu item on Mac, open _MainMenu.xib_ in XCode (it's in the _App_ folder).
* Select the "Main Menu" object, and add the menus and menu items.
* After adding a menu item, open the Identity Inspector and in the "Custom Class" section, set the class to ```GLMenuItem```.
* Still in the Identity Inspector, in the section "User Defined Runtime Attributes", add a new item with Key Path "commandId" of type "String". Set the value to the menu command ID string you'll be handling in your JavaScript.
* In the Connections Inspector connect the "selector" to the ```performClick:``` selector on ```App Delegate```: click and drag the circle next to "selector" in the "Sent Actions" section on the the ```App Delegate``` object and choose ```performClick:```.

#### Windows
* In the Resources view, open the menu.
* Add new menu(s) and/or menu item(s). They will be assigned command constants.
* Open _src/app_win.cpp_ and location the ```InitMenuCommands``` function.
* Add a new entry to the ```g_mapMenuCommands``` map like so: ```g_mapMenuCommands[<your-menu-constant>] = TEXT("<your-menu-cmdid>");``` where ```<your-menu-constant>``` is the menu command constant generated by Visual Studio, and ```<your-menu-cmdid>``` is the menu command ID string you'll be handling in your JavaScript.

## License

The project is licensend under the MIT license.

> Copyright (C) 2013-2014 Vanamco AG
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.