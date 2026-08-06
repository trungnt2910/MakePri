#include "StdAfx.h"

#include <Bcp47.h>

HRESULT LocalStringsArray::Initialize(const wchar_t* const strings, const wchar_t separator)
{
    if (strings == nullptr)
    {
        return E_INVALIDARG;
    }

    if (m_strings != nullptr)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_OPERATION);
    }

    HRESULT result = S_OK;
    if (separator != L'\0')
    {
        const std::size_t length = std::wcslen(strings);
        m_localStrings = new (std::nothrow) wchar_t[length + 2];
        if (m_localStrings == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        result = StringCchCopyW(m_localStrings, length + 2, strings);
        if (SUCCEEDED(result))
        {
            std::size_t index = 0;
            if (length != 0)
            {
                do
                {
                    if (m_localStrings[index] == separator)
                    {
                        m_localStrings[index] = L'\0';
                    }

                    ++index;
                } while (index < length);
            }

            m_localStrings[index + 1] = L'\0';
            m_strings = strings;
        }
    }
    else
    {
        m_strings = strings;
    }

    return result;
}

extern "C" bool WINAPI IsWellFormedTag(const wchar_t* const languageTag)
{
    if (languageTag == nullptr)
    {
        return false;
    }

    Windows::Internal::CLanguage language;
    language.m_subtags.flags &= 0xFC;
    language.ValidateTag(languageTag);
    return (language.m_subtags.flags & 0x04) != 0;
}

extern "C" HRESULT WINAPI GetDistanceOfClosestLanguageInList(
    const wchar_t* const languageTag,
    const wchar_t* const languageList,
    const wchar_t separator,
    double* const score)
{
    if ((languageTag == nullptr) || (languageList == nullptr) || (score == nullptr))
    {
        return E_POINTER;
    }

    LocalStringsArray languages;
    HRESULT result = languages.Initialize(languageList, separator);
    if (SUCCEEDED(result))
    {
        const wchar_t* closestLanguage = languageTag;
        const Windows::Internal::CLanguage language(languageTag, false);
        result = language.FindClosestInList(languages.GetStrings(), {}, &closestLanguage, score);
    }
    return result;
}

extern "C" HRESULT WINAPI
FormatLanguageTag(const wchar_t* const languageTag, const int maximumLength, const wchar_t* const source, wchar_t* const result)
{
    static_cast<void>(source);
    if ((languageTag == nullptr) || (result == nullptr))
    {
        return E_POINTER;
    }

    const Windows::Internal::CLanguage language(languageTag, static_cast<bool>(languageTag));
    *result = L'\0';

    wchar_t formattedLanguage[90];
    HRESULT operationResult =
        language.GetSubtagFields(static_cast<Windows::Internal::BCP47_SUBTAG_FLAGS>(maximumLength), formattedLanguage);
    if (SUCCEEDED(operationResult))
    {
        std::uint32_t resultSize;
        operationResult = Windows::Internal::CopyResultToClientBuffer(formattedLanguage, LOCALE_NAME_MAX_LENGTH, result, &resultSize);
    }
    return operationResult;
}

int WINAPI CompareBcp47Tags(const wchar_t* const left, const wchar_t* const right)
{
    if (left == nullptr)
    {
        return -(right != nullptr);
    }
    if (right == nullptr)
    {
        return 1;
    }

    const Windows::Internal::CLanguage leftLanguage(left, true);
    const std::uint64_t leftTag = leftLanguage.m_subtags.compactTag;
    const Windows::Internal::CLanguage rightLanguage(right, true);
    const std::uint64_t rightTag = rightLanguage.m_subtags.compactTag;

    const std::uint32_t leftHigh = static_cast<std::uint32_t>(leftTag >> 32) & 0x3F;
    const std::uint32_t rightHigh = static_cast<std::uint32_t>(rightTag >> 32) & 0x3F;
    const std::uint32_t leftLanguagePart = static_cast<std::uint32_t>(leftTag) & 0xFF000000;
    const std::uint32_t rightLanguagePart = static_cast<std::uint32_t>(rightTag) & 0xFF000000;
    if (leftHigh < rightHigh)
    {
        return -1;
    }
    if (leftHigh > rightHigh)
    {
        return 1;
    }
    if (leftLanguagePart > rightLanguagePart)
    {
        return 1;
    }
    if (leftHigh > rightHigh)
    {
        return 1;
    }
    if (leftHigh < rightHigh)
    {
        return -1;
    }
    if (leftLanguagePart < rightLanguagePart)
    {
        return -1;
    }

    const std::uint32_t leftScriptPart = static_cast<std::uint32_t>(leftTag) & 0xFF0000;
    const std::uint32_t rightScriptPart = static_cast<std::uint32_t>(rightTag) & 0xFF0000;
    if (leftScriptPart > rightScriptPart)
    {
        return 1;
    }
    if (leftScriptPart < rightScriptPart)
    {
        return -1;
    }

    const std::uint32_t leftRegionPart = static_cast<std::uint32_t>(leftTag) & 0xFF80;
    const std::uint32_t rightRegionPart = static_cast<std::uint32_t>(rightTag) & 0xFF80;
    int result = 0;
    if (leftRegionPart > rightRegionPart)
    {
        result = 1;
    }
    else if (leftRegionPart < rightRegionPart)
    {
        result = -1;
    }
    if ((leftRegionPart == 0) || (rightRegionPart == 0))
    {
        return -result;
    }
    return result;
}

Windows::Internal::CLanguage::~CLanguage() { delete[] m_ownedTag; }
