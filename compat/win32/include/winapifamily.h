#if __has_include_next(<winapifamily.h>)
#include_next <winapifamily.h>
#else

#pragma once

#define WINAPI_PARTITION_DESKTOP 1
#define WINAPI_PARTITION_APP 1
#define WINAPI_PARTITION_SYSTEM 1
#define WINAPI_FAMILY WINAPI_PARTITION_APP
#define WINAPI_FAMILY_PARTITION(partition) (partition)

#endif // __has_include_next(<winapifamily.h>)
