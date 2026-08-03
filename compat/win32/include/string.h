#pragma once

#include_next <string.h>

#include <stddef.h>

#include <corecrt.h>

#ifndef _NLSCMPERROR
#define _NLSCMPERROR 0x7fffffff
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    int memcpy_s(void* destination, size_t destinationSize, const void* source, size_t count);
#ifdef __cplusplus
}
#endif
