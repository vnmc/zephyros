// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef OS_WIN
#include "targetver.h"
#endif

#include "zephyros.h"


// include this headers only if you want to add more native extensions

#ifdef USE_CEF
#include "lib/cef/include/cef_base.h"
#include "lib/cef/include/base/cef_lock.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/wrapper/cef_helpers.h"
#endif

#include "native_extensions.h"
