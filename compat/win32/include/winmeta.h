#if __has_include_next(<winmeta.h>)
#include_next <winmeta.h>
#else

#pragma once

#define WINEVENT_LEVEL_LOG_ALWAYS 0
#define WINEVENT_LEVEL_CRITICAL 1
#define WINEVENT_LEVEL_ERROR 2
#define WINEVENT_LEVEL_WARNING 3
#define WINEVENT_LEVEL_INFO 4
#define WINEVENT_LEVEL_VERBOSE 5

#define WINEVENT_OPCODE_START 1
#define WINEVENT_OPCODE_STOP 2

#endif // __has_include_next(<winmeta.h>)
