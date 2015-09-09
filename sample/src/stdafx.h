// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef OS_WIN
#include "targetver.h"
#endif


// Define "NO_SPARKLE" if you don't want to use the in-app updater.
// If you don't use the updater (i.e., NO_SPARKLE is defined),
// include "Zephyros/src/stubs_win.cpp" in your project.
// If you use the updater, link your project against
// "Zephyros/lib/winsparkle/$(Platform)/WinSparkle.lib" and make sure to
// include the WinSparkle.dll in your executable's directory.
//#define NO_SPARKLE

// Define "NO_CRASHRPT" if you don't want to use crash reporting.
// If you don't use crash reporting (i.e., NO_CRASHRPT is defined),
// include "Zephyros/src/stubs_win.cpp" in your project.
// If you use crash reporting, link your project against
// "Zephyros/lib/CrashRpt/$(Platform)/CrashRpt*.lib" and make sure to
// include CrashRpt's DLLs, EXE and INI files in your executable's directory.
//#define NO_CRASHRPT


#include "zephyros.h"


// Include the headers below only if you want to add your own custom native extensions.

#ifdef USE_CEF
#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/base/cef_lock.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/wrapper/cef_helpers.h"
#endif

#include "native_extensions.h"
