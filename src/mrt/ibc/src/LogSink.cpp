#include "StdAfx.h"

#include <LogSink.h>

namespace Microsoft::Resources::Indexers
{
void LogSink::WriteError(
    const wchar_t* const file,
    const wchar_t* const category,
    const wchar_t* const severity,
    const int line,
    const int column,
    const HRESULT error,
    const wchar_t* const format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    if (line <= 0)
    {
        wprintf(L"%s : %s %s %08x : ", file, category, severity, error);
    }
    else if (column > 0)
    {
        wprintf(L"%s(%d,%d) : %s %s %08x : ", file, line, column, category, severity, error);
    }
    else
    {
        wprintf(L"%s(%d) : %s %s %08x : ", file, line, category, severity, error);
    }
    vwprintf_s(format, arguments);
    wprintf_s(L"\n");
    va_end(arguments);
}

void LogSink::WriteError(
    const wchar_t* const file,
    const wchar_t* const category,
    const wchar_t* const severity,
    const HRESULT error,
    const wchar_t* const format,
    ...)
{
    WriteError(file, category, severity, 0, 0, error, format);
}
} // namespace Microsoft::Resources::Indexers
