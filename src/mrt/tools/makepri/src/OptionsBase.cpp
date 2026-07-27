#include "StdAfx.h"

#include <ParameterManager.h>

namespace Microsoft::Resources
{
namespace
{

void SetStatusError(IDefStatusEx* const status, const HRESULT error, const std::uint32_t line)
{
    if (status != nullptr)
    {
        status->SetError(error, L"" __FILE__, line, L"", 0);
    }
}

} // namespace

OptionsBase::OptionsBase(
    const std::uint64_t ignoredInitialFlags,
    const std::uint32_t optionCount,
    const OptionSpec* const options,
    const std::uint32_t stringOptionCount,
    const StringOptionSpec* const stringOptions) :
    m_optionCount(optionCount), m_options(options), m_stringOptionCount(stringOptionCount), m_stringOptions(stringOptions)
{
    static_cast<void>(ignoredInitialFlags);
    if (m_stringOptionCount != 0)
    {
        const std::size_t size = _DefArray_Size(sizeof(wchar_t*), m_stringOptionCount);
        if (size != 0)
        {
            m_stringValues = static_cast<wchar_t**>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size));
        }
    }
}

OptionsBase::~OptionsBase()
{
    if (m_stringValues == nullptr)
    {
        return;
    }
    for (std::uint32_t index = 0; index < m_stringOptionCount; ++index)
    {
        if (m_stringValues[index] != nullptr)
        {
            HeapFree(GetProcessHeap(), 0, m_stringValues[index]);
        }
    }
    HeapFree(GetProcessHeap(), 0, m_stringValues);
}

bool OptionsBase::Initialize(const wchar_t* const options, IDefStatusEx* const status)
{
    const std::uint32_t valueCount = m_stringValues != nullptr ? m_stringOptionCount : 0;
    return ParseOptionsString(options, valueCount, status, &m_flags, m_stringValues) && VerifyStringOptions(status);
}

bool OptionsBase::VerifyStringOptions(IDefStatusEx* const status) { return status->Succeeded(); }

bool OptionsBase::ParseOptionsString(
    const wchar_t* const options,
    const std::uint32_t valueCount,
    IDefStatusEx* const status,
    std::uint64_t* const flags,
    wchar_t** const values)
{
    wchar_t buffer[MAX_PATH] {};
    const HRESULT copyResult = StringCchCopyW(buffer, MAX_PATH, options);
    if (FAILED(copyResult))
    {
        SetStatusError(status, copyResult, 214);
        return status->Succeeded();
    }

    wchar_t* token = buffer;
    std::uint64_t parsedFlags = *flags;
    while (*token != L'\0')
    {
        wchar_t* next = StrChrW(token, L',');
        if (next != nullptr)
        {
            *next = L'\0';
            ++next;
        }

        const wchar_t firstCharacter = *token;
        if (firstCharacter == L'-' || firstCharacter == L'+')
        {
            ++token;
        }

        wchar_t* value = StrChrW(token, L'=');
        if (value != nullptr)
        {
            *value = L'\0';
            ++value;

            std::uint32_t optionIndex = 0;
            while (optionIndex < m_stringOptionCount &&
                   DefString_CompareWithOptions(token, m_stringOptions[optionIndex].longName, DefCompare_CaseInsensitive) != Def_Equal &&
                   DefString_CompareWithOptions(token, m_stringOptions[optionIndex].shortName, DefCompare_CaseInsensitive) != Def_Equal)
            {
                SetStatusError(status, E_DEF_NOT_READY, 195);
                ++optionIndex;
            }

            if (optionIndex >= m_stringOptionCount)
            {
                SetStatusError(status, E_DEF_KEY_NOT_FOUND, 202);
                break;
            }
            if (values != nullptr && optionIndex < valueCount)
            {
                if (values[optionIndex] != nullptr)
                {
                    SetStatusError(status, E_DEF_ENTRY_ALREADY_EXISTS, 187);
                }
                else
                {
                    const HRESULT hr = DefString_Dup(value, &values[optionIndex]);
                    if (FAILED(hr))
                    {
                        SetStatusError(status, hr, 187);
                    }
                }
            }
        }
        else
        {
            std::uint32_t optionIndex = 0;
            while (optionIndex < m_optionCount &&
                   DefString_CompareWithOptions(token, m_options[optionIndex].longName, DefCompare_CaseInsensitive) != Def_Equal &&
                   DefString_CompareWithOptions(token, m_options[optionIndex].shortName, DefCompare_CaseInsensitive) != Def_Equal)
            {
                ++optionIndex;
            }

            if (optionIndex >= m_optionCount)
            {
                SetStatusError(status, E_DEF_KEY_NOT_FOUND, 202);
                break;
            }

            const OptionSpec& spec = m_options[optionIndex];
            const std::uint64_t selected = firstCharacter == L'-' ? spec.negativeValue : spec.positiveValue;
            parsedFlags = selected | (parsedFlags & ~spec.mask);
        }

        token = next;
        if (next == nullptr)
        {
            break;
        }
    }

    *flags = parsedFlags;
    return status->Succeeded();
}

} // namespace Microsoft::Resources
