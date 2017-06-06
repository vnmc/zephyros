
Zephyros
========

*A multi-platform HTML5 application wrapper.*


![Intro Zephyros](http://s29.postimg.org/ax53mwrmv/Newsletter_open_source.png)


## Synopsis

Zephyros is a library/framework which provides a native application shell for HTML5 desktop apps.

Zephyros enables you to create your GUI in HTML5/CSS and write your application logic in JavaScript. For operating system functionality which can't be accessed normally by JavaScript, Zephyros provides an easily extensible native layer.

Currently, Mac and Windows are supported. The Linux version still lacks implementation of some native features, but if you don't need them, you're good to go. (Please note, however, that the Linux version uses GTK3 and the default branch no longer builds on Linux because CEF introduced an incompatibility; for Linux, please use the ```develop-cef2623``` branch.)

On Windows, the Chromium browser is used via the [Chromium Embedded Framework](https://code.google.com/p/chromiumembedded/) (CEF); on Mac you have the choice between CEF and the native WebView, which is based on Safari. Note that if you want to publish and sell your app on the Mac AppStore, you should use WebView (using CEF will most likely get the app rejected). 

This project is similar to Adobe's [brackets-shell](https://github.com/adobe/brackets-shell) or Github's [Electron](https://electron.atom.io/). The differences are that Zephyros gives the option to use WebView on Mac (for compatibility with the AppStore), and that Zephyros provides (in our opinion) a more easily extensible JavaScript extension ("native") layer.

Another difference is that Zephyros is packaged as a library (a static library on Windows and a framework on Mac, against which you link your own app), so whenever Zephyros is updated you only need to update the library, not your source code.
(In release 1 of Zephyros, this was different; Zephyros was the base on which you could build your app (just like brackets-shell). If you're looking for that, you can still find it in the "Release-1" branch.)

In addition, Zephyros comes with built-in licensing allowing you to build demo and paid versions of your app, and the in-app updater [Sparkle](http://sparkle-project.org/) / [WinSparkle](http://winsparkle.org/).

## Using Zephyros for Your Own Apps

Please have a look at the [Wiki](https://github.com/vnmc/zephyros/wiki) for instructions how to create your Zephyros apps.

## License

The project is licensend under the MIT license.

> Copyright (C) 2013-2017 Vanamco AG, http://www.vanamco.com
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
