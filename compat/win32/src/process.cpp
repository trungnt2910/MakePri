#include <cstdint>

#include <processthreadsapi.h>

extern "C" HANDLE WINAPI GetCurrentProcess() { return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(-1)); }
