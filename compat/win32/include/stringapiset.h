#if __has_include_next(<stringapiset.h>)
#include_next <stringapiset.h>
#else

#pragma once

#include <minwindef.h>

extern "C"
{
    int WINAPI MultiByteToWideChar(UINT codePage, DWORD flags, LPCCH source, int sourceLength, LPWSTR destination, int destinationLength);
    int WINAPI WideCharToMultiByte(
        UINT codePage,
        DWORD flags,
        LPCWCH source,
        int sourceLength,
        LPSTR destination,
        int destinationLength,
        LPCSTR defaultChar,
        LPBOOL usedDefaultChar);
    int WINAPI CompareStringOrdinal(LPCWCH left, int leftLength, LPCWCH right, int rightLength, BOOL ignoreCase);
}

#endif // __has_include_next(<stringapiset.h>)
