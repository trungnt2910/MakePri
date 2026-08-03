#if __has_include_next(<rpcdce.h>)
#include_next <rpcdce.h>
#else

#pragma once

#include <rpc.h>

using UUID = GUID;
using RPC_WSTR = unsigned short*;

extern "C"
{
    RPC_STATUS WINAPI UuidCreate(UUID* uuid);
    RPC_STATUS WINAPI UuidToStringW(const UUID* uuid, RPC_WSTR* stringUuid);
    RPC_STATUS WINAPI RpcStringFreeW(RPC_WSTR* string);
}

#ifdef UNICODE
#define UuidToString UuidToStringW
#define RpcStringFree RpcStringFreeW
#endif

#endif // __has_include_next(<rpcdce.h>)
