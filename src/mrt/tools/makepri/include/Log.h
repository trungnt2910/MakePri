#pragma once

#include <cstdint>

#include <DefStatus.h>
#include <LogSink.h>
#include <ParameterParser.h>

#include <oleauto.h>
#include <windows.h>

#include <queue>
#include <string>
#include <vector>

namespace Common
{
class AutoBStr
{
public:
    AutoBStr() : m_value(nullptr) {}

    ~AutoBStr() { SysFreeString(m_value); }

    operator BSTR() const { return m_value; }

    BSTR* operator&() { return &m_value; }

private:
    BSTR m_value;
};
} // namespace Common

namespace Microsoft::Resources::Tools::MakePri
{
class Log
{
public:
    static void InfoVerbose(std::uint32_t stringId, ...);
    static void Error(std::uint32_t stringId, HRESULT error, ...);
    static void Item(std::uint32_t messageId, MrmResourceIndexerMessageSeverity severity, const wchar_t* message);
    static void ItemWithFormat(std::uint32_t messageId, MrmResourceIndexerMessageSeverity severity, const wchar_t* format, ...);
    static void Warning(std::uint32_t stringId, HRESULT error, ...);
    static void WriteWorkOutput(std::uint32_t stringId, ...);
    static void NewLine();
    static void InfoUnderscored(std::uint32_t stringId, ...);
    static void DisplayErrorMessage(const wchar_t* message, HRESULT error);
    static void StoreErrorMessage(std::uint32_t stringId, HRESULT error, ...);
    static bool DisplayStoredErrorMessages();
    static void RetrieveMessages(const std::vector<Indexers::LogItem>* messages);
    static bool Flush();
    static void DisplayStatusErrorAndWarnings(IDefStatusEx* status);
    static void StoreLastErrorInfo();
    static void PrintUsage(InputArgs* args);

    static bool _bUseVerbose;

private:
    static std::wstring FormatErrorPrefix(const wchar_t* prefix, std::uint32_t stringId, HRESULT error);
    static void _WriteFormattedLine(
        std::uint32_t stringId,
        bool warning,
        const wchar_t* prefix,
        HRESULT error,
        const wchar_t* description,
        const wchar_t* where,
        std::uint32_t line,
        const wchar_t* repeatedDescription);
    static std::uint32_t _GetErrorStringId(HRESULT error);
    static void _DisplayMappedMsg(const IDefStatus* status, bool warning);

    static std::queue<Indexers::LogItem> s_delayedMessageQueue;
    static std::queue<Indexers::LogItem> s_messageQueue;
};
} // namespace Microsoft::Resources::Tools::MakePri
