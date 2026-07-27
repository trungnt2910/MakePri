#pragma once

#include <mrm/BaseInternal.h>
#include <mrm/Collections.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/Checksums.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <IFsIndexer.h>

#include <string>

namespace Microsoft::Resources::Indexers
{
class CFIXmlConfig;
class CFileInfo;
class CFolderInfo;

class CFolderIndexer : public IFormatSpecificIndexer
{
public:
    ~CFolderIndexer() override;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pQualApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pSink, IDefStatusEx* pStatus, bool* pRemoveContainer) override;

    HRESULT
    ProcessSubFolders(CItemInstanceEntry* pEntry, CFolderInfo* pFolder, CItemInstanceSink* pSink, IDefStatusEx* pStatus);
    HRESULT ProcessFiles(CItemInstanceEntry* pEntry, CFolderInfo* pFolder, CItemInstanceSink* pSink, IDefStatusEx* pStatus);

private:
    struct ITEM_INSTANCE_ENTRY
    {
        const wchar_t* pCollectionName;
        const wchar_t* pItemName;
        MrmEnvironment::ResourceItemType itemType;
        MrmEnvironment::ResourceValueType resourceValueType;
        const wchar_t* pValue;
        std::int32_t conditionSetIndex;
        ULONG uiActionFlags;
    };

    HRESULT _Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        IDefStatusEx* pStatus);
    void _InitInstanceEntry(CItemInstanceEntry* pEntry, ITEM_INSTANCE_ENTRY* pLocalEntry) const;
    HRESULT _CreateFolderEntry(
        CItemInstanceEntry* pEntry,
        const std::wstring& wszFolderName,
        std::wstring& pwszItemName,
        std::wstring& pwszValue,
        ITEM_INSTANCE_ENTRY* pLocalEntry,
        IDefStatusEx* pStatus);
    HRESULT _CreateFileEntry(
        CItemInstanceEntry* pEntry,
        const std::wstring& wszFileName,
        std::wstring& pwszItemName,
        std::wstring& pwszValue,
        ITEM_INSTANCE_ENTRY* pLocalEntry,
        IDefStatusEx* pStatus);

    CFolderInfo* _pFolderInfo {};
    CQualifierApplicator* _pQualApplicator {};
    CFIXmlConfig* _pFIXmlConfig {};
    bool _bDoneInit {};
    bool _bError {};
    const IIndexOptions* _options {};
};
} // namespace Microsoft::Resources::Indexers
