#if __has_include_next(<processthreadsapi.h>)
#include_next <processthreadsapi.h>
#else

#pragma once

#include <minwindef.h>

extern "C"
{
    HANDLE WINAPI GetCurrentProcess();
    DWORD WINAPI GetCurrentThreadId();
}

#endif // __has_include_next(<processthreadsapi.h>)
