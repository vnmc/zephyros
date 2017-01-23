Windows / Linux
===============

The "Win32" and "x64" subfolders in the folder "libcef/windows" and the
"i386" and "x86_64" subfolders in the folder "libcef/linux" each contain the
following subfolders of the Windows 32bit and 64bit and Linux 32bit and 64bit
builds of the CEF project, respectively
(http://opensource.spotify.com/cefbuilds/index.html, currently branch 2883):

- Debug
- Release
- Resources

These folders aren't contained in this git project, please download the binary CEF distributions
from

    Windows 64-bit:
      http://opensource.spotify.com/cefbuilds/cef_binary_3.2883.1545.gd685d27_windows64.tar.bz2
    Windows 32-bit:
      http://opensource.spotify.com/cefbuilds/cef_binary_3.2883.1545.gd685d27_windows32.tar.bz2

    Linux 64-bit:
      http://opensource.spotify.com/cefbuilds/cef_binary_3.2883.1545.gd685d27_linux64.tar.bz2
    Linux 32-bit:
      http://opensource.spotify.com/cefbuilds/cef_binary_3.2883.1545.gd685d27_linux32.tar.bz2

and copy them into the subfolders.



macOS
=====

Download the CEF binary for Mac from
    http://opensource.spotify.com/cefbuilds/cef_binary_3.2883.1543.g6e0baff_macosx64.tar.bz2
and copy Release/Chromium Embedded Framework.framework into the folder
    "libcef/mac"
