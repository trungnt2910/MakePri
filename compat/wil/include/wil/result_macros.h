#pragma once

#include <new>

#include <windows.h>

#define RETURN_HR(result) return static_cast<HRESULT>(result)
#define RETURN_HR_IF(result, condition) \
    do \
    { \
        if (condition) \
            return static_cast<HRESULT>(result); \
    } while (false)
#define RETURN_HR_IF_NULL(result, pointer) RETURN_HR_IF((result), (pointer) == nullptr)
#define RETURN_HR_IF_EXPECTED(result, condition) RETURN_HR_IF((result), (condition))
#define RETURN_HR_IF_NULL_EXPECTED(result, pointer) RETURN_HR_IF_NULL((result), (pointer))
#define RETURN_IF_FAILED(expression) \
    do \
    { \
        const HRESULT wil_result = static_cast<HRESULT>(expression); \
        if (FAILED(wil_result)) \
            return wil_result; \
    } while (false)
#define RETURN_IF_FAILED_EXPECTED(expression) RETURN_IF_FAILED(expression)
#define RETURN_IF_FAILED_WITH_EXPECTED(expression, ...) RETURN_IF_FAILED(expression)
#define RETURN_IF_NULL_ALLOC(pointer) RETURN_HR_IF(E_OUTOFMEMORY, (pointer) == nullptr)
#define RETURN_IF_WIN32_BOOL_FALSE(expression) \
    do \
    { \
        if (!(expression)) \
            return HRESULT_FROM_WIN32(GetLastError()); \
    } while (false)
#define RETURN_IF_WIN32_BOOL_FALSE_EXPECTED(expression) RETURN_IF_WIN32_BOOL_FALSE(expression)
#define RETURN_IF_WIN32_ERROR(expression) \
    do \
    { \
        const DWORD wil_error = static_cast<DWORD>(expression); \
        if (wil_error != ERROR_SUCCESS) \
            return HRESULT_FROM_WIN32(wil_error); \
    } while (false)
#define RETURN_LAST_ERROR() return HRESULT_FROM_WIN32(GetLastError())
#define RETURN_LAST_ERROR_IF(condition) \
    do \
    { \
        if (condition) \
            RETURN_LAST_ERROR(); \
    } while (false)
#define RETURN_LAST_ERROR_IF_NULL(pointer) RETURN_LAST_ERROR_IF((pointer) == nullptr)

#define THROW_HR_IF_MSG(result, condition, ...) \
    do \
    { \
        if (condition) \
            throw static_cast<HRESULT>(result); \
    } while (false)

#define LOG_HR(result) static_cast<HRESULT>(result)
#define SUCCEEDED_LOG(result) SUCCEEDED(result)
#define LOG_LAST_ERROR_IF_NULL_MSG(pointer, ...) ((void)(pointer))
#define LOG_LAST_ERROR_IF_MSG(condition, ...) ((void)(condition))
