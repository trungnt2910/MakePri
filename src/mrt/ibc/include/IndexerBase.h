#pragma once

#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/DefObject.h>
#include <mrm/Results.h>
#include <mrm/Atoms.h>
#include <mrm/platform/base.h>
#include <ItemInstanceSink.h>
#include <IFsIndexer.h>
#include <IIndexOptions.h>
#include <ParameterParser.h>

#include <windows.h>

#include <list>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

struct IXMLDOMDocument2;
struct IXMLDOMNode;
struct IXMLDOMNodeList;

namespace Microsoft::Resources
{

class AtomPoolGroup;
class IHierarchicalSchema;
class MrmProfile;
class QualifierResult;
class QualifierSetResult;
class StandalonePriFile;
class UnifiedEnvironment;

namespace Build
{

class DecisionInfoBuilder;
class DecisionInfoQualifierSetBuilder;
class PriFileBuilder;
class PriSectionBuilder;

} // namespace Build
} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{

class CHIndexerBase;
class CIndexPass;
class CItemInstanceEntry;
class CPackageInfo;
class CPriSchemaReader;
class CQualifierApplicator;
enum class MappingFileFormat;
struct LogItem;

class CContentChecksumData
{
public:
    enum class ContentChecksumOperation : std::int32_t
    {
        MainPackage = 0,
        AutoMergePackage = 1,
        ResourcePackage = 2,
    };

    enum class NeutralLanguageCandidateCreation : std::int32_t
    {
        Always = 0,
        IfNoCandidates = 1,
        Never = 2,
    };

    static CContentChecksumData* New(
        bool enabled,
        ContentChecksumOperation operation,
        NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
        std::vector<std::wstring>* qualifierValues,
        std::uint32_t contentChecksumValue,
        bool checksumValueProvided,
        IDefStatusEx* status);

    bool enabled;
    ContentChecksumOperation operation;
    NeutralLanguageCandidateCreation neutralLanguageCandidateCreation;
    std::vector<std::wstring>* qualifierValues;
    std::uint32_t contentChecksumValue;
    std::uint32_t checksumItemCount;
    bool checksumValueProvided;
};

class IbcOptions : public IIndexOptions
{
public:
    IbcOptions();
    ~IbcOptions();

    [[nodiscard]] bool GetShouldProcessHiddenFiles() const override;
    [[nodiscard]] bool GetShouldProcessLinkedFiles() const override;
    [[nodiscard]] bool GetShouldBuildAutoMergePri() const override;
    [[nodiscard]] bool GetShouldBuildDeploymentMergeablePri() const override;
    [[nodiscard]] bool GetShouldValidateDefaultQualifiers() const override;
    [[nodiscard]] bool GetShouldSuppressConsoleOutput() const override;
    [[nodiscard]] bool GetShouldSuppressEmbeddedData() const override;
    [[nodiscard]] bool GetShouldGenerateReverseMap() const override;
    [[nodiscard]] bool GetShouldUseOptimizedEncoding() const override;
    [[nodiscard]] bool GetShouldOmitSchemaFromResourcePacks() const override;
    [[nodiscard]] bool GetShouldUseGranularResourceSplitting() const override;
    [[nodiscard]] bool GetShouldSplitLanguageVariants() const override;
    [[nodiscard]] bool GetShouldCreateContentChecksum() const override;
    [[nodiscard]] const wchar_t* GetIndexerSchemaPath() const override;
    [[nodiscard]] const ISchemaCollection* GetIndexerSchemaCollection(IDefStatusEx* status) const override;
    [[nodiscard]] bool GetShouldDisableDeduplication() const override;

    void Set(const IIndexOptions* options);

private:
    friend class CHIndexerBase;

    bool m_shouldProcessHiddenFiles;
    bool m_shouldProcessLinkedFiles;
    bool m_shouldBuildAutoMergePri;
    bool m_shouldBuildDeploymentMergeablePri;
    bool m_shouldValidateDefaultQualifiers;
    bool m_shouldSuppressConsoleOutput;
    bool m_shouldSuppressEmbeddedData;
    bool m_shouldGenerateReverseMap;
    bool m_shouldUseOptimizedEncoding;
    bool m_shouldOmitSchemaFromResourcePacks;
    StringResult m_indexerSchemaPath;
    const ISchemaCollection* m_indexerSchemaCollection;
    bool m_shouldDisableDeduplication;
    bool m_shouldUseGranularResourceSplitting;
    bool m_shouldSplitLanguageVariants;
    bool m_shouldCreateContentChecksum;
};

class CHIndexerBase
{
public:
    ~CHIndexerBase();

    static HRESULT NewForNew(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        const wchar_t* simpleId,
        ULONG majorVersion,
        IDefStatusEx* status,
        CHIndexerBase** result);
    static HRESULT NewForVersioned(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        const wchar_t* inputPriFile,
        IDefStatusEx* status,
        CHIndexerBase** result);
    static HRESULT NewForResourcePack(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        const wchar_t* unused,
        const wchar_t* inputPriFile,
        IDefStatusEx* status,
        CHIndexerBase** result);
    StandalonePriFile* GetBuiltPriFile(IDefStatusEx* status);
    HRESULT GenerateMappingFiles(MappingFileFormat format, const wchar_t* priFileOutputLocation, IDefStatusEx* status);
    HRESULT CheckIfOutputUnderRoot(const wchar_t* outputDirectory, IDefStatusEx* status, bool* result);
    bool SetOmitSchemaFromResourcePacks(IDefStatusEx* status);
    bool SetSplitLanguageVariants(IDefStatusEx* status);
    void CalibrateResourceIdCompressionLevel(IDefStatusEx* status);
    HRESULT ParsePackagingNode(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT Process(
        const wchar_t* indexName,
        CContentChecksumData::NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
        std::uint32_t flags,
        IDefStatusEx* status);
    HRESULT LogInfo(const wchar_t* format, ...);
    static HRESULT LogInfo(std::vector<LogItem>* logItems, const wchar_t* format, ...);

private:
    enum SchemaPermission
    {
        None = 0,
        ReadOnly = 1,
        Writable = 2,
    };

    enum ResourcePackageMode
    {
        FatPack = 0,
        AutoQualifier = 1,
        Manual = 2,
    };

    friend class CPackageInfo;
    friend auto ::MakePriNewVersionedPack(Tools::MakePri::InputArgs* inputArgs, Tools::MakePri::UsageScenario scenario) -> HRESULT;

    CHIndexerBase();
    HRESULT InitForNew(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        const wchar_t* simpleId,
        ULONG majorVersion,
        IDefStatusEx* status);
    HRESULT InitForResourcePack(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        const wchar_t* unused,
        const wchar_t* inputPriFile,
        IDefStatusEx* status);
    HRESULT _ValidateConfigurationSchema(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _InitializeIbc(
        IXMLDOMNode* root,
        const wchar_t* projectRoot,
        const wchar_t* outputDirectory,
        const IIndexOptions* indexOptions,
        const wchar_t* configurationFile,
        IDefStatusEx* status);
    HRESULT _InitializeEnvironment(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _InitSchemaInfoFromInputFile(const wchar_t* inputPriFile, IDefStatusEx* status);
    HRESULT _SetPlatformVersionAndProfile(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _SetMajorVersionNumber(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _SetDeploymentMergeableFlag(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _ParseIndexNodes(IXMLDOMNode* root, IDefStatusEx* status);
    HRESULT _ParseAutoResourcePackageNodeList(IXMLDOMNodeList* nodes, IDefStatusEx* status);
    HRESULT _ParseAutoPackagesNode(IXMLDOMNode* node, const wchar_t* qualifierName, IDefStatusEx* status);
    HRESULT _ParseManualResourcePackageNodeList(IXMLDOMNodeList* nodes, IDefStatusEx* status);
    HRESULT _ParseManualQualifierSetNodeList(IXMLDOMNodeList* nodes, const wchar_t* packageName, IDefStatusEx* status);
    bool _GetQsiFromQualNameValue(
        const wchar_t* qualifierName,
        const wchar_t* qualifierValue,
        IDefStatusEx* status,
        int* qualifierSetIndex,
        bool* isDefault);
    bool _GetQsiFromQualifierResult(QualifierResult qualifier, IDefStatusEx* status, int* qualifierSetIndex);
    bool _GetQsiFromQualifierResultArray(std::vector<QualifierResult>* qualifiers, IDefStatusEx* status, int* qualifierSetIndex);
    bool _FindDominantQsi(std::vector<int>* qualifierSetIndices, IDefStatusEx* status, int* qualifierSetIndex);
    bool _FindEffectiveQsi(int qualifierSetIndex, IDefStatusEx* status, int* effectiveQualifierSetIndex);
    bool _HasDefaultOrNeutralQualifier(int qualifierSetIndex, IDefStatusEx* status);
    bool _GetOverlappingQsiMap(int qualifierSetIndex, IDefStatusEx* status, std::map<int, int>* overlappingQualifierSets);
    bool _GetPackagingAffinityForQualifierSet(int qualifierSetIndex, IDefStatusEx* status, StringResult* affinity);
    bool _GetPackagingAffinityForQualifier(
        QualifierResult* qualifier,
        IDefStatusEx* status,
        IBuildQualifierType::PackagingFlags* packagingFlags,
        StringResult* affinity);
    HRESULT _AddEntriesToManualResourcePacks(IDefStatusEx* status);
    HRESULT _AddEntryToPackageWithName(
        int qualifierSetIndex,
        const wchar_t* mainPackageName,
        const wchar_t* resourcePackageName,
        CItemInstanceEntry* entry,
        IDefStatusEx* status);
    HRESULT _GenerateArpsAndAddEntries(const wchar_t* mainPackageName, IDefStatusEx* status);
    HRESULT _InitializeQualifierNameTokenMap(IDefStatusEx* status);
    HRESULT _CollectDefaultLanguages(IDefStatusEx* status);
    HRESULT _ProcessIndexSink(const wchar_t* indexName, IDefStatusEx* status);
    HRESULT _ValidateDefaultLanguageQualifiers(IDefStatusEx* status);
    HRESULT _WriteIndexFiles(
        CContentChecksumData::NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
        std::uint32_t flags,
        IDefStatusEx* status);
    HRESULT _AddReverseMapSection(IDefStatusEx* status);
    HRESULT _WriteStatusToStream(IDefStatusEx* status);
    HRESULT _ValidateFullPackage(IDefStatusEx* status);
    HRESULT _CreateMetaDataFile(IDefStatusEx* status);
    bool _IsArpQualifier(Atom qualifier);
    bool _HasArpQualifier(QualifierSetResult qualifierSet, IDefStatusEx* status);
    void _TryRemoveLanguage(std::vector<std::wstring>& languages, const std::wstring& language);

    std::vector<void*> _allocations;
    std::uint32_t _unknown10;
    void* _unknown14;
    StringResult _indexName;
    StringResult _simpleId;
    std::vector<CIndexPass*> _indexPassList;
    AtomPoolGroup* _pAtomPoolGroup;
    MrmProfile* _pMrmProfile;
    MrmProfile* _pMrmProfileForResourcePackGeneration;
    UnifiedEnvironment* _pUnifiedEnvironment;
    Build::DecisionInfoBuilder* _pDecisionInfoBuilder;
    MrmPlatformVersionInternal _platformVersion;
    CItemInstanceSink _traversalSink;
    CItemInstanceSink _indexSink;
    std::list<CItemInstanceEntry*> _disposalList;
    StandalonePriFile* _pInputFileReader;
    CPriSchemaReader* _pSchemaReader;
    const IHierarchicalSchema* _pPreviousSchema;
    IbcOptions _options;
    ULONG _ulMajorVersion;
    SchemaPermission _eSchemaPermission;
    ResourcePackageMode _eRpMode;
    std::int32_t _unknownD4;
    std::int32_t _resourceIdCompressionLevel;
    std::int32_t _useLegacyPriFileName;
    std::vector<Atom> _arpQualifierList;
    std::map<int, std::wstring> _manualQsiToRpNameMap;
    CPackageInfo* _pMainPackage;
    CPackageInfo* _pFatPackage;
    std::map<std::wstring, CPackageInfo*> _resourcePackages;
    std::map<int, int> _multiToSingleQsiMap;
    std::vector<std::wstring> _defaultLanguages;
    std::map<std::wstring, std::set<std::wstring>> _autoPackages;
    std::map<std::wstring, std::wstring> _qualifierNameTokenMap;
    std::vector<LogItem> _logItems;
    StringResult _projectRootFolder;
    StringResult _outputFolder;
    StringResult _metaDataFilePath;
    StringResult _priFileName;
    bool _unknown194;
};

enum class MappingFileFormat
{
    NoMapping = 0,
    AppX = 1,
};

class CPackageInfo
{
public:
    enum PackageState
    {
        Initial = 0,
        BuilderCreated = 1,
        Finalized = 2,
        FileWritten = 3,
    };

    ~CPackageInfo();

    static CPackageInfo* New(
        const wchar_t* mainPackageName,
        const wchar_t* resourcePackName,
        const IIndexOptions* indexOptions,
        Build::DecisionInfoBuilder* decisionInfoBuilder,
        UnifiedEnvironment* environment,
        AtomPoolGroup* atomPoolGroup,
        MrmProfile* mrmProfile,
        CHIndexerBase* indexer,
        IDefStatusEx* status);

    bool AddEntry(CItemInstanceEntry* entry, IDefStatusEx* status);
    bool AddEntry(int qualifierSetIndex, CItemInstanceEntry* entry, IDefStatusEx* status);
    bool AddReferencedEntry(CItemInstanceEntry* entry, IDefStatusEx* status);
    bool ComputeReportedQualifierSetIndices(IDefStatusEx* status);
    bool Build(const wchar_t* simpleId, std::uint16_t majorVersion, CContentChecksumData* contentChecksumData, IDefStatusEx* status);
    bool Build(const IHierarchicalSchema* schema, CContentChecksumData* contentChecksumData, IDefStatusEx* status);
    bool Build(const wchar_t* simpleId, const IHierarchicalSchema* schema, CContentChecksumData* contentChecksumData, IDefStatusEx* status);
    bool ConstructPriFileName(StringResult* result, IDefStatusEx* status);
    bool WriteToFile(const wchar_t* outputLocation, IDefStatusEx* status);
    bool AddContentsToLogNode(IXMLDOMDocument2* document, IXMLDOMNode* parent, bool addPriFileName, IDefStatusEx* status);
    StandalonePriFile* GetReader(IDefStatusEx* status);
    bool WriteStatusToStream(IDefStatusEx* status);
    bool GenerateMappingFile(
        MappingFileFormat format,
        const wchar_t* outputLocation,
        const wchar_t* priFileOutputLocation,
        const wchar_t* resourcePackName,
        std::vector<std::wstring>* defaultLanguages,
        IDefStatusEx* status);
    bool _ConstructResourceId(IDefStatusEx* status, StringResult* result);
    bool _ConstructResourceIdTokenAndValues(const wchar_t* resourcePackName, IDefStatusEx* status, StringResult* result);
    bool _ConstructResourceIdValuesOnly(const wchar_t* resourcePackName, IDefStatusEx* status, StringResult* result);
    bool _ConstructResourceIdValuesOnlyHashed(const wchar_t* resourcePackName, IDefStatusEx* status, StringResult* result);
    bool _ConstructResourceIdHashed(const wchar_t* resourceId, IDefStatusEx* status, StringResult* result);

private:
    friend class CHIndexerBase;

    CPackageInfo(
        const IIndexOptions* indexOptions,
        Build::DecisionInfoBuilder* decisionInfoBuilder,
        UnifiedEnvironment* environment,
        AtomPoolGroup* atomPoolGroup,
        MrmProfile* mrmProfile,
        CHIndexerBase* indexer);

    bool _Init(const wchar_t* mainPackageName, const wchar_t* resourcePackName, IDefStatusEx* status);
    HRESULT _GetConsolidatedSink(CItemInstanceSink* sink);
    bool _GetQsbFromQsi(
        int qualifierSetIndex,
        Build::PriSectionBuilder* priSectionBuilder,
        IDefStatusEx* status,
        Build::DecisionInfoQualifierSetBuilder** qualifierSetBuilder);
    bool _GetLanguageQualifierValue(
        Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder,
        StringResult* result,
        IDefStatusEx* status);
    bool _GetLanguageQualifierValues(
        Build::PriSectionBuilder* priSectionBuilder,
        std::set<int>* qualifierSetIndices,
        std::set<std::wstring>* result,
        IDefStatusEx* status);
    bool _IsItemContentChecksum(StringResult* itemName, IDefStatusEx* status);
    bool _QualifierSetAppliesForChecksumCalculation(
        StringResult* itemName,
        Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder,
        int qualifierSetIndex,
        int fallbackQualifierSetIndex,
        CContentChecksumData* contentChecksumData,
        IDefStatusEx* status);
    bool _LanguageIsDefaultLanguage(
        StringResult* language,
        std::vector<std::wstring>* defaultLanguages,
        bool* result,
        IDefStatusEx* status);
    void _DisplayContentChecksumInformation(CContentChecksumData* contentChecksumData, IDefStatusEx* status);
    bool _AddContentChecksumCandidates(
        Build::PriSectionBuilder* priSectionBuilder,
        std::set<int>* qualifierSetIndices,
        CContentChecksumData* contentChecksumData,
        IDefStatusEx* status);
    bool _GenerateAppXMappingFile(
        const wchar_t* outputLocation,
        const wchar_t* priFileOutputLocation,
        const wchar_t* resourcePackName,
        std::vector<std::wstring>* defaultLanguages,
        IDefStatusEx* status);
    bool _ConstructResourceDimensionList(IDefStatusEx* status, std::set<std::wstring>* dimensions);
    bool _PopulateBuilder(CContentChecksumData* contentChecksumData, IDefStatusEx* status);

    const IIndexOptions* _options;
    bool _bIsResourcePackage;
    PackageState _ePackageState;
    StringResult _strPackageName;
    StringResult _strResourcePackName;
    StringResult _strIndexFileLocation;
    std::set<int> _reportedQsiList;
    CItemInstanceSink _packageSink;
    CItemInstanceSink _readOnlyPackageSink;
    Build::PriFileBuilder* _pPriFileBuilder;
    const IHierarchicalSchema* _pSchema;
    StandalonePriFile* _overrideSchemaFile;
    Build::DecisionInfoBuilder* _pDecisionInfoBuilder;
    UnifiedEnvironment* _pUnifiedEnvironment;
    AtomPoolGroup* _pAtomPoolGroup;
    MrmProfile* _pMrmProfile;
    CHIndexerBase* _pIndexer;
    void* _pPriBuffer;
    std::uint32_t _cchBuffer;
    std::map<std::wstring, CUtilities::QualifierValues*> _qualifierMap;
    std::map<int, Build::DecisionInfoQualifierSetBuilder*> _qsiToQsbMap;
};

class CBootStrapIndexer
{
public:
    static HRESULT New(
        const UnifiedEnvironment* pEnvironment,
        Build::DecisionInfoBuilder* pDecisionInfoBuilder,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pQualifierApplicator,
        IDefStatusEx* pStatus,
        CBootStrapIndexer** ppBootStrapIndexer);

    ~CBootStrapIndexer();

    HRESULT Process(CItemInstanceSink* pTraversalSink, bool* pbRemoveContainerFromIndex);
    HRESULT GetIndexablePath(IDefStatusEx* pStatus, StringResult* strIndexablePath);
    const wchar_t* GetProjectRoot(IDefStatusEx* pStatus);

private:
    HRESULT _Init(
        const UnifiedEnvironment* pEnvironment,
        Build::DecisionInfoBuilder* pDecisionInfoBuilder,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pQualifierApplicator,
        IDefStatusEx* pStatus);
    HRESULT _ParseNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    HRESULT _ProcessIndexNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    HRESULT _AdjustSlashes(const wchar_t* pPath, StringResult& strAdjustedPath, IDefStatusEx* pStatus);
    HRESULT _NormalizePath(const wchar_t* pPath, StringResult& strNormalizedPath, IDefStatusEx* pStatus);
    HRESULT _ProcessConditionsNode(IXMLDOMNode* pConditionsNode, IDefStatusEx* pStatus);
    HRESULT _ProcessUltimateFallbackNode(IXMLDOMNode* pUltFallbackNode, IDefStatusEx* pStatus);
    HRESULT CreateStringEntry(
        const wchar_t* pSource,
        const wchar_t* pItemName,
        const wchar_t* pValue,
        MrmEnvironment::ResourceItemType resourceItemType,
        MrmEnvironment::ResourceValueType resourceValueType,
        int qualifierSetIndex,
        ULONG ulActionFlags,
        IDefStatusEx* pStatus);

    const UnifiedEnvironment* _c_pEnvironment {};
    StringResult _strNewProjectRootFolder;
    StringResult _strStartIndexAt;
    const wchar_t* _pProjectRootFolder {};
    StringResult _strValue;
    CItemInstanceEntry* _pItemInstanceEntry {};
    std::int32_t _baseQualifierSetIndex {-1};
    CQualifierApplicator* _pQualifierApplicator {};
};

class CIndexPass
{
public:
    static HRESULT WINAPI
    New(IXMLDOMNode* pIndexPassNode,
        const MrmProfile* pProfile,
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        Build::DecisionInfoBuilder* pDecisionInfoBuilder,
        const IIndexOptions* options,
        std::vector<LogItem>* pLogItems,
        IDefStatusEx* pStatus,
        CIndexPass** ppCIndexPass);

    ~CIndexPass();

    HRESULT Process(CItemInstanceSink* pTraversalSink, CItemInstanceSink* pIndexSink, IDefStatusEx* pStatus);
    HRESULT GetIndexablePath(IDefStatusEx* pStatus, StringResult* strIndexablePath);
    HRESULT GetDefaultQualifierValues(const wchar_t* pSource, IDefStatusEx* pStatus, StringResult* strQualifierValues);

private:
    CIndexPass() = default;

    HRESULT _Init(
        IXMLDOMNode* pIndexPassNode,
        const MrmProfile* pProfile,
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        Build::DecisionInfoBuilder* pDecisionInfoBuilder,
        const IIndexOptions* options,
        std::vector<LogItem>* pLogItems,
        IDefStatusEx* pStatus);
    HRESULT _ParseNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    HRESULT _ProcessIndexerNodes(IXMLDOMNodeList* pNodeList, IDefStatusEx* pStatus);
    HRESULT _InitializeIndexers(IXMLDOMNode* pIPNode, const IIndexOptions* options, IDefStatusEx* pStatus);
    HRESULT _DisplayQualifierInfo(CItemInstanceSink* pIndexSink, const wchar_t* pStartIndexAtValue, IDefStatusEx* pStatus);
    HRESULT _InstantiateIndexer(wchar_t* pIndexerType, IDefStatusEx* pStatus);
    HRESULT _ParseAllowedNodes(IXMLDOMNode* pIndexNode, IDefStatusEx* pStatus);
    HRESULT _ParseAllowedNodeQualifierValues(IXMLDOMNode* pQualifierNode, IDefStatusEx* pStatus);

    bool _bSuppressConsoleOutput {};
    bool _bSuppressEmbeddedData {};
    IXMLDOMNode* _pIndexPassNode {};
    const MrmProfile* _c_pProfile {};
    const UnifiedEnvironment* _c_pEnvironment {};
    const wchar_t* _pProjectRootFolder {};
    CBootStrapIndexer* _pBootStrapIndexer {};
    std::list<IFormatSpecificIndexer*> _FSILists[2];
    std::stack<IXMLDOMNode*> _IndexPassNodeDisposalList;
    Build::DecisionInfoBuilder* _pDecisionInfoBuilder {};
    CQualifierApplicator* _pQualifierApplicator {};
    std::map<std::wstring, std::set<std::wstring>> _allowedQualifierValues;
    std::vector<LogItem>* _pLogItems {};
};

} // namespace Microsoft::Resources::Indexers
