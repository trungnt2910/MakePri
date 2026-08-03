#pragma once

#include_next <stdlib.h>

#include <corecrt.h>

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof((array)[0]))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    void qsort_s(void* base, size_t count, size_t width, int (*compare)(void* context, const void* left, const void* right), void* context);
#ifdef __cplusplus
}
#endif
