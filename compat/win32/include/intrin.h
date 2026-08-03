#if __has_include_next(<intrin.h>)
#include_next <intrin.h>
#else

#pragma once

#include <cstdlib>

#define _ReturnAddress() __builtin_return_address(0)
#define __fastfail(code) std::abort()

#endif // __has_include_next(<intrin.h>)
