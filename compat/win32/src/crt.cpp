#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <vector>

#include <stdlib.h>
#include <string.h>

extern "C" int memcpy_s(void* const destination, const std::size_t destinationSize, const void* const source, const std::size_t count)
{
    if ((destination == nullptr && destinationSize != 0) || source == nullptr || count > destinationSize)
    {
        if (destination != nullptr && destinationSize != 0)
            std::memset(destination, 0, destinationSize);
        return EINVAL;
    }
    if (count != 0)
        std::memmove(destination, source, count);
    return 0;
}

extern "C" void qsort_s(
    void* const base,
    const std::size_t count,
    const std::size_t width,
    int (*const compare)(void* context, const void* left, const void* right),
    void* const context)
{
    if (base == nullptr || compare == nullptr || width == 0)
        return;
    auto* const bytes = static_cast<unsigned char*>(base);
    std::vector<unsigned char> temporary(width);
    for (std::size_t index = 1; index < count; ++index)
    {
        std::size_t position = index;
        std::memcpy(temporary.data(), bytes + index * width, width);
        while (position > 0 && compare(context, bytes + (position - 1) * width, temporary.data()) > 0)
        {
            std::memmove(bytes + position * width, bytes + (position - 1) * width, width);
            --position;
        }
        std::memcpy(bytes + position * width, temporary.data(), width);
    }
}
