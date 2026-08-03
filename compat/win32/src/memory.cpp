#include <cstdlib>
#include <cstring>

#include <errhandlingapi.h>
#include <heapapi.h>
#include <winbase.h>
#include <winerror.h>

namespace
{
struct LocalHeader
{
    SIZE_T size;
};
} // namespace

extern "C" HANDLE WINAPI GetProcessHeap() { return reinterpret_cast<HANDLE>(1); }

extern "C" LPVOID WINAPI HeapAlloc(HANDLE, const DWORD flags, const SIZE_T bytes)
{
    void* const result = std::malloc(bytes == 0 ? 1 : bytes);
    if (result == nullptr)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    else if ((flags & HEAP_ZERO_MEMORY) != 0)
    {
        std::memset(result, 0, bytes);
    }
    return result;
}

extern "C" BOOL WINAPI HeapFree(HANDLE, DWORD, LPVOID memory)
{
    std::free(memory);
    return TRUE;
}

extern "C" BOOL WINAPI HeapSetInformation(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T) { return TRUE; }

extern "C" HLOCAL WINAPI LocalAlloc(const UINT flags, const SIZE_T bytes)
{
    auto* const header = static_cast<LocalHeader*>(std::malloc(sizeof(LocalHeader) + bytes));
    if (header == nullptr)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return nullptr;
    }
    header->size = bytes;
    void* const result = header + 1;
    if ((flags & LMEM_ZEROINIT) != 0)
        std::memset(result, 0, bytes);
    return result;
}

extern "C" SIZE_T WINAPI LocalSize(const HLOCAL memory)
{
    return memory == nullptr ? 0 : (static_cast<const LocalHeader*>(memory) - 1)->size;
}

extern "C" HLOCAL WINAPI LocalFree(const HLOCAL memory)
{
    if (memory != nullptr)
        std::free(static_cast<LocalHeader*>(memory) - 1);
    return nullptr;
}
