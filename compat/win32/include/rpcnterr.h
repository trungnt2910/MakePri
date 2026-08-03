#if __has_include_next(<rpcnterr.h>)
#include_next <rpcnterr.h>
#else

#pragma once

#include <winerror.h>

#define RPC_S_OK ERROR_SUCCESS

#endif // __has_include_next(<rpcnterr.h>)
