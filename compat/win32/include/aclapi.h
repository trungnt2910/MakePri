#if __has_include_next(<aclapi.h>)
#include_next <aclapi.h>
#else

#pragma once

#include <accctrl.h>
#include <securitybaseapi.h>

extern "C"
{
    DWORD WINAPI GetNamedSecurityInfoW(
        LPCWSTR objectName,
        SE_OBJECT_TYPE objectType,
        SECURITY_INFORMATION information,
        PSID* owner,
        PSID* group,
        PACL* dacl,
        PACL* sacl,
        PSECURITY_DESCRIPTOR* descriptor);
    DWORD WINAPI SetNamedSecurityInfoW(
        LPWSTR objectName,
        SE_OBJECT_TYPE objectType,
        SECURITY_INFORMATION information,
        PSID owner,
        PSID group,
        PACL dacl,
        PACL sacl);
}

#ifdef UNICODE
#define GetNamedSecurityInfo GetNamedSecurityInfoW
#define SetNamedSecurityInfo SetNamedSecurityInfoW
#endif // __has_include_next(<aclapi.h>)

#endif
