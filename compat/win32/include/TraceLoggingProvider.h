#if __has_include_next(<TraceLoggingProvider.h>)
#include_next <TraceLoggingProvider.h>
#else

#pragma once

#include <evntprov.h>

using TraceLoggingHProvider = void*;
using TLG_PENABLECALLBACK = void (*)();

struct _TlgReflectorTag_Param0IsProviderType
{};

#define TRACELOGGING_DEFINE_PROVIDER_STORAGE(name, ...) static unsigned char name

#define TraceLoggingRegister(...) ((void)0)
#define TraceLoggingRegisterEx(...) ((void)0)
#define TraceLoggingUnregister(...) ((void)0)
#define TraceLoggingProviderEnabled(...) false

#define TraceLoggingWrite(...) ((void)0)
#define TraceLoggingWriteActivity(...) ((void)0)

#define _tlg_DefineProvider_annotation(...) ((void)0)
#define _tlg_FOREACH(macro, ...)

#endif // __has_include_next(<TraceLoggingProvider.h>)
