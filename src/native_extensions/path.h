//
//  path.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#ifndef Ghostlab_path_h
#define Ghostlab_path_h

#ifdef USE_CEF
#include "lib/cef/include/base/cef_lock.h"
#include "lib/cef/include/cef_client.h"
#include "lib/cef/include/wrapper/cef_helpers.h"
#endif

#include "native_extensions.h"

#ifdef OS_WIN
#define PATH_SEPARATOR TEXT('\\')
#else
#define PATH_SEPARATOR TEXT('/')
#endif

// the class "Path" is declared in native_extensions.h

#endif
