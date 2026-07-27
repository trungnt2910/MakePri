#pragma once

#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Collections.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/Checksums.h>
#include <mrm/DefObject.h>
#include <mrm/Results.h>
#include <mrm/Atoms.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/platform/base.h>
#include <DefStatus.h>

#include <windows.h>

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Microsoft::Resources
{
class AtomPoolGroup;
class IDecisionInfo;
class IDefStatusEx;
class StringResult;
} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{

class CItemInstanceEntry
{
public:
    ~CItemInstanceEntry();

    static CItemInstanceEntry* NewForEmbeddedData(
        const wchar_t* source,
        const wchar_t* itemName,
        MrmEnvironment::ResourceItemType resourceItemType,
        MrmEnvironment::ResourceValueType resourceValueType,
        BlobResult* value,
        int qualifierSetIndex,
        ULONG flags,
        const wchar_t* valueTypeName,
        std::map<std::wstring, std::wstring>* qualifiers,
        IDefStatusEx* status);
    static CItemInstanceEntry* NewForString(
        const wchar_t* source,
        const wchar_t* itemName,
        MrmEnvironment::ResourceItemType resourceItemType,
        MrmEnvironment::ResourceValueType resourceValueType,
        const wchar_t* value,
        int qualifierSetIndex,
        ULONG flags,
        const wchar_t* valueTypeName,
        std::map<std::wstring, std::wstring>* qualifiers,
        IDefStatusEx* status);
    static CItemInstanceEntry* NewForLink(
        const wchar_t* source,
        const wchar_t* itemName,
        const wchar_t* link,
        ULONG flags,
        const wchar_t* valueTypeName,
        std::map<std::wstring, std::wstring>* qualifiers,
        IDefStatusEx* status);

    const wchar_t* projectRoot;
    StringResult source;
    StringResult itemName;
    StringResult value;
    StringResult link;
    MrmEnvironment::ResourceItemType resourceItemType;
    MrmEnvironment::ResourceValueType resourceValueType;
    std::int32_t qualifierSetIndex;
    ULONG flags;
    StringResult valueTypeName;
    BlobResult blob;
    std::map<std::wstring, std::wstring> qualifiers;

private:
    CItemInstanceEntry(
        MrmEnvironment::ResourceItemType resourceItemType,
        MrmEnvironment::ResourceValueType resourceValueType,
        int qualifierSetIndex,
        ULONG flags);

    HRESULT _Init(
        const wchar_t* source,
        const wchar_t* itemName,
        const wchar_t* valueTypeName,
        std::map<std::wstring, std::wstring>* qualifiers);
    HRESULT _CopyString(const wchar_t* source, StringResult* result);
    HRESULT _ReleaseAndSetBlobResult(BlobResult* value);
};

class CItemInstanceSink
{
public:
    explicit CItemInstanceSink(bool sorted = false);
    ~CItemInstanceSink();

    HRESULT AddEntry(CItemInstanceEntry* entry);
    CItemInstanceEntry* PopEntry();
    CItemInstanceEntry* GetEntry(std::uint32_t index);
    [[nodiscard]] std::uint32_t GetNumberOfEntries() const;
    [[nodiscard]] bool empty() const;

private:
    bool _bSorted;
    std::vector<CItemInstanceEntry*> _IIESink;
};

struct AtomComparator
{
    bool operator()(const Atom& left, const Atom& right) const;
};

struct QualifierResultComparator
{
    bool operator()(const QualifierResult& left, const QualifierResult& right) const;
};

class CUtilities
{
public:
    struct QualifierValues
    {
        Atom qualifierNameAtom;
        std::wstring wstrValues;
    };

    static bool GetVersionFromString(const wchar_t* version, const wchar_t* minimumVersion, MrmPlatformVersionInternal* result);
    static HRESULT CheckIfFileOrFolder(const wchar_t* path, IDefStatusEx* status, bool* isFile);
    static HRESULT GetAbsolutePath(const wchar_t* path, IDefStatusEx* status, StringResult& result);
    static HRESULT AdjustForProjectRoot(const wchar_t* projectRoot, IDefStatusEx* status, std::wstring& path);
    static HRESULT GetPathInAccessibleFormat(
        const wchar_t* projectRoot,
        const wchar_t* path,
        IDefStatusEx* status,
        const wchar_t** accessiblePath);
    static HRESULT GetPathInAccessibleFormat(const wchar_t* path, const wchar_t** accessiblePath);
    static HRESULT LoadFile(const wchar_t* path, std::wstring& contents, IDefStatusEx* status);
    static HRESULT ReadUnicodeTextFile(const wchar_t* path, wchar_t** contents, ULONG* unusedSize, int unusedFlags);
    static bool GetQualifierTagFromQualifierIndex(
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        int qualifierIndex,
        IDefStatusEx* status,
        std::wstring& qualifierTag);
    static bool GetQualifierTagFromQualifierSetIndex(
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        int qualifierSetIndex,
        IDefStatusEx* status,
        std::wstring& qualifierSetTag);
    static HRESULT GetQualifierMapFromQualifierTag(std::wstring qualifierTag, std::map<std::wstring, std::wstring>* qualifiers);
    static HRESULT GetListOfUnusedQualifiers(
        const IDecisionInfo* decisionInfo,
        CItemInstanceSink* sink,
        std::list<int>* unusedQualifiers,
        IDefStatus* status);
    static HRESULT GetListOfExcludedQualifiers(
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        std::list<int>* excludedQualifiers,
        IDefStatus* status);
    static HRESULT GetQualifierStringMap(
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        CItemInstanceSink* sink,
        std::map<std::wstring, QualifierValues*>* result,
        IDefStatus* status,
        bool includeExcluded);
    static bool DisplayQualifierInformation(IDecisionInfo* decisionInfo, AtomPoolGroup* atomPoolGroup, IDefStatusEx* status);
    static bool GetLanguageOnlyQualifierSetIndexList(
        IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        std::list<int>* qualifierSetIndices,
        IDefStatusEx* status);
    static bool GetLanguageOnlyQualifierSetMap(
        IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        std::map<std::wstring, int>* qualifierSetIndices,
        IDefStatusEx* status);
    static std::wstring NormalizeLanguageTag(const std::wstring& languageTag);
    static void NormalizeAllLanguageTags(std::vector<std::wstring>& languageTags);

private:
    static bool _GetQualifierStringMap(
        std::map<Atom, std::set<QualifierResult, QualifierResultComparator>, AtomComparator> qualifierResults,
        AtomPoolGroup* atomPoolGroup,
        IDefStatus* status,
        std::map<std::wstring, QualifierValues*>* result);
};

} // namespace Microsoft::Resources::Indexers
