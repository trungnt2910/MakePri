#if __has_include_next(<rpc.h>)
#include_next <rpc.h>
#else

#pragma once

#include <guiddef.h>
#include <minwindef.h>

using RPC_STATUS = LONG;

#include <rpcdce.h>
#include <rpcnterr.h>

#endif // __has_include_next(<rpc.h>)
