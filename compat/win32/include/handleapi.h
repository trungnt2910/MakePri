#if __has_include_next(<handleapi.h>)
#include_next <handleapi.h>
#else

#pragma once

#include <minwindef.h>

#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1))

extern "C" BOOL WINAPI CloseHandle(HANDLE handle);

#endif // __has_include_next(<handleapi.h>)
