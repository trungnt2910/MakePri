#if __has_include_next(<accctrl.h>)
#include_next <accctrl.h>
#else

#pragma once

#include <minwindef.h>

enum SE_OBJECT_TYPE
{
    SE_UNKNOWN_OBJECT_TYPE = 0,
    SE_FILE_OBJECT = 1
};

#define INHERIT_ONLY 0x08

#endif // __has_include_next(<accctrl.h>)
