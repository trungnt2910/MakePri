#pragma once

#include <span>
#include <vector>

namespace win32_compat
{

void RegisterResourceData(std::span<const unsigned char> data);
std::vector<std::span<const unsigned char>> RegisteredResourceData();

} // namespace win32_compat
