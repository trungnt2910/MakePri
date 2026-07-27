#pragma once

#include <windows.h>

#include <cstdint>

int WINAPI CompareBcp47Tags(const wchar_t* left, const wchar_t* right);
extern "C" bool WINAPI IsWellFormedTag(const wchar_t* languageTag);
extern "C" HRESULT WINAPI
GetDistanceOfClosestLanguageInList(const wchar_t* language, const wchar_t* languageList, wchar_t separator, double* score);

namespace Windows::Internal
{

HRESULT CopyResultToClientBuffer(const wchar_t* result, std::uint32_t clientBufferSize, wchar_t* clientBuffer, std::uint32_t* resultSize);

enum BCP47_SUBTAG_FLAGS : std::uint32_t
{
    BSF_LANGUAGE_FIELD = 1,
    BSF_EXTLANG_FIELD = 2,
    BSF_SCRIPT_FIELD = 4,
    BSF_DIALECT_REGION_FIELD = 8,
    BSF_VARIANT_FIELD = 16,
    BSF_EXTENSION_FIELD = 32,
    BSF_PRIVATE_USE_FIELD = 64,
    BSF_FULL_TAG = 128,
    BSF_NLS_FORM = 512,
    BSF_SYSTEM_MUI_FORM = 1024,
    BSF_FORCE_SCRIPT_FIELD = 260,
    BSF_NONE = 0,
    BSF_COMMON_FORM = 127,
    BSF_ALL_SUBTAGS = 383,
    BSF_COMPARISON_FORM = 263,
    BSF_LANGNAME_FORM = 9,
    BSF_NEUTRAL_FORM = 7,
    BSF_NORMALIZED_BASE_FORM = 31,
};

enum BCP47_CLOSENESS_MEASURE : std::int32_t
{
    BCM_NO_MATCH = 0,
    BCM_REGIONS_MATCH = 10,
    BCM_SCRIPTS_MATCH = 40,
    BCM_LANGUAGES_MATCH = 50,
    BCM_ANY_MATCH = 60,
    BCM_ANY_MATCH_WITH_SCRIPT = 65,
    BCM_LANGUAGES_SCRIPTS_MATCH = 75,
    BCM_LANGUAGES_SCRIPTS_REGION_WEAK_HEURISTIC_AFFINITY_MATCH = 76,
    BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_HEURISTIC_AFFINITY_MATCH = 77,
    BCM_LANGUAGES_SCRIPTS_REGION_STRONG_HEURISTIC_AFFINITY_MATCH = 78,
    BCM_LANGUAGES_SCRIPTS_NO_REGION_MATCH = 80,
    BCM_LANGUAGES_SCRIPTS_REGION_WEAK_AFFINITY_MATCH = 84,
    BCM_LANGUAGES_SCRIPTS_REGION_MODERATE_AFFINITY_MATCH = 86,
    BCM_LANGUAGES_SCRIPTS_REGION_STRONG_AFFINITY_MATCH = 88,
    BCM_LANGUAGES_SCRIPTS_REGIONS_MATCH = 90,
    BCM_VARIANTS_MATCH = 97,
    BCM_EXACT_MATCH = 100,
    BCM_MAXIMUM_SCORE = 100,
};
enum BCP47_COMPARISON_ALGORITHM : std::int32_t;

struct Bcp47SubtagField
{
    std::uint8_t offset;
    std::uint8_t length;
};

struct Bcp47TagSubtagsInfo
{
    const wchar_t* tag;
    BCP47_SUBTAG_FLAGS subtagsMap;
    Bcp47SubtagField subtags[8];
    std::uint16_t languageIndex;
    std::uint16_t scriptIndex;
    std::uint16_t regionIndex;
    std::uint16_t variantIndex;
    std::uint16_t extlangIndex;
    bool suppressedScript;
    std::uint8_t padding[5];
    std::uint64_t compactTag;
    std::uint8_t flags;
};

class CLanguage
{
public:
    CLanguage();
    CLanguage(const wchar_t* tag, bool useDefaults);
    explicit CLanguage(std::uint64_t compactTag);
    virtual ~CLanguage();

    static bool WINAPI IsValidTag(const wchar_t* tag);

    HRESULT GetSubtagFields(BCP47_SUBTAG_FLAGS flags, wchar_t* result) const;
    HRESULT Compare(const CLanguage& other, double* score) const;
    HRESULT CheckLanguageRegionAffinity(const CLanguage& other, int* score) const;
    HRESULT TryFindFirstInList(const wchar_t* list, BCP47_CLOSENESS_MEASURE closenessMeasure, const wchar_t** result) const;
    HRESULT FindClosestInList(const wchar_t* list, BCP47_COMPARISON_ALGORITHM comparisonAlgorithm, const wchar_t** result, double* score)
        const;

protected:
    HRESULT Initialize(const wchar_t* tag);
    bool ValidateTag(const wchar_t* tag);
    bool ValidateTagAndInitialize(const wchar_t* tag);
    bool ParseTag(const wchar_t* tag);

private:
    friend int WINAPI ::CompareBcp47Tags(const wchar_t* left, const wchar_t* right);
    friend bool WINAPI ::IsWellFormedTag(const wchar_t* languageTag);

    std::uint32_t m_unknown;
    bool m_useDefaults;
    std::uint8_t m_padding[3];
    wchar_t* m_ownedTag;
    Bcp47TagSubtagsInfo m_subtags;
};

} // namespace Windows::Internal
