Windows / Linux
===============

The "Win32" and "x64" subfolders in the folder "libcef/windows" and the
"i386" and "x86_64" subfolders in the folder "libcef/linux" each contain the
following subfolders of the Windows 32bit and 64bit and Linux 32bit and 64bit
builds of the CEF project, respectively
(https://cefbuilds.com/, currently branch 2883):

- Debug
- Release
- Resources

These folders aren't contained in this git project, please download the binary CEF distributions
from http://opensource.spotify.com/cefbuilds/index.html (at least the one for the architecture you want to build for)
and copy them into the subfolders.



macOS
=====

Download the CEF binary for Mac from
    http://opensource.spotify.com/cefbuilds/index.html
and copy Release/Chromium Embedded Framework.framework into the folder
    "libcef/mac"
