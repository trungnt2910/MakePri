#if __has_include_next(<corecrt.h>)
#include_next <corecrt.h>
#else

#pragma once

using errno_t = int;

struct threadlocaleinfostruct;
struct threadmbcinfostruct;
typedef struct threadlocaleinfostruct* pthreadlocinfo;
typedef struct threadmbcinfostruct* pthreadmbcinfo;
typedef struct localeinfo_struct
{
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
} _locale_tstruct, *_locale_t;

#endif // __has_include_next(<corecrt.h>)
