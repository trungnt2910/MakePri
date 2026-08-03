#pragma once

#include <string_view>

namespace win32_compat
{

void SetErrorDescription(std::u16string_view description);

} // namespace win32_compat
