#if __has_include_next(<basetsd.h>)
#include_next <basetsd.h>
#else

#pragma once

#include <cstddef>
#include <cstdint>

using INT8 = std::int8_t;
using INT16 = std::int16_t;
using INT32 = std::int32_t;
using INT64 = std::int64_t;
using UINT8 = std::uint8_t;
using UINT16 = std::uint16_t;
using UINT32 = std::uint32_t;
using UINT64 = std::uint64_t;
using LONG32 = std::int32_t;
using ULONG32 = std::uint32_t;
using LONG64 = std::int64_t;
using ULONG64 = std::uint64_t;
using INT_PTR = std::intptr_t;
using UINT_PTR = std::uintptr_t;
using LONG_PTR = std::intptr_t;
using ULONG_PTR = std::uintptr_t;
using DWORD_PTR = std::uintptr_t;
using SIZE_T = std::size_t;
using SSIZE_T = std::ptrdiff_t;
using DWORD64 = std::uint64_t;

#endif // __has_include_next(<basetsd.h>)
