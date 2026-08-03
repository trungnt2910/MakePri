#if __has_include_next(<heapapi.h>)
#include_next <heapapi.h>
#else

#pragma once

#include <minwindef.h>

enum HEAP_INFORMATION_CLASS
{
    HeapCompatibilityInformation = 0,
    HeapEnableTerminationOnCorruption = 1,
};

extern "C" BOOL WINAPI
HeapSetInformation(HANDLE heap, HEAP_INFORMATION_CLASS informationClass, PVOID information, SIZE_T informationLength);

extern "C"
{
    HANDLE WINAPI GetProcessHeap();
    LPVOID WINAPI HeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes);
    BOOL WINAPI HeapFree(HANDLE heap, DWORD flags, LPVOID memory);
}

#endif // __has_include_next(<heapapi.h>)
