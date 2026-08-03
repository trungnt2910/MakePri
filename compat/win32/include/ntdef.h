#if __has_include_next(<ntdef.h>)
#include_next <ntdef.h>
#else

#pragma once

#include <winnt.h>

using NTSTATUS = LONG;

#endif // __has_include_next(<ntdef.h>)
