#include <atomic>

#include <winnt.h>

extern "C" LONG WINAPI InterlockedIncrement(volatile LONG* const value) { return ++std::atomic_ref(*const_cast<LONG*>(value)); }

extern "C" LONG WINAPI InterlockedDecrement(volatile LONG* const value) { return --std::atomic_ref(*const_cast<LONG*>(value)); }

extern "C" LONG WINAPI InterlockedExchange(volatile LONG* const target, const LONG value)
{
    return std::atomic_ref(*const_cast<LONG*>(target)).exchange(value);
}

extern "C" PVOID WINAPI InterlockedCompareExchangePointer(PVOID volatile* const destination, PVOID exchange, PVOID comparand)
{
    std::atomic_ref atomic(*const_cast<PVOID*>(destination));
    atomic.compare_exchange_strong(comparand, exchange);
    return comparand;
}

extern "C" PVOID WINAPI InterlockedExchangePointer(PVOID volatile* const destination, PVOID value)
{
    return std::atomic_ref(*const_cast<PVOID*>(destination)).exchange(value);
}
