#if __has_include_next(<io.h>)
#include_next <io.h>
#else

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
    int _setmode(int fileDescriptor, int mode);
    long long _filelengthi64(int fileDescriptor);
#ifdef __cplusplus
}
#endif

#endif // __has_include_next(<io.h>)
