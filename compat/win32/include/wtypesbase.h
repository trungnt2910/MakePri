#if __has_include_next(<wtypesbase.h>)
#include_next <wtypesbase.h>
#else

#pragma once

#include <minwindef.h>

enum CLSCTX
{
    CLSCTX_INPROC_SERVER = 0x1,
    CLSCTX_INPROC_HANDLER = 0x2,
    CLSCTX_LOCAL_SERVER = 0x4,
    CLSCTX_REMOTE_SERVER = 0x10,
};

using OLECHAR = WCHAR;
using LPOLESTR = OLECHAR*;

#endif // __has_include_next(<wtypesbase.h>)
