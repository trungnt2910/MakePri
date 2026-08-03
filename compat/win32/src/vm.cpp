#include <cstring>

#include <memoryapi.h>

extern "C" SIZE_T WINAPI VirtualQuery(LPCVOID const address, PMEMORY_BASIC_INFORMATION const information, const SIZE_T length)
{
    if (address == nullptr || information == nullptr || length < sizeof(*information))
    {
        return 0;
    }
    std::memset(information, 0, sizeof(*information));
    information->BaseAddress = const_cast<void*>(address);
    information->AllocationBase = const_cast<void*>(address);
    information->RegionSize = static_cast<SIZE_T>(-1);
    return sizeof(*information);
}
