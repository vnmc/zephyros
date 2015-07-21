//#define DEBUG_LOGGING_ON

#if defined(OS_MACOSX)

#ifdef DEBUG_LOGGING_ON
#define DEBUG_LOG(...) NSLog(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

#elif defined(OS_WIN)

#ifdef DEBUG_LOGGING_ON
#define DEBUG_LOG(s) App::Log(s)
#else
#define DEBUG_LOG(...)
#endif

#endif
