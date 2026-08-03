#include <algorithm>
#include <string>

#include <shlwapi.h>

#include "internal/strings.h"

extern "C" LPWSTR WINAPI StrChrW(LPCWSTR const string, const WCHAR character)
{
    const std::u16string_view input = win32_compat::WideView(string);
    const char16_t searched = static_cast<char16_t>(character);
    const auto iterator = std::find(input.begin(), input.end(), searched);
    return iterator == input.end() ? nullptr : const_cast<LPWSTR>(string + std::distance(input.begin(), iterator));
}

// libc++ builds its exported wchar_t specialization with the platform wchar width. Instantiating
// the header implementation inside the isolated short-wchar world keeps its storage operations on
// UTF-16 code units; the executable's linker group selects these definitions before native libc++.
template class std::basic_string<wchar_t>;
