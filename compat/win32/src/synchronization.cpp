#include <mutex>
#include <new>

#include <synchapi.h>

namespace
{
std::mutex* GetMutex(PSRWLOCK const lock) { return static_cast<std::mutex*>(lock->Ptr); }
} // namespace

extern "C" void WINAPI InitializeSRWLock(PSRWLOCK const lock) { lock->Ptr = new (std::nothrow) std::mutex(); }

extern "C" void WINAPI AcquireSRWLockExclusive(PSRWLOCK const lock) { GetMutex(lock)->lock(); }
extern "C" void WINAPI AcquireSRWLockShared(PSRWLOCK const lock) { GetMutex(lock)->lock(); }
extern "C" void WINAPI ReleaseSRWLockExclusive(PSRWLOCK const lock) { GetMutex(lock)->unlock(); }
extern "C" void WINAPI ReleaseSRWLockShared(PSRWLOCK const lock) { GetMutex(lock)->unlock(); }
