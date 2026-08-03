#pragma once

#include_next <stdio.h>

#include <corecrt_wstdio.h>

extern "C" int _fileno(FILE* stream);
