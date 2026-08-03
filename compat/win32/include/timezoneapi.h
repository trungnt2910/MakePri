#if __has_include_next(<timezoneapi.h>)
#include_next <timezoneapi.h>
#else

#pragma once

#include <minwinbase.h>

extern "C" BOOL WINAPI SystemTimeToFileTime(const SYSTEMTIME* systemTime, FILETIME* fileTime);

#endif // __has_include_next(<timezoneapi.h>)
