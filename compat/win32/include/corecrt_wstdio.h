#if __has_include_next(<corecrt_wstdio.h>)
#include_next <corecrt_wstdio.h>
#else

#pragma once

#include <stdio.h>

extern "C"
{
    FILE* _wfopen(const wchar_t* path, const wchar_t* mode);
    int _wfopen_s(FILE** stream, const wchar_t* path, const wchar_t* mode);
}

#endif // __has_include_next(<corecrt_wstdio.h>)
