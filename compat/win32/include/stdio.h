#pragma once

#ifndef _WIN32
// The prototpye for fclose on some systems has nonnull(1),
// which triggers UBSan if fclose(NULL) is called.
#define fclose system_fclose
#endif

#include_next <stdio.h>

#ifndef _WIN32
#undef fclose
extern "C" int fclose(FILE* stream);
#endif

#include <corecrt_wstdio.h>

extern "C" int _fileno(FILE* stream);
