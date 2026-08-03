#if __has_include_next(<appmodel.h>)
#include_next <appmodel.h>
#else

#pragma once

#include <minwindef.h>
#include <winerror.h>

extern "C"
{
    LONG WINAPI GetCurrentPackageFullName(UINT32* packageFullNameLength, PWSTR packageFullName);
}

#endif // __has_include_next(<appmodel.h>)
