#include "StdAfx.h"

#include <Bcp47.h>
#include <Language.h>

namespace Windows::Internal
{

static void WINAPI ToUpperCase(wchar_t* value, std::uint32_t length)
{
    while ((*value != L'\0') && (length != 0))
    {
        if (static_cast<std::uint16_t>(*value - L'a') <= 25)
        {
            *value &= 0xDF;
        }

        ++value;
        --length;
    }
}

static void WINAPI ToLowerCase(wchar_t* value, std::uint32_t length)
{
    while ((*value != L'\0') && (length != 0))
    {
        if (static_cast<std::uint16_t>(*value - L'A') <= 25)
        {
            *value |= 0x20;
        }

        ++value;
        --length;
    }
}

HRESULT CopyResultToClientBuffer(
    const wchar_t* const result,
    const std::uint32_t clientBufferSize,
    wchar_t* const clientBuffer,
    std::uint32_t* const resultSize)
{
    static_cast<void>(resultSize);
    const wchar_t* current = result;
    while (*current != L'\0')
    {
        ++current;
    }
    const std::uint32_t resultLength = static_cast<std::uint32_t>(current - result);
    HRESULT operationResult = resultLength < 85 ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    if ((resultLength < 85) && (clientBuffer != nullptr))
    {
        operationResult = StringCchCopyW(clientBuffer, clientBufferSize, result);
    }
    return operationResult;
}

namespace
{

constexpr std::uint8_t TagParsed = 0x01;
constexpr std::uint8_t TagValidated = 0x02;
constexpr std::uint8_t TagWellFormed = 0x04;
constexpr std::uint8_t TagValid = 0x08;
constexpr std::uint8_t TagGrandfathered = 0x10;
constexpr std::uint8_t TagPseudo = 0x20;
constexpr std::uint8_t TagPrivateUse = 0x40;
constexpr std::uint8_t TagMappedGrandfathered = 0x80;

constexpr std::uint32_t MaximumTagLength = 84;

} // namespace

CLanguage::CLanguage()
{
    m_subtags.flags = (m_subtags.flags & 0x83) | 3;
    m_useDefaults = true;
    m_subtags.languageIndex = 0;
    m_subtags.scriptIndex = 0;
    m_subtags.regionIndex = 0;
    m_subtags.variantIndex = 0;
    m_subtags.extlangIndex = 0;
    m_ownedTag = nullptr;
    m_subtags.tag = nullptr;
    m_subtags.subtagsMap = BSF_NONE;
    m_subtags.compactTag = 0;
    m_subtags.suppressedScript = false;
}

CLanguage::CLanguage(const wchar_t* const tag, const bool useDefaults)
{
    static_cast<void>(useDefaults);

    m_ownedTag = nullptr;
    m_subtags.tag = nullptr;
    m_subtags.flags &= 0xFC;
    m_useDefaults = true;
    ValidateTagAndInitialize(tag);
}

CLanguage::CLanguage(const std::uint64_t compactTag)
{
    m_ownedTag = nullptr;
    m_subtags.tag = nullptr;
    m_subtags.flags = (m_subtags.flags & 0x88) | 7;

    const auto languageIndex = static_cast<std::uint16_t>((compactTag >> 24) & 0x3FFF);
    auto scriptIndex = static_cast<std::uint16_t>((compactTag >> 16) & 0xFF);
    auto regionIndex = static_cast<std::uint16_t>((compactTag >> 7) & 0x1FF);
    const auto variantIndex = static_cast<std::uint16_t>(compactTag & 0x7F);
    auto initializedTag = compactTag;

    m_useDefaults = true;
    if ((languageIndex < 8646) && (static_cast<std::uint16_t>(scriptIndex - 1) <= 236) && (regionIndex < 342) && (variantIndex < 85))
    {
        m_subtags.flags |= TagValid;
    }
    else
    {
        m_subtags.flags &= ~TagValid;
    }

    bool suppressedScript = false;
    std::uint16_t defaultScript = 0;
    if (TryFindSuppressedScript(languageIndex, variantIndex, &defaultScript, &regionIndex, &suppressedScript, true))
    {
        if (scriptIndex == 0)
        {
            m_subtags.flags |= TagValid;
            scriptIndex = defaultScript;
            initializedTag |= static_cast<std::uint64_t>(defaultScript) << 16;
        }

        if (suppressedScript && (defaultScript != scriptIndex))
        {
            suppressedScript = false;
        }
    }

    m_subtags.compactTag = initializedTag;
    m_subtags.subtagsMap = static_cast<BCP47_SUBTAG_FLAGS>(
        (languageIndex != 0 ? BSF_LANGUAGE_FIELD : 0) | (scriptIndex != 0 ? BSF_SCRIPT_FIELD : 0) |
        (regionIndex != 0 ? BSF_DIALECT_REGION_FIELD : 0) | (variantIndex != 0 ? BSF_VARIANT_FIELD : 0));

    if (((m_subtags.flags & TagValid) != 0) && (languageIndex == 0))
    {
        m_subtags.flags &= 0xF3;
    }

    m_subtags.languageIndex = languageIndex;
    m_subtags.extlangIndex = 0;
    m_subtags.regionIndex = regionIndex;
    m_subtags.scriptIndex = scriptIndex;
    m_subtags.variantIndex = variantIndex;
    m_subtags.suppressedScript = suppressedScript;
}

HRESULT CLanguage::Initialize(const wchar_t* const tag)
{
    if (m_ownedTag != nullptr)
    {
        ::operator delete(m_ownedTag);
        m_ownedTag = nullptr;
    }

    m_subtags.tag = nullptr;
    m_subtags.flags &= 0xFC;
    return ValidateTagAndInitialize(tag) ? S_OK : E_INVALIDARG;
}

bool WINAPI CLanguage::IsValidTag(const wchar_t* const tag)
{
    if (tag == nullptr)
    {
        return false;
    }

    CLanguage language;
    language.m_subtags.flags &= 0xFC;
    return language.ValidateTag(tag);
}

bool CLanguage::ValidateTag(const wchar_t* const tag)
{
    const wchar_t* validationTag = tag;
    std::uint16_t languageIndex = 0;
    std::uint16_t scriptIndex = 0;
    std::uint16_t regionIndex = 0;
    std::uint16_t variantIndex = 0;
    std::uint16_t extlangIndex = 0;
    bool suppressedScript = false;

    m_subtags.compactTag = 0;
    if ((m_subtags.flags & TagParsed) == 0)
    {
        m_subtags.flags &= ~TagGrandfathered;
        const auto* const grandfatheredTag = static_cast<const GrandfatheredTagsTableEntry*>(
            ::bsearch(tag, grandfatheredTagsTable, 26, sizeof(GrandfatheredTagsTableEntry), CompareGrandfatheredTags));

        if (grandfatheredTag != nullptr)
        {
            m_subtags.flags |= TagMappedGrandfathered;
            if (grandfatheredTag->replacement[0] != L'\0')
            {
                validationTag = grandfatheredTag->replacement;
            }
            else
            {
                m_subtags.flags |= TagGrandfathered;
            }
        }
        else
        {
            m_subtags.flags &= ~TagMappedGrandfathered;
        }
    }

    if (ParseTag(validationTag) && ((m_subtags.flags & TagValidated) == 0))
    {
        std::uint64_t searchKey;
        const bool initialized = InitSearchKey(validationTag, 5, &searchKey);
        m_subtags.flags = (m_subtags.flags & ~TagValid) | (initialized ? TagValid : 0);
        if (initialized)
        {
            if ((m_subtags.subtagsMap & BSF_LANGUAGE_FIELD) == 0)
            {
                if (((m_subtags.flags & TagGrandfathered) == 0) &&
                    (((m_subtags.flags & TagPrivateUse) == 0) || (m_subtags.subtags[6].offset != 0)))
                {
                    m_subtags.flags &= ~TagWellFormed;
                }

                m_subtags.flags &= ~TagValid;
                goto LanguageFinished;
            }

            {
                std::uint64_t languageResult;
                if (TryFindTableEntry(searchKey, 0, SearchTableLanguages, &languageResult, nullptr))
                {
                    languageIndex = static_cast<std::uint16_t>(languageResult);
                    if (static_cast<std::uint16_t>(languageIndex - 5689) <= 519)
                    {
                        if (languageIndex == 6097)
                        {
                            if ((CompareStringOrdinal(validationTag, -1, L"qps-ploc", -1, true) == CSTR_EQUAL) ||
                                (CompareStringOrdinal(validationTag, -1, L"qps-latn-x-sh", -1, true) == CSTR_EQUAL))
                            {
                                m_subtags.flags |= TagPseudo;
                                regionIndex = 265;
                                languageIndex = 38;
                                scriptIndex = 85;
                                suppressedScript = true;
                                goto LanguageLookupFinished;
                            }

                            if (CompareStringOrdinal(validationTag, -1, L"qps-plocm", -1, true) == CSTR_EQUAL)
                            {
                                m_subtags.flags |= TagPseudo;
                                regionIndex = 221;
                                m_subtags.subtagsMap =
                                    static_cast<BCP47_SUBTAG_FLAGS>(BSF_FULL_TAG | BSF_SCRIPT_FIELD | BSF_LANGUAGE_FIELD);
                                languageIndex = 8;
                                scriptIndex = 5;
                                suppressedScript = true;
                                m_subtags.subtags[2] = {4, 5};
                            }
                            else
                            {
                                languageIndex = static_cast<std::uint16_t>(languageResult >> 32);
                                if (CompareStringOrdinal(validationTag, -1, L"qps-ploca", -1, true) == CSTR_EQUAL)
                                {
                                    m_subtags.flags |= TagPseudo;
                                    m_subtags.subtagsMap =
                                        static_cast<BCP47_SUBTAG_FLAGS>(BSF_FULL_TAG | BSF_SCRIPT_FIELD | BSF_LANGUAGE_FIELD);
                                    languageIndex = 77;
                                    scriptIndex = 68;
                                    suppressedScript = true;
                                    m_subtags.subtags[2] = {4, 5};
                                    goto LanguageLookupFinished;
                                }
                            }
                        }
                    }
                }
                else if ((m_subtags.subtags[7].length >= 3) && (CompareStringOrdinal(validationTag, 3, L"qut", 3, true) == CSTR_EQUAL))
                {
                    languageIndex = 6211;
                }
            }

        LanguageLookupFinished:
            m_subtags.flags = (m_subtags.flags & ~TagValid) | (languageIndex != 0 ? TagValid : 0);

        LanguageFinished:
            if ((m_subtags.flags & TagPseudo) != 0)
            {
                goto FinishValidation;
            }

            if ((m_subtags.subtagsMap & BSF_EXTLANG_FIELD) != 0)
            {
                std::uint64_t extlangSearchKey;
                if (InitSearchKey(&validationTag[m_subtags.subtags[1].offset], 5, &extlangSearchKey))
                {
                    std::uint64_t extlangResult;
                    const void* extlangEntry;
                    std::uint64_t extlangLanguage;
                    if (TryFindTableEntry(extlangSearchKey, 0, SearchTableExtlangLanguages, &extlangResult, &extlangEntry) &&
                        TryFindTableEntry(extlangResult, 1, SearchTableExtlangs, &extlangLanguage, nullptr) &&
                        (languageIndex == static_cast<std::uint16_t>(extlangLanguage)))
                    {
                        if (((*static_cast<const std::uint8_t*>(extlangEntry) & 1) != 0))
                        {
                            std::uint64_t replacementLanguage;
                            if (TryFindTableEntry(0, 1, SearchTableSuppressScripts, &replacementLanguage, nullptr))
                            {
                                languageIndex = static_cast<std::uint16_t>(replacementLanguage);
                            }
                            else
                            {
                                languageIndex = static_cast<std::uint16_t>(extlangResult);
                            }
                        }
                        else
                        {
                            languageIndex = static_cast<std::uint16_t>(extlangResult);
                        }

                        m_subtags.subtags[0] = m_subtags.subtags[1];
                        m_subtags.subtagsMap = static_cast<BCP47_SUBTAG_FLAGS>(m_subtags.subtagsMap & ~BSF_EXTLANG_FIELD);
                    }

                    if ((m_subtags.subtagsMap & BSF_EXTLANG_FIELD) != 0)
                    {
                        extlangIndex = static_cast<std::uint16_t>(extlangSearchKey);
                    }
                }
            }

            if ((m_subtags.subtagsMap & BSF_SCRIPT_FIELD) != 0)
            {
                std::uint64_t scriptSearchKey;
                std::uint64_t scriptResult;
                if (InitSearchKey(&validationTag[m_subtags.subtags[2].offset], 5, &scriptSearchKey) &&
                    TryFindTableEntry(scriptSearchKey, 0, SearchTableScripts, &scriptResult, nullptr))
                {
                    scriptIndex = static_cast<std::uint16_t>(scriptResult);
                }

                if (scriptIndex == 0)
                {
                    if (CompareStringOrdinal(validationTag, -1, L"ar-ploc-SA", -1, true) == CSTR_EQUAL)
                    {
                        scriptIndex = 5;
                        regionIndex = 221;
                    }
                    else if (CompareStringOrdinal(validationTag, -1, L"ja-ploc-JP", -1, true) == CSTR_EQUAL)
                    {
                        scriptIndex = 68;
                    }
                    else if (CompareStringOrdinal(validationTag, -1, L"en-locr-US", -1, true) == CSTR_EQUAL)
                    {
                        scriptIndex = 85;
                        regionIndex = 265;
                    }

                    if (scriptIndex != 0)
                    {
                        m_subtags.flags |= TagPseudo | TagValid;
                        suppressedScript = true;
                    }
                }

                if ((m_subtags.flags & TagValid) != 0)
                {
                    m_subtags.flags = (m_subtags.flags & ~TagValid) | (scriptIndex != 0 ? TagValid : 0);
                }
            }

            if ((m_subtags.flags & TagPseudo) != 0)
            {
                goto FinishValidation;
            }

            if ((m_subtags.subtagsMap & BSF_DIALECT_REGION_FIELD) != 0)
            {
                std::uint64_t regionSearchKey;
                std::uint64_t regionResult;
                const void* regionEntry;
                if (InitSearchKey(&validationTag[m_subtags.subtags[3].offset], 3, &regionSearchKey) &&
                    TryFindTableEntry(regionSearchKey, 0, SearchTableRegions, &regionResult, &regionEntry))
                {
                    regionIndex = static_cast<std::uint16_t>(regionResult);
                    if ((*static_cast<const std::uint8_t*>(regionEntry) & 1) != 0)
                    {
                        std::uint64_t replacementRegion;
                        if (TryFindTableEntry(regionIndex, 0, SearchTableRegionReplacements, &replacementRegion, nullptr))
                        {
                            regionIndex = static_cast<std::uint16_t>(replacementRegion);
                        }
                    }
                }

                if ((m_subtags.flags & TagValid) != 0)
                {
                    m_subtags.flags = (m_subtags.flags & ~TagValid) | (regionIndex != 0 ? TagValid : 0);
                }
            }

            if ((m_subtags.subtagsMap & BSF_VARIANT_FIELD) != 0)
            {
                if (languageIndex == 23)
                {
                    if (CompareStringOrdinal(&validationTag[m_subtags.subtags[4].offset], -1, L"valencia", -1, true) == CSTR_EQUAL)
                    {
                        variantIndex = 81;
                    }
                }
                else if (
                    (languageIndex == 14) &&
                    (CompareStringOrdinal(&validationTag[m_subtags.subtags[4].offset], -1, L"tarask", -1, true) == CSTR_EQUAL))
                {
                    variantIndex = 75;
                }
            }

            if ((m_subtags.flags & TagValid) != 0)
            {
                if (TryFindSuppressedScript(languageIndex, variantIndex, &scriptIndex, &regionIndex, &suppressedScript, m_useDefaults))
                {
                    if ((m_subtags.subtagsMap & BSF_SCRIPT_FIELD) == 0)
                    {
                        m_subtags.subtagsMap = static_cast<BCP47_SUBTAG_FLAGS>(m_subtags.subtagsMap | BSF_SCRIPT_FIELD);
                        m_subtags.subtags[2].length = 0;
                    }
                }

                m_subtags.flags = (m_subtags.flags & ~TagValid) | (scriptIndex != 0 ? TagValid : 0);
            }

            if ((m_subtags.flags & TagGrandfathered) != 0)
            {
                m_subtags.flags = (m_subtags.flags & ~TagValid) | TagWellFormed | (scriptIndex != 0 ? TagValid : 0);
            }

        FinishValidation:
            m_subtags.flags |= TagValidated;
        }
    }

    m_subtags.extlangIndex = extlangIndex;
    m_subtags.scriptIndex = scriptIndex;
    m_subtags.variantIndex = variantIndex;
    m_subtags.suppressedScript = suppressedScript;
    m_subtags.languageIndex = languageIndex;
    m_subtags.regionIndex = regionIndex;
    m_subtags.compactTag =
        static_cast<std::uint16_t>(variantIndex) +
        (((((static_cast<std::uint64_t>(languageIndex) << 8) + static_cast<std::uint16_t>(scriptIndex)) << 9) + regionIndex) << 7);
    return (m_subtags.flags & TagValid) != 0;
}

bool CLanguage::ValidateTagAndInitialize(const wchar_t* const tag)
{
    bool valid = ValidateTag(tag);
    if (((m_subtags.flags & 0x44) != 0) &&
        (((m_subtags.flags & 0x70) != 0) || ((m_subtags.subtagsMap & 0x70) != 0) ||
         (((m_subtags.flags & TagValid) == 0) &&
          ((m_subtags.languageIndex == 0) ||
           ((m_subtags.languageIndex == 6211) && (CompareStringOrdinal(tag, -1, L"qut", -1, true) == CSTR_EQUAL)) ||
           ((m_subtags.extlangIndex == 0) && ((m_subtags.subtagsMap & BSF_EXTLANG_FIELD) != 0)) ||
           ((m_subtags.scriptIndex == 0) && ((m_subtags.subtagsMap & BSF_SCRIPT_FIELD) != 0)) ||
           ((m_subtags.regionIndex == 0) && ((m_subtags.subtagsMap & BSF_DIALECT_REGION_FIELD) != 0))))))
    {
        std::size_t tagLength;
        if ((tag != m_ownedTag) && SUCCEEDED(StringCchLengthW(tag, MaximumTagLength + 1, &tagLength)))
        {
            ++tagLength;
            auto* const copiedTag = static_cast<wchar_t*>(::operator new[](sizeof(wchar_t) * tagLength, std::nothrow));
            if (copiedTag != nullptr)
            {
                if (SUCCEEDED(StringCchCopyW(copiedTag, tagLength, tag)))
                {
                    if ((m_subtags.subtagsMap & BSF_LANGUAGE_FIELD) != 0)
                    {
                        ToLowerCase(&copiedTag[m_subtags.subtags[0].offset], m_subtags.subtags[0].length);
                    }

                    if (((m_subtags.subtagsMap & BSF_SCRIPT_FIELD) != 0) && (m_subtags.subtags[2].length == 4) &&
                        ((m_subtags.flags & TagPseudo) == 0))
                    {
                        ToLowerCase(&copiedTag[m_subtags.subtags[2].offset], m_subtags.subtags[2].length);
                        ToUpperCase(&copiedTag[m_subtags.subtags[2].offset], 1);
                    }

                    if ((m_subtags.subtagsMap & BSF_DIALECT_REGION_FIELD) != 0)
                    {
                        ToUpperCase(&copiedTag[m_subtags.subtags[3].offset], m_subtags.subtags[3].length);
                    }

                    if ((m_subtags.subtagsMap & BSF_VARIANT_FIELD) != 0)
                    {
                        ToLowerCase(&copiedTag[m_subtags.subtags[4].offset], m_subtags.subtags[4].length);
                    }

                    if ((m_subtags.subtagsMap & BSF_EXTENSION_FIELD) != 0)
                    {
                        ToLowerCase(&copiedTag[m_subtags.subtags[5].offset], m_subtags.subtags[5].length);
                    }

                    if ((m_subtags.subtagsMap & BSF_PRIVATE_USE_FIELD) != 0)
                    {
                        ToLowerCase(&copiedTag[m_subtags.subtags[6].offset], m_subtags.subtags[6].length);
                    }

                    ::operator delete(m_ownedTag);
                    m_ownedTag = copiedTag;
                    goto CopyFinished;
                }

                ::operator delete(copiedTag);
            }

            valid = false;
        }

    CopyFinished:
        m_subtags.tag = m_ownedTag;
    }

    return valid;
}

HRESULT CLanguage::GetSubtagFields(const BCP47_SUBTAG_FLAGS flags, wchar_t* const result) const
{
    if (result == nullptr)
    {
        return E_POINTER;
    }

    auto requestedFlags = static_cast<std::uint32_t>(flags);
    *result = L'\0';

    if ((requestedFlags & BSF_SYSTEM_MUI_FORM) != 0)
    {
        if ((m_subtags.flags & TagPseudo) == 0)
        {
            std::uint64_t canonicalTag = 0;
            if (!TryFindTableEntry(m_subtags.compactTag, 0, SearchTableCanonicalVariants, &canonicalTag, nullptr) ||
                (canonicalTag != m_subtags.compactTag))
            {
                const auto languageAndScript =
                    m_subtags.compactTag & ((static_cast<std::uint64_t>(0x3FFF) << 24) | (static_cast<std::uint64_t>(0xFF) << 16));
                const auto language = m_subtags.compactTag & (static_cast<std::uint64_t>(0x3FFF) << 24);
                if ((((m_subtags.compactTag >> 16) & 0xFF) == 0) ||
                    !TryFindTableEntry(languageAndScript, 0, SearchTableCanonicalScripts, &canonicalTag, nullptr))
                {
                    if (!TryFindTableEntry(language, 0, SearchTableCanonicalLanguages, &canonicalTag, nullptr))
                    {
                        canonicalTag = 0;
                    }
                }
            }

            if (canonicalTag != 0)
            {
                CLanguage language(canonicalTag);
                const auto canonicalLanguage = canonicalTag & (static_cast<std::uint64_t>(0x3FFF) << 24);
                const auto canonicalFlags = TryFindTableEntry(canonicalLanguage, 0, SearchTableCanonicalLanguageRegions, nullptr, nullptr) ?
                                                BSF_LANGNAME_FORM :
                                                BSF_COMMON_FORM;
                return language.GetSubtagFields(canonicalFlags, result);
            }
        }

        requestedFlags = (requestedFlags ^ BSF_SYSTEM_MUI_FORM) | BSF_COMMON_FORM;
    }

    if ((requestedFlags & BSF_FULL_TAG) != 0)
    {
        requestedFlags = (requestedFlags ^ BSF_FULL_TAG) | BSF_COMMON_FORM;
    }
    else if (requestedFlags == BSF_SCRIPT_FIELD)
    {
        requestedFlags = BSF_FORCE_SCRIPT_FIELD;
    }

    HRESULT status = (m_subtags.flags & TagWellFormed) != 0 ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    wchar_t* destination = result;
    std::uint32_t remaining = MaximumTagLength;
    bool firstSubtag = true;
    if (FAILED(status))
    {
        *destination = L'\0';
        return status;
    }

    for (std::uint32_t subtagIndex = 0; subtagIndex < 8; ++subtagIndex)
    {
        const auto subtagFlag = 1U << subtagIndex;
        if ((requestedFlags & subtagFlag) == 0)
        {
            continue;
        }

        bool copyStoredSubtag = false;
        if (subtagIndex == 0)
        {
            if (m_subtags.tag != nullptr)
            {
                copyStoredSubtag = true;
            }
        }
        else if (subtagIndex == 1)
        {
            if (m_subtags.tag != nullptr)
            {
                copyStoredSubtag = true;
            }
            else if ((m_subtags.subtagsMap & BSF_EXTLANG_FIELD) == 0)
            {
                continue;
            }
        }
        else if (subtagIndex == 2)
        {
            if ((m_subtags.tag != nullptr) && (m_subtags.subtags[2].length != 0) &&
                (((m_subtags.flags & TagPseudo) == 0) || (requestedFlags != BSF_FORCE_SCRIPT_FIELD)))
            {
                copyStoredSubtag = true;
            }
            else if (
                ((m_subtags.subtagsMap & BSF_SCRIPT_FIELD) != 0) || ((requestedFlags & BSF_FORCE_SCRIPT_FIELD) == BSF_FORCE_SCRIPT_FIELD))
            {
                auto scriptIndex = static_cast<std::uint16_t>((m_subtags.compactTag >> 16) & 0xFF);
                if ((scriptIndex < 238) &&
                    (!m_subtags.suppressedScript || ((requestedFlags & BSF_FORCE_SCRIPT_FIELD) == BSF_FORCE_SCRIPT_FIELD)))
                {
                    if (remaining < static_cast<std::uint32_t>(!firstSubtag) + 4)
                    {
                        status = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                        break;
                    }

                    if (scriptIndex == 0)
                    {
                        scriptIndex = 236;
                    }

                    const std::uint32_t packedScript = scriptTags[scriptIndex - 1];
                    if (packedScript != 0)
                    {
                        if (firstSubtag)
                        {
                            firstSubtag = false;
                        }
                        else
                        {
                            *destination++ = L'-';
                            --remaining;
                        }

                        std::uint32_t characterCount = 4;
                        std::uint32_t shift = 20;
                        while (shift != 0)
                        {
                            shift -= 5;
                            const auto packedCharacter = (packedScript >> shift) & 0x1F;
                            const wchar_t character = static_cast<wchar_t>((characterCount == 4 ? L'A' : L'a') - 1 + packedCharacter);
                            if (character == L'\0')
                            {
                                status = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                                break;
                            }

                            *destination++ = character;
                            --characterCount;
                            --remaining;
                        }

                        if (FAILED(status))
                        {
                            break;
                        }
                    }
                }
            }
        }
        else if (subtagIndex == 3)
        {
            if (m_subtags.tag != nullptr)
            {
                copyStoredSubtag = true;
            }
            else
            {
                const auto regionIndex = static_cast<std::uint16_t>((m_subtags.compactTag >> 7) & 0x1FF);
                if (static_cast<std::uint16_t>(regionIndex - 1) <= 340)
                {
                    const auto packedRegion = static_cast<std::uint16_t>(regionTags[regionIndex - 1] >> 1);
                    if (packedRegion != 0)
                    {
                        const bool alphabetical = (packedRegion & 0x1000) == 0;
                        auto characterCount = static_cast<std::uint32_t>(alphabetical ? 2 : 3);
                        if (remaining < static_cast<std::uint32_t>(!firstSubtag) + characterCount)
                        {
                            status = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                            break;
                        }

                        if (firstSubtag)
                        {
                            firstSubtag = false;
                        }
                        else
                        {
                            *destination++ = L'-';
                            --remaining;
                        }

                        while (characterCount != 0)
                        {
                            const auto shift = (characterCount - 1) * (alphabetical ? 5 : 4);
                            const auto packedCharacter = (packedRegion >> shift) & (alphabetical ? 0x1F : 0x0F);
                            const wchar_t character = static_cast<wchar_t>((alphabetical ? L'A' : L'0') - 1 + packedCharacter);
                            if (character == L'\0')
                            {
                                status = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                                break;
                            }

                            *destination++ = character;
                            --characterCount;
                            --remaining;
                        }

                        if (FAILED(status))
                        {
                            break;
                        }
                    }
                }
            }
        }
        else if ((subtagIndex == 4) && (m_subtags.tag == nullptr))
        {
            const char* variant = nullptr;
            std::uint32_t variantLength = 0;
            if (m_subtags.variantIndex == 81)
            {
                variant = "valencia";
                variantLength = 8;
            }
            else if (m_subtags.variantIndex == 75)
            {
                variant = "tarask";
                variantLength = 6;
            }

            if (variant != nullptr)
            {
                if (firstSubtag)
                {
                    firstSubtag = false;
                }
                else
                {
                    *destination++ = L'-';
                    --remaining;
                }

                while ((remaining != 0) && (variantLength != 0) && (*variant != '\0'))
                {
                    *destination++ = static_cast<std::uint8_t>(*variant++);
                    --remaining;
                    --variantLength;
                }

                if ((variantLength != 0) || (*variant != '\0'))
                {
                    status = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    break;
                }

                requestedFlags ^= BSF_VARIANT_FIELD;
            }
            else
            {
                copyStoredSubtag = true;
            }
        }
        else
        {
            copyStoredSubtag = true;
        }

        if ((subtagIndex <= 1) && !copyStoredSubtag)
        {
            const auto languageIndex = static_cast<std::uint32_t>((m_subtags.compactTag >> 24) & 0x3FFF);
            if ((languageIndex - 1) <= 8644)
            {
                const auto packedLanguage = static_cast<std::uint16_t>(
                    (firstSubtag ? langTags[languageIndex - 1] : m_subtags.extlangIndex) >> (firstSubtag ? 1 : 0));
                if (packedLanguage != 0)
                {
                    if (!firstSubtag)
                    {
                        *destination++ = L'-';
                        --remaining;
                    }

                    auto characterCount = static_cast<std::uint32_t>(packedLanguage > 0x3FF ? 3 : 2);
                    while ((remaining != 0) && (characterCount != 0))
                    {
                        *destination++ = static_cast<wchar_t>(((packedLanguage >> (5 * --characterCount)) & 0x1F) + L'`');
                        --remaining;
                    }

                    firstSubtag = false;
                }
            }
        }
        else if (copyStoredSubtag && (m_subtags.tag != nullptr))
        {
            Bcp47SubtagField field;
            const bool wholeTag = firstSubtag && ((m_subtags.flags & TagGrandfathered) != 0) &&
                                  ((requestedFlags & BSF_NORMALIZED_BASE_FORM) == BSF_NORMALIZED_BASE_FORM);
            if (wholeTag)
            {
                field = m_subtags.subtags[7];
            }
            else
            {
                if ((m_subtags.subtagsMap & subtagFlag) == 0)
                {
                    continue;
                }

                field = m_subtags.subtags[subtagIndex];
            }

            std::uint32_t sourceOffset;
            std::uint32_t copyLength;
            if (firstSubtag)
            {
                sourceOffset = field.offset;
                copyLength = field.length;
            }
            else
            {
                if (field.offset == 0)
                {
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }

                sourceOffset = field.offset - 1;
                copyLength = field.length + 1;
            }

            if ((copyLength < remaining) && SUCCEEDED(StringCchCopyNW(destination, remaining, &m_subtags.tag[sourceOffset], copyLength)))
            {
                remaining -= copyLength;
                destination += copyLength;
                firstSubtag = false;
            }

            if (wholeTag)
            {
                break;
            }
        }
    }

    *destination = L'\0';
    return status;
}

HRESULT CLanguage::Compare(const CLanguage& other, double* const score) const
{
    if (score == nullptr)
    {
        return E_POINTER;
    }

    *score = 0.0;
    if (((m_subtags.flags & TagValid) == 0) || ((other.m_subtags.flags & TagValid) == 0))
    {
        return HRESULT_FROM_WIN32(ERROR_NO_MATCH);
    }

    int comparison = BCM_NO_MATCH;
    if ((m_subtags.compactTag & (static_cast<std::uint64_t>(0x3FFF) << 24)) ==
        (other.m_subtags.compactTag & (static_cast<std::uint64_t>(0x3FFF) << 24)))
    {
        comparison = BCM_LANGUAGES_MATCH;
    }

    if ((m_subtags.compactTag & 0xFF0000) == (other.m_subtags.compactTag & 0xFF0000))
    {
        comparison = comparison != BCM_NO_MATCH ? BCM_LANGUAGES_SCRIPTS_MATCH : BCM_SCRIPTS_MATCH;
    }

    if ((m_subtags.compactTag & 0xFF80) != (other.m_subtags.compactTag & 0xFF80))
    {
        if (comparison == BCM_LANGUAGES_SCRIPTS_MATCH)
        {
            CheckLanguageRegionAffinity(other, &comparison);
        }
    }
    else
    {
        if (comparison == BCM_NO_MATCH)
        {
            comparison = BCM_REGIONS_MATCH;
        }
        else if (comparison == BCM_LANGUAGES_SCRIPTS_MATCH)
        {
            comparison = BCM_LANGUAGES_SCRIPTS_REGIONS_MATCH;
        }

        if ((comparison == BCM_LANGUAGES_SCRIPTS_REGIONS_MATCH) && ((m_subtags.compactTag & 0x7F) == (other.m_subtags.compactTag & 0x7F)))
        {
            if ((m_subtags.subtagsMap & BSF_VARIANT_FIELD) != 0)
            {
                comparison = BCM_VARIANTS_MATCH;
            }

            if ((m_ownedTag != nullptr) && (other.m_ownedTag != nullptr))
            {
                if (CompareStringOrdinal(m_ownedTag, -1, other.m_ownedTag, -1, true) == CSTR_EQUAL)
                {
                    comparison = BCM_EXACT_MATCH;
                }
            }
            else if (m_ownedTag == other.m_ownedTag)
            {
                comparison = BCM_EXACT_MATCH;
            }
        }
    }

    const bool thisPseudo = (m_subtags.flags & TagPseudo) != 0;
    const bool otherPseudo = (other.m_subtags.flags & TagPseudo) != 0;
    if (thisPseudo != otherPseudo)
    {
        if (comparison < BCM_LANGUAGES_SCRIPTS_MATCH)
        {
            if (comparison == BCM_NO_MATCH)
            {
                goto NoMatch;
            }

            comparison = BCM_REGIONS_MATCH;
        }
        else
        {
            comparison = BCM_SCRIPTS_MATCH;
        }
    }
    else if (thisPseudo)
    {
        const int stringComparison = CompareStringOrdinal(m_ownedTag, -1, other.m_ownedTag, -1, true);
        if (stringComparison == 0)
        {
            comparison = BCM_REGIONS_MATCH;
        }
        else if (stringComparison != CSTR_EQUAL)
        {
            if (comparison >= BCM_LANGUAGES_SCRIPTS_MATCH)
            {
                comparison = BCM_SCRIPTS_MATCH;
            }
            else if (comparison == BCM_NO_MATCH)
            {
                goto NoMatch;
            }
            else
            {
                comparison = BCM_REGIONS_MATCH;
            }
        }

        if (comparison == BCM_NO_MATCH)
        {
            goto NoMatch;
        }
    }
    else if (comparison == BCM_NO_MATCH)
    {
        goto NoMatch;
    }

    *score = static_cast<double>(comparison) / 100.0;
    if (*score > 0.65)
    {
        return S_OK;
    }

    {
        const bool norwegian =
            (((m_subtags.compactTag & 0xFF000000) == 0x6E000000) && (((m_subtags.compactTag >> 32) & 0x3F) == 0x1D)) ||
            (((other.m_subtags.compactTag & 0xFF000000) == 0x6E000000) && (((other.m_subtags.compactTag >> 32) & 0x3F) == 0x1D));
        if (!norwegian)
        {
            return S_OK;
        }

        if (*score == 0.4)
        {
            *score = 0.65;
        }
        else
        {
            const auto thisScript = static_cast<std::uint8_t>(m_subtags.compactTag >> 16);
            const auto otherScript = static_cast<std::uint8_t>(other.m_subtags.compactTag >> 16);
            if ((((thisScript != 0) && (thisScript != 236)) || (*score >= 0.6)) &&
                (((otherScript != 0) && (otherScript != 236)) || (*score >= 0.6)))
            {
                return S_OK;
            }

            *score = 0.6;
        }
    }

    return S_OK;

NoMatch:
    if ((((m_subtags.compactTag & 0xFF000000) == 0x6E000000) && (((m_subtags.compactTag >> 32) & 0x3F) == 0x1D)) ||
        (((other.m_subtags.compactTag & 0xFF000000) == 0x6E000000) && (((other.m_subtags.compactTag >> 32) & 0x3F) == 0x1D)))
    {
        const auto thisScript = static_cast<std::uint8_t>(m_subtags.compactTag >> 16);
        const auto otherScript = static_cast<std::uint8_t>(other.m_subtags.compactTag >> 16);
        if (!(((thisScript != 0) && (thisScript != 236)) || (*score >= 0.6)) ||
            !(((otherScript != 0) && (otherScript != 236)) || (*score >= 0.6)))
        {
            *score = 0.6;
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_NO_MATCH);
}

HRESULT CLanguage::CheckLanguageRegionAffinity(const CLanguage& other, int* const score) const
{
    HRESULT status = S_OK;
    int comparison = *score;
    if (comparison < BCM_LANGUAGES_SCRIPTS_MATCH)
    {
        *score = comparison;
        return status;
    }

    const auto thisRegion = static_cast<std::uint16_t>((m_subtags.compactTag >> 7) & 0x1FF);
    const auto otherRegion = static_cast<std::uint16_t>((other.m_subtags.compactTag >> 7) & 0x1FF);
    if (thisRegion == otherRegion)
    {
        comparison = BCM_LANGUAGES_SCRIPTS_REGIONS_MATCH;
    }
    else if ((thisRegion == 0) || (otherRegion == 0) || (thisRegion == 312) || (otherRegion == 312))
    {
        comparison = BCM_LANGUAGES_SCRIPTS_NO_REGION_MATCH;
    }

    if ((thisRegion > 312) || (otherRegion > 312))
    {
        wchar_t thisRegionTag[90];
        wchar_t otherRegionTag[88];
        status = GetSubtagFields(BSF_DIALECT_REGION_FIELD, thisRegionTag);
        if (SUCCEEDED(status))
        {
            status = other.GetSubtagFields(BSF_DIALECT_REGION_FIELD, otherRegionTag);
            if (SUCCEEDED(status))
            {
                CRegion firstRegion(thisRegionTag);
                CRegion secondRegion(otherRegionTag);
                double regionScore;
                if (SUCCEEDED(firstRegion.Compare(secondRegion, &regionScore)))
                {
                    if ((regionScore != 0.0) || SUCCEEDED(secondRegion.Compare(firstRegion, &regionScore)))
                    {
                        if (regionScore > 0.2)
                        {
                            if (regionScore <= 0.4)
                            {
                                comparison = BCM_LANGUAGES_SCRIPTS_REGION_WEAK_AFFINITY_MATCH;
                            }
                            else if (regionScore <= 0.7)
                            {
                                comparison = BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_AFFINITY_MATCH;
                            }
                            else
                            {
                                comparison = BCM_LANGUAGES_SCRIPTS_REGION_STRONG_AFFINITY_MATCH;
                            }

                            *score = comparison;
                            return status;
                        }
                    }
                }
            }
        }
    }

    if ((comparison < BCM_LANGUAGES_SCRIPTS_NO_REGION_MATCH) && ((m_subtags.compactTag & 0xFF000000) == 0x26000000) &&
        (((m_subtags.compactTag >> 32) & 0x3F) == 0))
    {
        if (thisRegion == 265)
        {
            if ((otherRegion == 191) || (otherRegion == 144))
            {
                comparison = BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_HEURISTIC_AFFINITY_MATCH;
            }
            else
            {
                comparison = BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH;
            }
        }
        else if (otherRegion == 265)
        {
            if ((thisRegion == 191) || (thisRegion == 144))
            {
                comparison = BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_HEURISTIC_AFFINITY_MATCH;
            }
            else
            {
                comparison = BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH;
            }
        }
        else if (thisRegion == 89)
        {
            comparison = ((otherRegion == 191) || (otherRegion == 144)) ? BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH :
                                                                          BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_HEURISTIC_AFFINITY_MATCH;
        }
        else if (otherRegion == 89)
        {
            comparison = ((thisRegion == 191) || (thisRegion == 144)) ? BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH :
                                                                        BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_HEURISTIC_AFFINITY_MATCH;
        }
    }

    if (comparison < BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH)
    {
        wchar_t firstTag[90];
        wchar_t secondTag[88];
        if (SUCCEEDED(GetSubtagFields(BSF_COMMON_FORM, firstTag)) && SUCCEEDED(other.GetSubtagFields(BSF_SYSTEM_MUI_FORM, secondTag)) &&
            (CompareStringOrdinal(firstTag, -1, secondTag, -1, true) == CSTR_EQUAL))
        {
            comparison = BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH;
        }

        status = GetSubtagFields(BSF_SYSTEM_MUI_FORM, firstTag);
        if (SUCCEEDED(status))
        {
            status = other.GetSubtagFields(BSF_COMMON_FORM, secondTag);
            if (SUCCEEDED(status) && (CompareStringOrdinal(firstTag, -1, secondTag, -1, true) == CSTR_EQUAL))
            {
                comparison = BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH;
            }
        }
    }

    *score = comparison;
    return status;
}

HRESULT CLanguage::TryFindFirstInList(const wchar_t* list, const BCP47_CLOSENESS_MEASURE closenessMeasure, const wchar_t** const result)
    const
{
    HRESULT status = S_OK;
    CLanguage language;
    *result = nullptr;

    while (*list != L'\0')
    {
        double comparison = 0.0;
        language.Initialize(list);

        std::size_t length;
        if (FAILED(StringCchLengthW(list, MaximumTagLength + 1, &length)))
        {
            status = E_INVALIDARG;
            break;
        }

        list += length + 1;
        if (SUCCEEDED(Compare(language, &comparison)) && (comparison >= static_cast<double>(closenessMeasure) / 100.0))
        {
            *result = list;
            break;
        }
    }

    return status;
}

HRESULT CLanguage::FindClosestInList(
    const wchar_t* const list,
    const BCP47_COMPARISON_ALGORITHM comparisonAlgorithm,
    const wchar_t** const result,
    double* const score) const
{
    static_cast<void>(comparisonAlgorithm);

    if ((list == nullptr) || (result == nullptr))
    {
        return E_POINTER;
    }

    HRESULT status = HRESULT_FROM_WIN32(ERROR_NO_MATCH);
    CLanguage language;
    const wchar_t* current = list;
    const wchar_t* closest = nullptr;
    std::uint32_t currentIndex = 0;
    std::uint32_t closestIndex = 0;
    double closestScore = 0.0;

    const bool norwegian = ((m_subtags.compactTag & 0xFF000000) == 0x6E000000) && (((m_subtags.compactTag >> 32) & 0x3F) == 0x1D);
    const auto norwegianScript = norwegian ? static_cast<std::uint8_t>(m_subtags.compactTag >> 16) : 0;

    if (score != nullptr)
    {
        *score = 0.0;
    }

    if (*current == L'\0')
    {
        return status;
    }

    while (true)
    {
        std::size_t length;
        if (FAILED(StringCchLengthW(current, MaximumTagLength + 1, &length)))
        {
            status = E_INVALIDARG;
            break;
        }

        double comparison = 0.0;
        language.Initialize(current);
        if (SUCCEEDED(Compare(language, &comparison)) || norwegian)
        {
            if (norwegian)
            {
                if (comparison == 0.4)
                {
                    comparison = 0.65;
                }
                else if (((norwegianScript == 0) || (norwegianScript == 236)) && (comparison < 0.6))
                {
                    comparison = 0.6;
                }
            }

            if (comparison > closestScore)
            {
                closestIndex = currentIndex;
                closest = current;
                closestScore = comparison;
                if ((comparison >= 0.8) || norwegian)
                {
                    status = S_OK;
                    break;
                }

                if (comparison >= 0.75)
                {
                    int closeness = 75;
                    if (comparison > 0.75)
                    {
                        if (comparison < 0.78)
                        {
                            closeness = 76 + (comparison >= 0.77 ? 1 : 0);
                        }
                        else
                        {
                            closeness = 78;
                        }
                    }

                    const wchar_t* laterMatch = nullptr;
                    status = TryFindFirstInList(current + length + 1, static_cast<BCP47_CLOSENESS_MEASURE>(closeness), &laterMatch);
                    if (laterMatch != nullptr)
                    {
                        if (FAILED(status))
                        {
                            break;
                        }

                        closestIndex = 0;
                        closestScore = 0.0;
                        closest = nullptr;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        current += length + 1;
        ++currentIndex;
        if (*current == L'\0')
        {
            if (FAILED(status))
            {
                break;
            }

            *result = closest;
            if ((score != nullptr) && (closestScore >= 0.5))
            {
                if (closestIndex < 7)
                {
                    *score = (closestScore / 10.0) + (static_cast<double>(9 - closestIndex) * 0.1);
                }
                else if (closestIndex < 12)
                {
                    *score = (closestScore / 20.0) + (static_cast<double>(12 - closestIndex) * 0.05);
                }
                else
                {
                    *score = closestScore / 20.0;
                }
            }

            return status;
        }
    }

    if (SUCCEEDED(status))
    {
        *result = closest;
        if ((score != nullptr) && (closestScore >= 0.5))
        {
            if (closestIndex < 7)
            {
                *score = (closestScore / 10.0) + (static_cast<double>(9 - closestIndex) * 0.1);
            }
            else if (closestIndex < 12)
            {
                *score = (closestScore / 20.0) + (static_cast<double>(12 - closestIndex) * 0.05);
            }
            else
            {
                *score = closestScore / 20.0;
            }
        }
    }

    return status;
}

} // namespace Windows::Internal
