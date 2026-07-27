#pragma once

#if __has_include(<MrmResourceIndexer.h>)
#include <MrmResourceIndexer.h>
#else
enum MrmResourceIndexerMessageSeverity
{
    MrmResourceIndexerMessageSeverityVerbose = 0,
    MrmResourceIndexerMessageSeverityInfo = 1,
    MrmResourceIndexerMessageSeverityWarning = 2,
    MrmResourceIndexerMessageSeverityError = 3,
};
#endif

#include <windows.h>

#include <string>

namespace Microsoft::Resources::Indexers
{
struct LogItem
{
    LogItem(MrmResourceIndexerMessageSeverity severity, DWORD messageId, std::wstring message);
    LogItem(const LogItem&) = default;
    ~LogItem() = default;

    MrmResourceIndexerMessageSeverity severity;
    DWORD messageId;
    std::wstring message;
};

class LogSink
{
public:
    static void WriteError(
        const wchar_t* file,
        const wchar_t* category,
        const wchar_t* severity,
        int line,
        int column,
        HRESULT error,
        const wchar_t* format,
        ...);
    static void WriteError(
        const wchar_t* file,
        const wchar_t* category,
        const wchar_t* severity,
        HRESULT error,
        const wchar_t* format,
        ...);
};
} // namespace Microsoft::Resources::Indexers
