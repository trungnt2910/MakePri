#if __has_include_next(<synchapi.h>)
#include_next <synchapi.h>
#else

#pragma once

#include <minwindef.h>

using SRWLOCK = RTL_SRWLOCK;
using PSRWLOCK = SRWLOCK*;
#define SRWLOCK_INIT RTL_SRWLOCK_INIT

extern "C"
{
    void WINAPI InitializeSRWLock(PSRWLOCK lock);
    void WINAPI AcquireSRWLockExclusive(PSRWLOCK lock);
    void WINAPI AcquireSRWLockShared(PSRWLOCK lock);
    void WINAPI ReleaseSRWLockExclusive(PSRWLOCK lock);
    void WINAPI ReleaseSRWLockShared(PSRWLOCK lock);
    void WINAPI Sleep(DWORD milliseconds);
}

#endif // __has_include_next(<synchapi.h>)
