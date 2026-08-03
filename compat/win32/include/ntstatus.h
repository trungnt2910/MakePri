#if __has_include_next(<ntstatus.h>)
#include_next <ntstatus.h>
#else

#pragma once

#include <ntdef.h>

#define STATUS_INVALID_PARAMETER static_cast<NTSTATUS>(0xC000000DU)
#define STATUS_NO_MEMORY static_cast<NTSTATUS>(0xC0000017U)
#define STATUS_BUFFER_TOO_SMALL static_cast<NTSTATUS>(0xC0000023U)
#define STATUS_NONCONTINUABLE_EXCEPTION static_cast<NTSTATUS>(0xC0000025U)

#endif // __has_include_next(<ntstatus.h>)
