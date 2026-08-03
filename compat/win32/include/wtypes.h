#if __has_include_next(<wtypes.h>)
#include_next <wtypes.h>
#else

#pragma once

#include <wtypesbase.h>

using BSTR = wchar_t*;
using VARTYPE = USHORT;
using VARIANT_BOOL = SHORT;

#define VARIANT_TRUE static_cast<VARIANT_BOOL>(-1)
#define VARIANT_FALSE static_cast<VARIANT_BOOL>(0)

#define VT_EMPTY 0
#define VT_I4 3
#define VT_R8 5
#define VT_BSTR 8
#define VT_DISPATCH 9
#define VT_BOOL 11
#define VT_UNKNOWN 13

#endif // __has_include_next(<wtypes.h>)
