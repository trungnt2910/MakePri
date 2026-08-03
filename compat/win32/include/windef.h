#if __has_include_next(<windef.h>)
#include_next <windef.h>
#else

#pragma once

#include <minwindef.h>

using HWND = HANDLE;

#endif // __has_include_next(<windef.h>)
