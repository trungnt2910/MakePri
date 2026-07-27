#pragma once

#include <cstdint>

enum SEARCH_TABLE_TYPE : std::int32_t
{
    SearchTableSuppressScriptsCompact = 0,
    SearchTableSuppressScripts = 1,
    SearchTableExtlangs = 2,
    SearchTableLanguages = 3,
    SearchTableExtlangLanguages = 4,
    SearchTableScripts = 5,
    SearchTableRegions = 6,
    SearchTableCanonicalVariants = 9,
    SearchTableCanonicalScripts = 10,
    SearchTableCanonicalLanguages = 11,
    SearchTableCanonicalLanguageRegions = 12,
    SearchTableParentLanguages = 13,
    SearchTableRegionReplacements = 14,
};

struct GrandfatheredTagsTableEntry
{
    const wchar_t* tag;
    const wchar_t* replacement;
};

class LocalStringsArray
{
public:
    LocalStringsArray() = default;

    ~LocalStringsArray() { delete[] m_localStrings; }

    HRESULT Initialize(const wchar_t* strings, wchar_t separator);

    [[nodiscard]] const wchar_t* GetStrings() const { return m_localStrings != nullptr ? m_localStrings : m_strings; }

private:
    const wchar_t* m_strings {};
    wchar_t* m_localStrings {};
};

extern const std::uint16_t regionTags[];
extern const std::uint16_t langTags[];
extern const std::uint32_t scriptTags[];
extern const GrandfatheredTagsTableEntry grandfatheredTagsTable[];

int CompareGrandfatheredTags(const void* key, const void* entry);

extern "C" bool WINAPI IsWellFormedTag(const wchar_t* languageTag);
extern "C" HRESULT WINAPI
GetDistanceOfClosestLanguageInList(const wchar_t* language, const wchar_t* languageList, wchar_t separator, double* score);
extern "C" HRESULT WINAPI FormatLanguageTag(const wchar_t* languageTag, int maximumLength, const wchar_t* source, wchar_t* result);

int WINAPI CompareBcp47Tags(const wchar_t* left, const wchar_t* right);

bool WINAPI TryFindTableEntry(
    std::uint64_t searchKey,
    std::uint64_t useAlternateKey,
    SEARCH_TABLE_TYPE tableType,
    std::uint64_t* resultKey,
    const void** result);

const std::uint64_t* WINAPI FindInCompactBcp47TagTable(std::uint64_t searchKey, const std::uint64_t* table, std::uint32_t count);

bool WINAPI TryFindSuppressedScript(
    std::uint16_t languageIndex,
    std::uint16_t variantIndex,
    std::uint16_t* scriptIndex,
    std::uint16_t* regionIndex,
    bool* suppressedScript,
    bool useDefaults);

bool WINAPI InitSearchKey(const wchar_t* value, std::uint16_t characterBits, std::uint64_t* searchKey);
