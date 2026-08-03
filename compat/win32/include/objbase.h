#if __has_include_next(<objbase.h>)
#include_next <objbase.h>
#else

#pragma once

#include <combaseapi.h>

#define STGM_WRITE 0x00000001L
#define STGM_SHARE_DENY_WRITE 0x00000020L
#define STGM_CREATE 0x00001000L

#define COINIT_MULTITHREADED 0x0

#endif // __has_include_next(<objbase.h>)
