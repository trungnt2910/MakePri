#if __has_include_next(<winver.h>)
#include_next <winver.h>
#else

#pragma once

#define VOS_NT_WINDOWS32 0x00040004L
#define VFT_APP 0x00000001L
#define VFT2_UNKNOWN 0x00000000L

#endif // __has_include_next(<winver.h>)
