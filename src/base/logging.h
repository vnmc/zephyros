#include "zephyros.h"


#ifdef OS_MACOSX

#ifdef __OBJC__
#define DEBUG_LOG(...) if (Zephyros::UseLogging()) { NSLog(__VA_ARGS__); }
#else
#define DEBUG_LOG(s) if (Zephyros::UseLogging()) { App::Log(s); }
#endif

#define DEBUG_LOGC(s) if (Zephyros::UseLogging()) { App::Log(s); }

#endif


#ifdef OS_WIN
#define DEBUG_LOG(s) if (Zephyros::UseLogging()) { App::Log(s); }
#define DEBUG_LOGC(s) if (Zephyros::UseLogging()) { App::Log(s); }
#endif


#ifdef OS_LINUX
#define DEBUG_LOG(s) if (Zephyros::UseLogging()) { App::Log(s); }
#define DEBUG_LOGC(s) if (Zephyros::UseLogging()) { App::Log(s); }
#endif
