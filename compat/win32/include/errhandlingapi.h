#if __has_include_next(<errhandlingapi.h>)
#include_next <errhandlingapi.h>
#else

#pragma once

#include <minwindef.h>

extern "C"
{
    DWORD WINAPI GetLastError();
    void WINAPI SetLastError(DWORD error);
}

#endif // __has_include_next(<errhandlingapi.h>)
