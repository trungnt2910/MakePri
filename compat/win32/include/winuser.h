#if __has_include_next(<winuser.h>)
#include_next <winuser.h>
#else

#pragma once

#include <windef.h>

#define RT_MANIFEST 24

extern "C" int WINAPI LoadStringW(HINSTANCE instance, UINT id, LPWSTR buffer, int bufferLength);
extern "C" int WINAPI LoadStringA(HINSTANCE instance, UINT id, LPSTR buffer, int bufferLength);

#endif // __has_include_next(<winuser.h>)
