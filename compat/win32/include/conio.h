#if __has_include_next(<conio.h>)
#include_next <conio.h>
#else

#pragma once

#include <wchar.h>

extern "C" wint_t _getwche();

#endif // __has_include_next(<conio.h>)
