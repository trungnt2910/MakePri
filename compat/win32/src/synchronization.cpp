#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include <synchapi.h>

namespace
{
struct LockRegistry
{
    std::mutex mutex;
    std::unordered_map<PSRWLOCK, std::unique_ptr<std::shared_mutex>> locks;
};

LockRegistry& GetLockRegistry()
{
    static LockRegistry* const registry = new LockRegistry();
    return *registry;
}

std::shared_mutex& GetMutex(const PSRWLOCK lock)
{
    LockRegistry& registry = GetLockRegistry();
    const std::lock_guard guard(registry.mutex);
    auto& mutex = registry.locks[lock];
    if (mutex == nullptr)
    {
        mutex = std::make_unique<std::shared_mutex>();
    }
    return *mutex;
}
} // namespace

extern "C" void WINAPI InitializeSRWLock(const PSRWLOCK lock) { static_cast<void>(GetMutex(lock)); }

extern "C" void WINAPI AcquireSRWLockExclusive(const PSRWLOCK lock) { GetMutex(lock).lock(); }

extern "C" void WINAPI AcquireSRWLockShared(const PSRWLOCK lock) { GetMutex(lock).lock_shared(); }

extern "C" void WINAPI ReleaseSRWLockExclusive(const PSRWLOCK lock) { GetMutex(lock).unlock(); }

extern "C" void WINAPI ReleaseSRWLockShared(const PSRWLOCK lock) { GetMutex(lock).unlock_shared(); }
