#if __has_include_next(<securitybaseapi.h>)
#include_next <securitybaseapi.h>
#else

#pragma once

#include <minwindef.h>

extern "C"
{
    PVOID WINAPI FreeSid(PSID sid);
    BOOL WINAPI AllocateAndInitializeSid(
        PSID_IDENTIFIER_AUTHORITY authority,
        BYTE subAuthorityCount,
        DWORD subAuthority0,
        DWORD subAuthority1,
        DWORD subAuthority2,
        DWORD subAuthority3,
        DWORD subAuthority4,
        DWORD subAuthority5,
        DWORD subAuthority6,
        DWORD subAuthority7,
        PSID* sid);
    BOOL WINAPI GetAce(PACL acl, DWORD index, LPVOID* ace);
    BOOL WINAPI EqualSid(PSID left, PSID right);
    DWORD WINAPI GetLengthSid(PSID sid);
    BOOL WINAPI AddAccessAllowedAceEx(PACL acl, DWORD revision, DWORD flags, DWORD mask, PSID sid);
}

#endif // __has_include_next(<securitybaseapi.h>)
