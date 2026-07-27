#pragma once

#include <windows.h>

inline void DebugOutput(const wchar_t* const output)
{
#if defined(_DEBUG)
    OutputDebugStringW(output);
#else
    static_cast<void>(output);
#endif
}
