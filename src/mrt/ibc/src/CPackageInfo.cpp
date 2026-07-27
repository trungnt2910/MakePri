#include "StdAfx.h"

#include <IndexerBase.h>
#include <mrm/build/MrmBuilders.h>

namespace Microsoft::Resources::Indexers
{

CPackageInfo* CPackageInfo::New(
    const wchar_t* const mainPackageName,
    const wchar_t* const resourcePackName,
    const IIndexOptions* const indexOptions,
    Build::DecisionInfoBuilder* const decisionInfoBuilder,
    UnifiedEnvironment* const environment,
    AtomPoolGroup* const atomPoolGroup,
    MrmProfile* const mrmProfile,
    CHIndexerBase* const indexer,
    IDefStatusEx* const status)
{
    if (status == nullptr)
    {
        return nullptr;
    }
    if (mainPackageName == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 42, L"mainPackageName", 0);
        return nullptr;
    }
    if (decisionInfoBuilder == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 43, L"decisionInfoBuilder", 0);
        return nullptr;
    }
    if (environment == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 44, L"environment", 0);
        return nullptr;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 45, L"atomPoolGroup", 0);
        return nullptr;
    }
    if (mrmProfile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 46, L"mrmProfile", 0);
        return nullptr;
    }

    auto* const result =
        new (std::nothrow) CPackageInfo(indexOptions, decisionInfoBuilder, environment, atomPoolGroup, mrmProfile, indexer);
    if (status->Failed())
    {
        delete result;
        return nullptr;
    }
    if (result == nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 57, L"", 0);
        return nullptr;
    }
    if (!result->_Init(mainPackageName, resourcePackName, status))
    {
        delete result;
        return nullptr;
    }
    return result;
}

CPackageInfo::CPackageInfo(
    const IIndexOptions* const indexOptions,
    Build::DecisionInfoBuilder* const decisionInfoBuilder,
    UnifiedEnvironment* const environment,
    AtomPoolGroup* const atomPoolGroup,
    MrmProfile* const mrmProfile,
    CHIndexerBase* const indexer) :
    _options(indexOptions),
    _bIsResourcePackage(false),
    _ePackageState(Initial),
    _pPriFileBuilder(nullptr),
    _pSchema(nullptr),
    _overrideSchemaFile(nullptr),
    _pDecisionInfoBuilder(decisionInfoBuilder),
    _pUnifiedEnvironment(environment),
    _pAtomPoolGroup(atomPoolGroup),
    _pMrmProfile(mrmProfile),
    _pIndexer(indexer),
    _pPriBuffer(nullptr),
    _cchBuffer(0)
{}

bool CPackageInfo::_Init(const wchar_t* const mainPackageName, const wchar_t* const resourcePackName, IDefStatusEx* const status)
{
    if (resourcePackName == nullptr)
    {
        if (Def_HrFailed0(_strResourcePackName.SetRef(L""), status) ||
            Def_HrFailed0(DefStringResult_SetCopy(_strPackageName.GetStringResult(), mainPackageName), status))
        {
            return false;
        }
        return true;
    }

    _bIsResourcePackage = true;
    if (Def_HrFailed0(DefStringResult_SetCopy(_strResourcePackName.GetStringResult(), resourcePackName), status) ||
        Def_HrFailed0(_strPackageName.SetRef(mainPackageName), status) ||
        Def_HrFailed0(DefStringResult_Concat(_strPackageName.GetStringResult(), L"."), status) ||
        Def_HrFailed0(DefStringResult_Concat(_strPackageName.GetStringResult(), resourcePackName), status))
    {
        return false;
    }
    return true;
}

CPackageInfo::~CPackageInfo()
{
    while (!_packageSink.empty())
    {
        delete _packageSink.PopEntry();
    }
    while (!_readOnlyPackageSink.empty())
    {
        _readOnlyPackageSink.PopEntry();
    }
    for (const auto& entry : _qualifierMap)
    {
        delete entry.second;
    }
    for (const auto& entry : _qsiToQsbMap)
    {
        delete entry.second;
    }
    if (_pPriBuffer != nullptr)
    {
        HeapFree(GetProcessHeap(), 0, _pPriBuffer);
        _pPriBuffer = nullptr;
    }
    delete _pPriFileBuilder;
}

bool CPackageInfo::AddReferencedEntry(CItemInstanceEntry* const entry, IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 129, L"", 0);
        }
        return false;
    }

    const HRESULT result = _readOnlyPackageSink.AddEntry(entry);
    if (FAILED(result))
    {
        if (status != nullptr)
        {
            status->SetError(result, L"" __FILE__, 136, L"", 0);
        }
        return false;
    }
    return true;
}

bool CPackageInfo::AddEntry(CItemInstanceEntry* const entry, IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 148, L"", 0);
        }
        return false;
    }

    const HRESULT result = _packageSink.AddEntry(entry);
    if (FAILED(result))
    {
        if (status != nullptr)
        {
            status->SetError(result, L"" __FILE__, 157, L"", 0);
        }
        return false;
    }

    _reportedQsiList.insert(entry->qualifierSetIndex);
    return true;
}

bool CPackageInfo::AddEntry(const int qualifierSetIndex, CItemInstanceEntry* const entry, IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 171, L"", 0);
        }
        return false;
    }

    const HRESULT result = _packageSink.AddEntry(entry);
    if (FAILED(result))
    {
        if (status != nullptr)
        {
            status->SetError(result, L"" __FILE__, 180, L"", 0);
        }
        return false;
    }

    _reportedQsiList.insert(qualifierSetIndex);
    return true;
}

bool CPackageInfo::ComputeReportedQualifierSetIndices(IDefStatusEx* const status)
{
    const int numberOfQualifierSets = _pDecisionInfoBuilder->GetNumQualifierSets();
    if (status->Succeeded())
    {
        for (int qualifierSetIndex = 0; qualifierSetIndex < numberOfQualifierSets; ++qualifierSetIndex)
        {
            if (qualifierSetIndex != 0)
            {
                _reportedQsiList.insert(qualifierSetIndex);
            }
        }
    }
    return status->Succeeded();
}

bool CPackageInfo::Build(
    const wchar_t* const simpleId,
    const std::uint16_t majorVersion,
    CContentChecksumData* const contentChecksumData,
    IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 195, L"", 0);
        }
        return false;
    }

    Def_HrFailed0(Build::PriFileBuilder::CreateInstance(simpleId, majorVersion, _pMrmProfile, &_pPriFileBuilder), status);
    if ((status != nullptr) && status->Failed())
    {
        if (_pPriFileBuilder != nullptr)
        {
            delete _pPriFileBuilder;
            _pPriFileBuilder = nullptr;
        }
        return false;
    }
    if (_pPriFileBuilder == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 204, L"", 0);
        }
        return false;
    }
    if (!_PopulateBuilder(contentChecksumData, status))
    {
        return false;
    }

    _ePackageState = BuilderCreated;
    return true;
}

bool CPackageInfo::Build(
    const IHierarchicalSchema* const schema,
    CContentChecksumData* const contentChecksumData,
    IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 225, L"", 0);
        }
        return false;
    }

    _pSchema = schema;
    Def_HrFailed0(Build::PriFileBuilder::CreateInstance(schema, _pMrmProfile, &_pPriFileBuilder), status);
    if ((status != nullptr) && status->Failed())
    {
        if (_pPriFileBuilder != nullptr)
        {
            delete _pPriFileBuilder;
            _pPriFileBuilder = nullptr;
        }
        return false;
    }
    if (_pPriFileBuilder == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 234, L"", 0);
        }
        return false;
    }
    if (!_PopulateBuilder(contentChecksumData, status))
    {
        return false;
    }

    _ePackageState = BuilderCreated;
    return true;
}

bool CPackageInfo::Build(
    const wchar_t* const simpleId,
    const IHierarchicalSchema* const schema,
    CContentChecksumData* const contentChecksumData,
    IDefStatusEx* const status)
{
    if (_ePackageState != Initial)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 256, L"", 0);
        }
        return false;
    }

    _pSchema = schema;
    Def_HrFailed0(Build::PriFileBuilder::CreateForResourcePack(simpleId, schema, _pMrmProfile, &_pPriFileBuilder), status);
    if ((status != nullptr) && status->Failed())
    {
        if (_pPriFileBuilder != nullptr)
        {
            delete _pPriFileBuilder;
            _pPriFileBuilder = nullptr;
        }
        return false;
    }
    if (_pPriFileBuilder == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 266, L"", 0);
        }
        return false;
    }
    if (!_PopulateBuilder(contentChecksumData, status))
    {
        return false;
    }

    _ePackageState = BuilderCreated;
    return true;
}

bool CPackageInfo::ConstructPriFileName(StringResult* const result, IDefStatusEx* const status)
{
    if ((_pIndexer->_platformVersion != MrmPlatformVersionInternal::WindowsCoreVNext) || _pIndexer->_useLegacyPriFileName)
    {
        return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), _strPackageName.GetRef()), status);
    }

    if (!_bIsResourcePackage)
    {
        return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), _pIndexer->_priFileName.GetRef()), status);
    }

    StringResult resourceId;
    bool succeeded = false;
    if (!Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), _pIndexer->_priFileName.GetRef()), status) &&
        !Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), L"."), status) && _ConstructResourceId(status, &resourceId))
    {
        succeeded = !Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), resourceId.GetRef()), status);
    }
    return succeeded;
}

bool CPackageInfo::WriteToFile(const wchar_t* const outputLocation, IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::WriteToFile");
    if ((_ePackageState != BuilderCreated) && (_ePackageState != Finalized))
    {
        status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 311, L"", 0);
        return false;
    }

    status->DiagnosticLogA("Output Location: [%S]", outputLocation);

    StringResult outputPath;
    Def_HrFailed0(DefStringResult_InitRef(outputPath.GetStringResult(), outputLocation), status);
    StringResult priFileName;
    if (status->Succeeded())
    {
        if (ConstructPriFileName(&priFileName, status))
        {
            if (!Def_HrFailed0(DefStringResult_ConcatPathElement(outputPath.GetStringResult(), priFileName.GetRef(), L'\\'), status))
            {
                Def_HrFailed0(DefStringResult_Concat(outputPath.GetStringResult(), L".pri"), status);
            }
        }
    }

    if (status->Succeeded() && (_ePackageState != Finalized))
    {
        if (Def_HrFailed0(_pPriFileBuilder->FinalizeAllSections(), status))
        {
            status->DiagnosticLogWithErrorCodeA("FinalizeAllSections Failed!", status->GetHResult());
        }
        else
        {
            status->DiagnosticLogA("Begin writing file to [%S]", outputPath.GetRef());
            if (Def_HrFailed0(_pPriFileBuilder->WriteToFile(outputPath.GetRef()), status))
            {
                status->DiagnosticLogWithErrorCodeA("WriteToFile Failed!", status->GetHResult());
            }
            else
            {
                status->DiagnosticLogA("File successfully written to [%S]", outputPath.GetRef());
                Def_HrFailed0(DefStringResult_SetCopy(_strIndexFileLocation.GetStringResult(), outputPath.GetRef()), status);
                _ePackageState = FileWritten;
                delete _pPriFileBuilder;
                _pPriFileBuilder = nullptr;
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::WriteToFile", status->GetHResult());
    return status->Succeeded();
}

bool CPackageInfo::AddContentsToLogNode(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const bool addPriFileName,
    IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (_ePackageState != FileWritten)
    {
        status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 369, L"", 0);
        return false;
    }

    IXMLDOMNode* resourceNode = nullptr;
    IXMLDOMNode* valueNode = nullptr;
    CItemInstanceSink itemInstances;
    const HRESULT result = _GetConsolidatedSink(&itemInstances);
    if (SUCCEEDED(result))
    {
        try
        {
            if (addPriFileName)
            {
                StringResult priFileName;
                if (ConstructPriFileName(&priFileName, status) &&
                    !Def_HrFailed0(DefStringResult_Concat(priFileName.GetStringResult(), L".pri"), status) &&
                    FAILED(CXMLUtil::AddAttribute(document, parent, L"filename", priFileName.GetRef())))
                {
                    goto Cleanup;
                }
            }

            while (!itemInstances.empty())
            {
                CItemInstanceEntry* const entry = itemInstances.PopEntry();
                const wchar_t* valueTypeName = entry->valueTypeName.GetRef();
                if (valueTypeName == nullptr)
                {
                    valueTypeName = L"";
                }
                if (DefString_CompareWithOptions(valueTypeName, L"(null)", DefCompare_CaseInsensitive) == 0)
                {
                    valueTypeName = L"";
                }

                std::wstring itemName(entry->source.GetRef());
                itemName.append(L"\\");
                itemName.append(entry->itemName.GetRef());
                std::replace(itemName.begin(), itemName.end(), L'/', L'\\');

                if (entry->link.GetRef() == nullptr)
                {
                    std::wstring qualifierTag;
                    if (entry->qualifierSetIndex != 0)
                    {
                        CUtilities::GetQualifierTagFromQualifierSetIndex(
                            _pDecisionInfoBuilder, _pAtomPoolGroup, entry->qualifierSetIndex, status, qualifierTag);
                    }
                    if (status->Succeeded())
                    {
                        const wchar_t* qualifierTagValue = qualifierTag.c_str();
                        if (qualifierTagValue == nullptr)
                        {
                            qualifierTagValue = L"";
                        }

                        StringResult resourceValueType;
                        Def_HrFailed0(
                            resourceValueType.SetRef(MrmEnvironment::ResourceValueTypeNames[static_cast<int>(entry->resourceValueType)]),
                            status);
                        if (FAILED(CXMLUtil::AddElement(document, parent, L"instance", &resourceNode)) ||
                            FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"itemname", itemName.c_str())) ||
                            FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"qualifiers", qualifierTagValue)) ||
                            FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"src", valueTypeName)) ||
                            FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"type", resourceValueType.GetRef())) ||
                            FAILED(CXMLUtil::AddElement(document, resourceNode, L"value", &valueNode)) ||
                            FAILED(
                                CXMLUtil::SetElementValue(
                                    valueNode, entry->value.GetRef(), CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings)))
                        {
                            goto Cleanup;
                        }

                        CXMLUtil::CleanupNode(&valueNode);
                        for (const auto& qualifier : entry->qualifiers)
                        {
                            if (FAILED(CXMLUtil::AddElement(document, resourceNode, L"metadata", &valueNode)) ||
                                FAILED(CXMLUtil::AddAttribute(document, valueNode, L"type", qualifier.first.c_str())) ||
                                FAILED(
                                    CXMLUtil::SetElementValue(
                                        valueNode, qualifier.second.c_str(), CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings)))
                            {
                                goto Cleanup;
                            }
                            CXMLUtil::CleanupNode(&valueNode);
                        }
                        CXMLUtil::CleanupNode(&resourceNode);
                    }
                }
                else
                {
                    if (FAILED(CXMLUtil::AddElement(document, parent, L"link", &resourceNode)) ||
                        FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"itemname", itemName.c_str())) ||
                        FAILED(CXMLUtil::AddAttribute(document, resourceNode, L"src", valueTypeName)) ||
                        FAILED(CXMLUtil::AddElement(document, resourceNode, L"linksto", &valueNode)) ||
                        FAILED(CXMLUtil::SetElementValue(valueNode, entry->link.GetRef(), CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings)))
                    {
                        goto Cleanup;
                    }

                    CXMLUtil::CleanupNode(&valueNode);
                    for (const auto& qualifier : entry->qualifiers)
                    {
                        if (FAILED(CXMLUtil::AddElement(document, resourceNode, L"metadata", &valueNode)) ||
                            FAILED(CXMLUtil::AddAttribute(document, valueNode, L"type", qualifier.first.c_str())) ||
                            FAILED(
                                CXMLUtil::SetElementValue(
                                    valueNode, qualifier.second.c_str(), CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings)))
                        {
                            goto Cleanup;
                        }
                        CXMLUtil::CleanupNode(&valueNode);
                    }
                    CXMLUtil::CleanupNode(&resourceNode);
                }
            }
        }
        catch (std::bad_alloc)
        {
            if (status->Succeeded())
            {
                status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 482, L"", 0);
            }
        }
    }

Cleanup:
    CXMLUtil::CleanupNode(&valueNode);
    CXMLUtil::CleanupNode(&resourceNode);
    while (!itemInstances.empty())
    {
        itemInstances.PopEntry();
    }
    if (status->Succeeded() && FAILED(result))
    {
        status->SetError(result, L"" __FILE__, 499, L"", 0);
    }
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, status));
    return status->Succeeded();
}

bool CPackageInfo::GenerateMappingFile(
    const MappingFileFormat format,
    const wchar_t* const outputLocation,
    const wchar_t* const priFileOutputLocation,
    const wchar_t* const resourcePackName,
    std::vector<std::wstring>* const defaultLanguages,
    IDefStatusEx* const status)
{
    if (_ePackageState != FileWritten)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 1392, L"", 0);
        }
        return false;
    }

    if (format == MappingFileFormat::NoMapping)
    {
        return true;
    }
    if (format == MappingFileFormat::AppX)
    {
        return _GenerateAppXMappingFile(outputLocation, priFileOutputLocation, resourcePackName, defaultLanguages, status);
    }
    return false;
}

HRESULT CPackageInfo::_GetConsolidatedSink(CItemInstanceSink* const sink)
{
    HRESULT result = S_OK;
    for (std::uint32_t index = 0; index < _packageSink.GetNumberOfEntries(); ++index)
    {
        result = sink->AddEntry(_packageSink.GetEntry(index));
        if (FAILED(result))
        {
            break;
        }
    }
    for (std::uint32_t index = 0; SUCCEEDED(result) && (index < _readOnlyPackageSink.GetNumberOfEntries()); ++index)
    {
        result = sink->AddEntry(_readOnlyPackageSink.GetEntry(index));
    }
    return result;
}

bool CPackageInfo::_PopulateBuilder(CContentChecksumData* const contentChecksumData, IDefStatusEx* const status)
{
    std::uint32_t priFileFlags = 0;
    if (_options->GetShouldBuildAutoMergePri())
    {
        priFileFlags = 1;
    }
    if (_options->GetShouldBuildDeploymentMergeablePri())
    {
        priFileFlags |= 2;
    }

    Build::PriSectionBuilder* const priSectionBuilder = _pPriFileBuilder->GetDescriptor();
    Def_HrFailed0(priSectionBuilder->SetPriFileFlags(priFileFlags), status);
    if (_options->GetShouldDisableDeduplication())
    {
        struct BuildConfiguration : Microsoft::Resources::Build::DataItemOrchestrator
        {
            static constexpr MrmBuildConfiguration* DataItemOrchestrator::* Get() { return &BuildConfiguration::m_buildConfiguration; }
        };
        MrmBuildConfiguration* const buildConfiguration = priSectionBuilder->GetDataItemOrchestrator()->*BuildConfiguration::Get();
        buildConfiguration->SetFlags(buildConfiguration->GetFlags() & ~MrmBuildConfiguration::UseDeduplicationFlag);
    }

    if (status->Succeeded())
    {
        bool addContentChecksumCandidates = false;
        bool calculateContentChecksum = false;
        if ((contentChecksumData != nullptr) && contentChecksumData->enabled && (contentChecksumData->qualifierValues != nullptr))
        {
            addContentChecksumCandidates = true;
            if (!contentChecksumData->checksumValueProvided)
            {
                calculateContentChecksum = true;
                _DisplayContentChecksumInformation(contentChecksumData, status);
            }
        }

        CItemInstanceSink itemInstances(addContentChecksumCandidates);
        if (SUCCEEDED(_GetConsolidatedSink(&itemInstances)))
        {
            if (_qualifierMap.empty())
            {
                CUtilities::GetQualifierStringMap(_pDecisionInfoBuilder, _pAtomPoolGroup, &itemInstances, &_qualifierMap, status, false);
            }

            if (status->Succeeded())
            {
                StringResult resourceName;
                CItemInstanceEntry* entry = itemInstances.PopEntry();
                std::set<int> qualifierSetIndices;
                while (entry != nullptr)
                {
                    if (status->Succeeded())
                    {
                        if (!Def_HrFailed0(DefStringResult_SetCopy(resourceName.GetStringResult(), entry->source.GetRef()), status) &&
                            !Def_HrFailed0(
                                DefStringResult_ConcatPathElement(resourceName.GetStringResult(), entry->itemName.GetRef(), L'\\'),
                                status) &&
                            !_IsItemContentChecksum(&resourceName, status))
                        {
                            if (std::wcsstr(resourceName.GetRef(), L"..\\") != nullptr)
                            {
                                status->SetError(E_DEF_IBC_INVALID_CANDIDATE, resourceName.GetRef(), 0, L"..\\");
                            }
                            else if (entry->link.GetRef() != nullptr)
                            {
                                Def_HrFailed0(priSectionBuilder->AddResourceLink(resourceName.GetRef(), entry->link.GetRef()), status);
                            }
                            else
                            {
                                const int qualifierSetIndex = entry->qualifierSetIndex;
                                Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder = nullptr;
                                if (_GetQsbFromQsi(qualifierSetIndex, priSectionBuilder, status, &qualifierSetBuilder))
                                {
                                    qualifierSetIndices.insert(qualifierSetIndex);
                                    status->DiagnosticLogA(
                                        "[Checksum] IIE [%S] uses qualifier set "
                                        "index [%d]",
                                        resourceName.GetRef(),
                                        qualifierSetIndex);

                                    bool useForChecksum = false;
                                    if (calculateContentChecksum)
                                    {
                                        useForChecksum = _QualifierSetAppliesForChecksumCalculation(
                                            &resourceName, qualifierSetBuilder, qualifierSetIndex, 0, contentChecksumData, status);
                                    }
                                    status->DiagnosticLogA(
                                        "[Checksum] Summary of IIE [%S] qsi (%d): "
                                        "name [use] value [%s]",
                                        resourceName.GetRef(),
                                        qualifierSetIndex,
                                        useForChecksum ? "use" : "skip");

                                    if (status->Succeeded())
                                    {
                                        std::uint32_t candidateChecksum = 0;
                                        std::uint32_t checksum = 0;
                                        if (calculateContentChecksum)
                                        {
                                            Def_HrFailed0(
                                                DefChecksum::ComputeStringChecksum(
                                                    contentChecksumData->contentChecksumValue, false, resourceName.GetRef(), &checksum),
                                                status);
                                            candidateChecksum = checksum;
                                        }

                                        if (status->Succeeded())
                                        {
                                            if (entry->resourceValueType == MrmEnvironment::ResourceValueType_EmbeddedData)
                                            {
                                                std::size_t dataSize = 0;
                                                const auto* const data = static_cast<const std::uint8_t*>(entry->blob.GetRef(&dataSize));
                                                status->DiagnosticLogA(
                                                    "Adding Candidate - %S, %S", resourceName.GetRef(), entry->value.GetRef());
                                                Def_HrFailed0(
                                                    priSectionBuilder->AddCandidateWithEmbeddedData(
                                                        nullptr,
                                                        resourceName.GetRef(),
                                                        entry->resourceValueType,
                                                        data,
                                                        static_cast<UINT>(dataSize),
                                                        qualifierSetBuilder),
                                                    status);
                                                if (status->Succeeded() && useForChecksum && status->Succeeded())
                                                {
                                                    candidateChecksum =
                                                        DefChecksum::ComputeChecksum(candidateChecksum, data, static_cast<UINT>(dataSize));
                                                }
                                            }
                                            else if (MrmEnvironment::IsPathResourceValueType(entry->resourceValueType))
                                            {
                                                const wchar_t* const value = entry->value.GetRef();
                                                status->DiagnosticLogA("Adding Candidate - %S, Path %S", resourceName.GetRef(), value);
                                                Def_HrFailed0(
                                                    priSectionBuilder->AddCandidateWithString(
                                                        nullptr,
                                                        resourceName.GetRef(),
                                                        entry->resourceValueType,
                                                        value,
                                                        qualifierSetBuilder),
                                                    status);
                                                if (status->Succeeded() && useForChecksum)
                                                {
                                                    StringResult path;
                                                    Def_HrFailed0(
                                                        DefStringResult_InitRef(path.GetStringResult(), entry->projectRoot), status);
                                                    if (status->Succeeded() &&
                                                        !Def_HrFailed0(
                                                            DefStringResult_ConcatPathElement(path.GetStringResult(), value, L'\\'),
                                                            status))
                                                    {
                                                        StringResult absolutePath;
                                                        if (SUCCEEDED(CUtilities::GetAbsolutePath(path.GetRef(), status, absolutePath)))
                                                        {
                                                            Def_HrFailed0(
                                                                DefChecksum::ComputeFileChecksum(
                                                                    candidateChecksum, absolutePath.GetRef(), &checksum),
                                                                status);
                                                            candidateChecksum = checksum;
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                const wchar_t* const value = entry->value.GetRef();
                                                status->DiagnosticLogA("Adding Candidate - %S, %S", resourceName.GetRef(), value);
                                                Def_HrFailed0(
                                                    priSectionBuilder->AddCandidateWithString(
                                                        nullptr,
                                                        resourceName.GetRef(),
                                                        entry->resourceValueType,
                                                        value,
                                                        qualifierSetBuilder),
                                                    status);
                                                if (status->Succeeded() && useForChecksum)
                                                {
                                                    Def_HrFailed0(
                                                        DefChecksum::ComputeStringChecksum(candidateChecksum, false, value, &checksum),
                                                        status);
                                                    candidateChecksum = checksum;
                                                }
                                            }
                                        }

                                        if (status->Succeeded() && calculateContentChecksum &&
                                            (contentChecksumData->operation == CContentChecksumData::ContentChecksumOperation::MainPackage))
                                        {
                                            status->DiagnosticLogA(
                                                "[Checksum] Using candidate [%S] qsi "
                                                "(%d) for checksum calculation. Entry "
                                                "checksum: [%u]. Current content "
                                                "checksum: [%u]",
                                                resourceName.GetRef(),
                                                qualifierSetIndex,
                                                candidateChecksum,
                                                contentChecksumData->contentChecksumValue);
                                            if (!contentChecksumData->checksumValueProvided)
                                            {
                                                ++contentChecksumData->checksumItemCount;
                                                contentChecksumData->contentChecksumValue = candidateChecksum;
                                            }
                                        }

                                        if (status->Failed())
                                        {
                                            std::wstring normalizedName(resourceName.GetRef());
                                            std::replace(normalizedName.begin(), normalizedName.end(), L'\\', L'/');
                                            const HRESULT error = status->GetHResult();
                                            if ((error == HRESULT_FROM_WIN32(ERROR_MRM_SCOPE_ITEM_CONFLICT)) ||
                                                (error == HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY)) || (error == E_DEF_INVALID_ARG))
                                            {
                                                status->SetError(error, normalizedName.c_str(), 0, L"..\\");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    entry = itemInstances.PopEntry();
                }

                if (status->Succeeded())
                {
                    if (calculateContentChecksum)
                    {
                        Build::HierarchicalSchemaSectionBuilder* const schema = priSectionBuilder->GetSchema();
                        if (schema != nullptr)
                        {
                            const std::uint16_t version[] {
                                schema->GetMajorVersion(),
                                schema->GetMinorVersion(),
                            };
                            const std::uint32_t checksum = _DefComputeCrc32(
                                contentChecksumData->contentChecksumValue,
                                reinterpret_cast<const BYTE*>(version),
                                static_cast<UINT32>(sizeof(version)));
                            status->DiagnosticLogA(
                                "[Checksum] Adding version major [%d] minor [%d] "
                                "schema checksum [%u] to checksum calculation. "
                                "Current value [%u]",
                                version[0],
                                version[1],
                                checksum,
                                contentChecksumData->contentChecksumValue);
                            if (!contentChecksumData->checksumValueProvided)
                            {
                                ++contentChecksumData->checksumItemCount;
                                contentChecksumData->contentChecksumValue = checksum;
                            }
                            status->DiagnosticLogA(
                                "[Checksum] Final content checksum after adding "
                                "schema version checksum [%u]",
                                contentChecksumData->contentChecksumValue);
                        }
                        if (contentChecksumData->operation == CContentChecksumData::ContentChecksumOperation::MainPackage)
                        {
                            status->DiagnosticLogA(
                                "[Checksum] [%d] candidates used for checksum "
                                "calculation",
                                contentChecksumData->checksumItemCount);
                        }
                    }
                    if (addContentChecksumCandidates)
                    {
                        _AddContentChecksumCandidates(priSectionBuilder, &qualifierSetIndices, contentChecksumData, status);
                    }
                }
                else
                {
                    while (!itemInstances.empty())
                    {
                        itemInstances.PopEntry();
                    }
                }
            }
        }
        return status->Succeeded();
    }
    return status->Succeeded();
}

bool CPackageInfo::_GetQsbFromQsi(
    const int qualifierSetIndex,
    Build::PriSectionBuilder* const priSectionBuilder,
    IDefStatusEx* const status,
    Build::DecisionInfoQualifierSetBuilder** const qualifierSetBuilder)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_GetQsbFromQsi");
    *qualifierSetBuilder = nullptr;

    const auto existing = _qsiToQsbMap.find(qualifierSetIndex);
    if (existing != _qsiToQsbMap.end())
    {
        *qualifierSetBuilder = existing->second;
        status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_GetQsbFromQsi", status->GetHResult());
        return true;
    }

    QualifierSetResult qualifierSet;
    QualifierResult qualifier;
    StringResult qualifierName;
    StringResult qualifierValue;
    Build::DecisionInfoQualifierSetBuilder* newQualifierSetBuilder = nullptr;
    int qualifierIndex = 0;
    Def_HrFailed0(
        Build::DecisionInfoQualifierSetBuilder::CreateInstance(
            static_cast<Build::DecisionInfoBuilder*>(priSectionBuilder->GetDecisionInfoBuilder()), &newQualifierSetBuilder),
        status);
    if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
    {
        for (int index = 0; index < qualifierSet.GetNumQualifiers(); ++index)
        {
            if (!status->Succeeded())
            {
                break;
            }
            if (!Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, &qualifierIndex), status))
            {
                Atom qualifierAtom {};
                if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierAtom), status) &&
                    !Def_HrFailed0(
                        _pUnifiedEnvironment->GetName(
                            UnifiedEnvironment::EnvironmentNamesType::QualifierNames, qualifierAtom, &qualifierName),
                        status) &&
                    !Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status))
                {
                    double fallbackScore {};
                    Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), status);
                    Def_HrFailed0(
                        newQualifierSetBuilder->AddQualifier(
                            qualifierName.GetRef(), qualifierValue.GetRef(), fallbackScore, &qualifierIndex),
                        status);
                }
            }
        }
    }

    if (status->Succeeded())
    {
        _qsiToQsbMap.insert({qualifierSetIndex, newQualifierSetBuilder});
        *qualifierSetBuilder = newQualifierSetBuilder;
    }
    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_GetQsbFromQsi", status->GetHResult());
    return status->Succeeded();
}

bool CPackageInfo::_GetLanguageQualifierValue(
    Build::DecisionInfoQualifierSetBuilder* const qualifierSetBuilder,
    StringResult* const result,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_GetLanguageQualifierValue");

    Atom languageAtom {};
    Def_HrFailed0(
        _pUnifiedEnvironment->GetAtom(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, L"Language", &languageAtom), status);
    for (int index = 0; (index < qualifierSetBuilder->GetNumQualifiers()) && status->Succeeded(); ++index)
    {
        QualifierResult qualifier;
        Def_HrFailed0(qualifierSetBuilder->GetQualifier(index, &qualifier, nullptr), status);
        if (status->Succeeded())
        {
            Atom qualifierAtom {};
            Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierAtom), status);
            if (status->Succeeded() && (qualifierAtom.GetPoolIndex() == languageAtom.GetPoolIndex()) &&
                (qualifierAtom.GetIndex() == languageAtom.GetIndex()))
            {
                Def_HrFailed0(qualifier.GetOperand2Literal(result), status);
                break;
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_GetLanguageQualifierValue", status->GetHResult());
    return status->Succeeded();
}

bool CPackageInfo::_GetLanguageQualifierValues(
    Build::PriSectionBuilder* const priSectionBuilder,
    std::set<int>* const qualifierSetIndices,
    std::set<std::wstring>* const result,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_GetLanguageQualifierValues");

    for (const int qualifierSetIndex : *qualifierSetIndices)
    {
        Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder = nullptr;
        StringResult language;
        if (_GetQsbFromQsi(qualifierSetIndex, priSectionBuilder, status, &qualifierSetBuilder) &&
            _GetLanguageQualifierValue(qualifierSetBuilder, &language, status))
        {
            const wchar_t* const value = language.GetRef();
            if ((value != nullptr) && (*value != L'\0'))
            {
                const std::wstring languageValue(value);
                status->DiagnosticLogA(
                    "QualifierSetIndex [%d] has language qualifier value: [%S]", qualifierSetIndex, languageValue.c_str());
                result->insert(languageValue);
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_GetLanguageQualifierValues", status->GetHResult());
    return status->Succeeded();
}

bool CPackageInfo::_IsItemContentChecksum(StringResult* const itemName, IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_IsItemContentChecksum");

    std::size_t firstReplacement {};
    Def_HrFailed0(DefStringResult_ReplaceAll(itemName->GetStringResult(), L'\\', L'/', &firstReplacement), status);
    if (!status->Succeeded())
    {
        status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_IsItemContentChecksum", status->GetHResult());
        return false;
    }

    const bool result =
        itemName->CompareWithOptions(L"MicrosoftInternalMetadata/ContentChecksumValue", DefCompare_CaseInsensitive) == Def_Equal;
    if (result)
    {
        status->DiagnosticLogA(
            "Checksum item already part of the candidates with value [%S] for checksum "
            "calculation, it is a checksum value",
            itemName->GetRef());
    }
    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_IsItemContentChecksum", status->GetHResult());
    return result;
}

bool CPackageInfo::_QualifierSetAppliesForChecksumCalculation(
    StringResult* const itemName,
    Build::DecisionInfoQualifierSetBuilder* const qualifierSetBuilder,
    const int qualifierSetIndex,
    const int fallbackQualifierSetIndex,
    CContentChecksumData* const contentChecksumData,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA(
        "Start - ",
        "Microsoft::Resources::Indexers::CPackageInfo::"
        "_QualifierSetAppliesForChecksumCalculation");

    bool result = false;
    StringResult language;
    if ((qualifierSetIndex != fallbackQualifierSetIndex) && _GetLanguageQualifierValue(qualifierSetBuilder, &language, status))
    {
        bool isDefaultLanguage = false;
        if (_LanguageIsDefaultLanguage(&language, contentChecksumData->qualifierValues, &isDefaultLanguage, status))
        {
            result = isDefaultLanguage;
        }
    }

    status->DiagnosticLogA(
        "[Checksum] [%S] QualifierSet index [%d] [%s] used for value checksum calculation",
        itemName->GetRef(),
        qualifierSetIndex,
        result ? "will be" : "will not be");
    status->DiagnosticLogWithErrorCodeA(
        "Microsoft::Resources::Indexers::CPackageInfo::"
        "_QualifierSetAppliesForChecksumCalculation",
        status->GetHResult());
    return result;
}

bool CPackageInfo::_LanguageIsDefaultLanguage(
    StringResult* const language,
    std::vector<std::wstring>* const defaultLanguages,
    bool* const result,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_LanguageIsDefaultLanguage");

    *result = false;
    if (!language->IsEmpty())
    {
        const std::wstring normalizedLanguage = CUtilities::NormalizeLanguageTag(std::wstring(language->GetRef()));
        for (const std::wstring& defaultLanguage : *defaultLanguages)
        {
            const std::wstring normalizedDefaultLanguage = CUtilities::NormalizeLanguageTag(std::wstring(defaultLanguage.c_str()));
            if (CompareBcp47Tags(normalizedLanguage.c_str(), normalizedDefaultLanguage.c_str()) == 0)
            {
                *result = true;
                break;
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::_LanguageIsDefaultLanguage", status->GetHResult());
    return status->Succeeded();
}

void CPackageInfo::_DisplayContentChecksumInformation(CContentChecksumData* const contentChecksumData, IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_DisplayContentChecksumInformation");

    if (contentChecksumData == nullptr)
    {
        status->DiagnosticLogWithErrorCodeA(
            "Microsoft::Resources::Indexers::CPackageInfo::"
            "_DisplayContentChecksumInformation",
            status->GetHResult());
        status->DiagnosticLogA("contentChecksumData is NULL");
        return;
    }

    status->DiagnosticLogA(
        "[Checksum] Executing content checksum operation [%d]. Neutral Candidate Creation "
        "[%d]",
        static_cast<int>(contentChecksumData->operation),
        static_cast<int>(contentChecksumData->neutralLanguageCandidateCreation));
    if (contentChecksumData->operation != CContentChecksumData::ContentChecksumOperation::MainPackage)
    {
        status->DiagnosticLogA("[Checksum] Using checksum value: [%d]", contentChecksumData->contentChecksumValue);
    }
    else if (contentChecksumData->qualifierValues != nullptr)
    {
        std::wstring languages;
        for (auto language = contentChecksumData->qualifierValues->begin(); language != contentChecksumData->qualifierValues->end();
             ++language)
        {
            if (language != contentChecksumData->qualifierValues->begin())
            {
                languages.append(L";");
            }
            languages.append(*language, 0, std::wstring::npos);
        }
        status->DiagnosticLogA("[Checksum] Generation of content checksum enabled for default languages [%S]", languages.c_str());
    }
    else
    {
        status->DiagnosticLogA(
            "[Checksum] Generation of content checksum disabled because defaultLanguages "
            "is NULL");
    }

    status->DiagnosticLogWithErrorCodeA(
        "Microsoft::Resources::Indexers::CPackageInfo::_DisplayContentChecksumInformation", status->GetHResult());
}

bool CPackageInfo::_AddContentChecksumCandidates(
    Build::PriSectionBuilder* const priSectionBuilder,
    std::set<int>* const qualifierSetIndices,
    CContentChecksumData* const contentChecksumData,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::_AddContentChecksumCandidates");

    if (contentChecksumData->enabled && (contentChecksumData->qualifierValues != nullptr))
    {
        StringResult checksumValue;
        if (Def_HrFailed0(checksumValue.SetCopyInteger(contentChecksumData->contentChecksumValue), status))
        {
            status->SetError(E_DEF_INTERNAL_ERROR, L"" __FILE__, 1123, L"", 0);
            status->DiagnosticLogWithErrorCodeA(
                "Microsoft::Resources::Indexers::CPackageInfo::"
                "_AddContentChecksumCandidates",
                status->GetHResult());
            return false;
        }

        bool foundCandidates = false;
        if (contentChecksumData->operation == CContentChecksumData::ContentChecksumOperation::MainPackage)
        {
            std::list<int> languageQualifierSetIndices;
            if (CUtilities::GetLanguageOnlyQualifierSetIndexList(
                    _pDecisionInfoBuilder, _pAtomPoolGroup, &languageQualifierSetIndices, status))
            {
                foundCandidates = !languageQualifierSetIndices.empty();
                for (const int qualifierSetIndex : languageQualifierSetIndices)
                {
                    status->DiagnosticLogA(
                        "Creating the content checksum candidate using the qualifier set "
                        "index [%d]",
                        qualifierSetIndex);
                    Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder = nullptr;
                    if (_GetQsbFromQsi(qualifierSetIndex, priSectionBuilder, status, &qualifierSetBuilder))
                    {
                        Def_HrFailed0(
                            priSectionBuilder->AddCandidateWithString(
                                nullptr,
                                L"MicrosoftInternalMetadata/ContentChecksumValue",
                                static_cast<MrmEnvironment::ResourceValueType>(0),
                                checksumValue.GetRef(),
                                qualifierSetBuilder),
                            status);
                    }
                }
            }
        }
        else
        {
            std::map<std::wstring, int> languageQualifierSetIndices;
            std::set<std::wstring> languageQualifierValues;
            if (CUtilities::GetLanguageOnlyQualifierSetMap(_pDecisionInfoBuilder, _pAtomPoolGroup, &languageQualifierSetIndices, status) &&
                _GetLanguageQualifierValues(priSectionBuilder, qualifierSetIndices, &languageQualifierValues, status))
            {
                foundCandidates = !languageQualifierSetIndices.empty();
                for (const std::wstring& language : languageQualifierValues)
                {
                    const auto qualifierSetIndex = languageQualifierSetIndices.find(language);
                    if (qualifierSetIndex != languageQualifierSetIndices.end())
                    {
                        Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder = nullptr;
                        if (_GetQsbFromQsi(qualifierSetIndex->second, priSectionBuilder, status, &qualifierSetBuilder))
                        {
                            status->DiagnosticLogA(
                                "Creating the content checksum candidate using the qualifier "
                                "set index [%d]",
                                qualifierSetIndex->second);
                            Def_HrFailed0(
                                priSectionBuilder->AddCandidateWithString(
                                    nullptr,
                                    L"MicrosoftInternalMetadata/"
                                    "ContentChecksumValue",
                                    static_cast<MrmEnvironment::ResourceValueType>(0),
                                    checksumValue.GetRef(),
                                    qualifierSetBuilder),
                                status);
                        }
                    }
                }
            }
        }

        if (((contentChecksumData->operation != CContentChecksumData::ContentChecksumOperation::ResourcePackage) &&
             (contentChecksumData->neutralLanguageCandidateCreation == CContentChecksumData::NeutralLanguageCandidateCreation::Always)) ||
            ((contentChecksumData->neutralLanguageCandidateCreation ==
              CContentChecksumData::NeutralLanguageCandidateCreation::IfNoCandidates) &&
             !foundCandidates))
        {
            status->DiagnosticLogA("Creating the content checksum candidate using the empty decision qualifier");
            Build::DecisionInfoQualifierSetBuilder* qualifierSetBuilder = nullptr;
            if (_GetQsbFromQsi(0, priSectionBuilder, status, &qualifierSetBuilder))
            {
                Def_HrFailed0(
                    priSectionBuilder->AddCandidateWithString(
                        nullptr,
                        L"MicrosoftInternalMetadata/ContentChecksumValue",
                        static_cast<MrmEnvironment::ResourceValueType>(0),
                        checksumValue.GetRef(),
                        qualifierSetBuilder),
                    status);
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA(
        "Microsoft::Resources::Indexers::CPackageInfo::_AddContentChecksumCandidates", status->GetHResult());
    return true;
}

StandalonePriFile* CPackageInfo::GetReader(IDefStatusEx* const status)
{
    if ((_ePackageState != BuilderCreated) && (_ePackageState != Finalized) && (_ePackageState != FileWritten))
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 1248, L"", 0);
        }
        return nullptr;
    }

    status->DiagnosticLogWithPrefixA("Start - ", "Microsoft::Resources::Indexers::CPackageInfo::GetReader");
    if (_ePackageState == BuilderCreated)
    {
        if (!Def_HrFailed0(_pPriFileBuilder->FinalizeAllSections(), status))
        {
            _ePackageState = Finalized;
        }
    }

    SchemaCollectionSchemaWrapper* schemaCollection = nullptr;
    StandalonePriFile* reader = nullptr;
    if (status->Succeeded())
    {
        if (_ePackageState == Finalized)
        {
            if (_pPriBuffer == nullptr)
            {
                Def_HrFailed0(_pPriFileBuilder->GenerateFileContents(&_pPriBuffer, &_cchBuffer), status);
            }

            if (status->Succeeded())
            {
                const ISchemaCollection* overrideSchemas = nullptr;
                if (_overrideSchemaFile == nullptr)
                {
                    if (_pMrmProfile->GetBuildConfiguration()->UseSchemaReference())
                    {
                        Def_HrFailed0(SchemaCollectionSchemaWrapper::CreateInstance(_pSchema, &schemaCollection), status);
                        overrideSchemas = schemaCollection;
                    }
                }

                if (overrideSchemas != nullptr)
                {
                    Def_HrFailed0(
                        StandalonePriFile::CreateInstance(
                            0, static_cast<const std::uint8_t*>(_pPriBuffer), _cchBuffer, _pMrmProfile, overrideSchemas, &reader),
                        status);
                }
                else
                {
                    Def_HrFailed0(
                        StandalonePriFile::CreateInstance(
                            0, static_cast<const std::uint8_t*>(_pPriBuffer), _cchBuffer, _pMrmProfile, &reader),
                        status);
                }
            }
        }
        else if (_ePackageState == FileWritten)
        {
            const ISchemaCollection* overrideSchemas = nullptr;
            if (_overrideSchemaFile == nullptr)
            {
                if (_pMrmProfile->GetBuildConfiguration()->UseSchemaReference())
                {
                    Def_HrFailed0(SchemaCollectionSchemaWrapper::CreateInstance(_pSchema, &schemaCollection), status);
                    overrideSchemas = schemaCollection;
                }
            }
            else
            {
                overrideSchemas = static_cast<const ISchemaCollection*>(static_cast<const PriFile*>(_overrideSchemaFile));
            }

            if (overrideSchemas != nullptr)
            {
                Def_HrFailed0(
                    StandalonePriFile::CreateInstance(0, _strIndexFileLocation.GetRef(), _pMrmProfile, overrideSchemas, &reader), status);
            }
            else
            {
                Def_HrFailed0(StandalonePriFile::CreateInstance(0, _strIndexFileLocation.GetRef(), _pMrmProfile, &reader), status);
            }
        }
    }

    if (status->Failed() && (reader != nullptr))
    {
        delete reader;
        reader = nullptr;
        status->DiagnosticLogA("GetReader() failed with DefStatus Error: %x", status->GetHResult());
    }
    status->DiagnosticLogWithErrorCodeA("Microsoft::Resources::Indexers::CPackageInfo::GetReader", status->GetHResult());
    return reader;
}

bool CPackageInfo::WriteStatusToStream(IDefStatusEx* const status)
{
    if (_ePackageState != FileWritten)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_IBC_INVALID_STATE, L"" __FILE__, 1331, L"", 0);
        }
        return false;
    }

    StandalonePriFile* const reader = GetReader(status);
    if (reader != nullptr)
    {
        const IHierarchicalSchema* schema = nullptr;
        Def_HrFailed0(reader->GetPrimarySchema(&schema), status);
        const std::uint16_t majorVersion = schema->GetMajorVersion();
        const std::uint16_t minorVersion = schema->GetMinorVersion();

        int candidateCount = 0;
        for (int index = 0; index < reader->GetNumResourceMaps(); ++index)
        {
            const IResourceMapBase* resourceMap = nullptr;
            Def_HrFailed0(reader->GetResourceMap(index, &resourceMap), status);
            candidateCount += resourceMap->GetTotalNumResourceValues();
        }

        StringResult priFileName;
        if ((ConstructPriFileName(&priFileName, status) &&
             Def_HrFailed0(_pIndexer->LogInfo(L"Resource File: %s.pri", priFileName.GetRef()), status)) ||
            Def_HrFailed0(_pIndexer->LogInfo(L"Version: %d.%d", majorVersion, minorVersion), status) ||
            Def_HrFailed0(_pIndexer->LogInfo(L"Resource Candidates: %d", candidateCount), status))
        {
            return false;
        }

        for (const auto& qualifier : _qualifierMap)
        {
            if (Def_HrFailed0(
                    _pIndexer->LogInfo(L"%s Qualifiers: %s", qualifier.first.c_str(), qualifier.second->wstrValues.c_str()), status))
            {
                return false;
            }
        }
        delete reader;
    }
    return status->Succeeded();
}

bool CPackageInfo::_GenerateAppXMappingFile(
    const wchar_t* const outputLocation,
    const wchar_t* const priFileOutputLocation,
    const wchar_t* const resourcePackName,
    std::vector<std::wstring>* const defaultLanguages,
    IDefStatusEx* const status)
{
    CItemInstanceSink itemInstances;
    _GetConsolidatedSink(&itemInstances);

    StringResult mappingFilePath;
    Def_HrFailed0(DefStringResult_InitRef(mappingFilePath.GetStringResult(), outputLocation), status);
    StringResult priFileName;
    if (status->Succeeded())
    {
        if (ConstructPriFileName(&priFileName, status))
        {
            if (!Def_HrFailed0(DefStringResult_ConcatPathElement(mappingFilePath.GetStringResult(), priFileName.GetRef(), L'\\'), status))
            {
                Def_HrFailed0(DefStringResult_Concat(mappingFilePath.GetStringResult(), L".map.txt"), status);
            }
        }
    }

    StringResult priFilePath;
    if (status->Succeeded())
    {
        if (priFileOutputLocation == nullptr)
        {
            Def_HrFailed0(priFilePath.SetRef(_strIndexFileLocation.GetRef()), status);
        }
        else if (
            !Def_HrFailed0(DefStringResult_SetCopy(priFilePath.GetStringResult(), priFileOutputLocation), status) &&
            !Def_HrFailed0(DefStringResult_ConcatPathElement(priFilePath.GetStringResult(), priFileName.GetRef(), L'\\'), status))
        {
            Def_HrFailed0(DefStringResult_Concat(priFilePath.GetStringResult(), L".pri"), status);
        }
    }

    if (status->Succeeded())
    {
        FILE* stream = nullptr;
        if (_wfopen_s(&stream, mappingFilePath.GetRef(), L"w, ccs=UTF-8") != 0)
        {
            HRESULT result = ErrnoToHResult(errno);
            if (result == E_FAIL)
            {
                result = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_FILE);
            }
            status->SetError(result, L"" __FILE__, 1751, L"", 0);
        }
        else
        {
            std::fwprintf(stream, L"[ResourceMetadata]\n");
            if (_bIsResourcePackage)
            {
                StringResult resourceId;
                if (_ConstructResourceId(status, &resourceId))
                {
                    std::fwprintf(stream, L"\"ResourceId\"\t\t\t\"%s\"\n", resourceId.GetRef());
                }
            }

            bool hasLanguageDimension = false;
            bool skipDimensions = false;
            if (resourcePackName != nullptr)
            {
                DEFCOMPARISON comparison = Def_Equal;
                DefStringResult_CompareWithOptions(
                    _strResourcePackName.GetStringResult(), resourcePackName, DefCompare_CaseInsensitive, &comparison);
                skipDimensions = comparison == Def_Equal;
            }
            if (!skipDimensions)
            {
                std::set<std::wstring> dimensions;
                if (_ConstructResourceDimensionList(status, &dimensions))
                {
                    for (const std::wstring& dimension : dimensions)
                    {
                        std::fwprintf(stream, L"\"ResourceDimensions\"\t\t\"%s\"\n", dimension.c_str());
                        if (!hasLanguageDimension)
                        {
                            hasLanguageDimension = dimension.find(L"language-", 0, 9) != std::wstring::npos;
                        }
                    }
                }
            }
            if (!hasLanguageDimension && (defaultLanguages != nullptr) && !defaultLanguages->empty() && (resourcePackName == nullptr))
            {
                for (const std::wstring& defaultLanguage : *defaultLanguages)
                {
                    std::wstring language(defaultLanguage);
                    std::transform(language.begin(), language.end(), language.begin(), std::towlower);
                    std::fwprintf(stream, L"\"ResourceDimensions\"\t\t\"language-%s\"\n", language.c_str());
                }
            }

            std::fwprintf(stream, L"\n");
            if (status->Succeeded())
            {
                std::fwprintf(stream, L"[Files]\n");
                StringResult absolutePriFilePath;
                if (SUCCEEDED(CUtilities::GetAbsolutePath(priFilePath.GetRef(), status, absolutePriFilePath)))
                {
                    std::fwprintf(stream, L"\"%s\"\t\t\t\"%s\"\n", absolutePriFilePath.GetRef(), L"resources.pri");
                }

                StringResult absoluteSourcePath;
                while (!itemInstances.empty())
                {
                    CItemInstanceEntry* const entry = itemInstances.PopEntry();
                    if (entry != nullptr)
                    {
                        const wchar_t* const resourcePath = entry->value.GetRef();
                        if ((resourcePath != nullptr) &&
                            (DefString_CompareWithOptions(resourcePath, L"appxmanifest.xml", DefCompare_CaseInsensitive) != 0) &&
                            SUCCEEDED(CUtilities::GetAbsolutePath(entry->projectRoot, status, absoluteSourcePath)) &&
                            MrmEnvironment::IsPathResourceValueType(entry->resourceValueType))
                        {
                            std::fwprintf(stream, L"\"%s\\%s\"\t\t\t\"%s\"\n", absoluteSourcePath.GetRef(), resourcePath, resourcePath);
                        }
                    }
                }
            }
            std::fclose(stream);
        }
    }

    if (status->Failed())
    {
        while (!itemInstances.empty())
        {
            itemInstances.PopEntry();
        }
    }
    return status->Succeeded();
}

bool CPackageInfo::_ConstructResourceDimensionList(IDefStatusEx* const status, std::set<std::wstring>* const dimensions)
{
    QualifierSetResult qualifierSet;
    QualifierResult qualifier;
    Atom qualifierAtom {};
    StringResult affinity;
    std::set<int> reportedQualifierIndices;

    for (const int qualifierSetIndex : _reportedQsiList)
    {
        if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
        {
            const int numberOfQualifiers = qualifierSet.GetNumQualifiers();
            if (numberOfQualifiers > 0)
            {
                for (int index = 0; index < numberOfQualifiers; ++index)
                {
                    if (!status->Succeeded())
                    {
                        break;
                    }
                    if (!Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, nullptr), status) &&
                        !Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierAtom), status))
                    {
                        const IBuildQualifierType* qualifierType = nullptr;
                        if (!Def_HrFailed0(_pUnifiedEnvironment->GetTypeOfQualifier(qualifierAtom, &qualifierType), status))
                        {
                            IBuildQualifierType::PackagingFlags packagingFlags {};
                            Def_HrFailed0(
                                qualifierType->GetPackagingInfo(
                                    &qualifier, _pMrmProfile->GetBuildConfiguration()->GetFlags(), nullptr, 0, &packagingFlags, &affinity),
                                status);
                            int qualifierIndex = 0;
                            Def_HrFailed0(qualifier.GetQualifierIndex(&qualifierIndex), status);
                            if (status->Succeeded() && ((packagingFlags & IBuildQualifierType::PackagingReportQualifier) != 0) &&
                                (reportedQualifierIndices.find(qualifierIndex) == reportedQualifierIndices.end()))
                            {
                                std::wstring qualifierTag;
                                if (CUtilities::GetQualifierTagFromQualifierIndex(
                                        _pDecisionInfoBuilder, _pAtomPoolGroup, qualifierIndex, status, qualifierTag))
                                {
                                    std::transform(qualifierTag.begin(), qualifierTag.end(), qualifierTag.begin(), std::towlower);
                                    dimensions->insert(qualifierTag);
                                    reportedQualifierIndices.insert(qualifierIndex);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return status->Succeeded();
}

bool CPackageInfo::_ConstructResourceId(IDefStatusEx* const status, StringResult* const result)
{
    if ((_pIndexer->_platformVersion != MrmPlatformVersionInternal::WindowsCoreVNext) || _pIndexer->_useLegacyPriFileName)
    {
        return _ConstructResourceIdHashed(_strPackageName.GetRef(), status, result);
    }

    switch (_pIndexer->_resourceIdCompressionLevel)
    {
    case 0:
        return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), _strResourcePackName.GetRef()), status);
    case 1:
        return _ConstructResourceIdTokenAndValues(_strResourcePackName.GetRef(), status, result);
    case 2:
        return _ConstructResourceIdValuesOnly(_strResourcePackName.GetRef(), status, result);
    case 3:
        return _ConstructResourceIdValuesOnlyHashed(_strResourcePackName.GetRef(), status, result);
    default:
        return false;
    }
}

bool CPackageInfo::_ConstructResourceIdTokenAndValues(
    const wchar_t* const resourcePackName,
    IDefStatusEx* const status,
    StringResult* const result)
{
    const std::wstring input(resourcePackName);
    std::wstring resourceId;
    std::size_t segmentStart = 0;
    int hyphenIndex = -1;
    for (std::size_t index = 0; index <= input.length(); ++index)
    {
        if ((index < input.length()) && (input[index] == L'-') && (hyphenIndex <= 0))
        {
            hyphenIndex = static_cast<int>(index);
        }
        if ((index == input.length()) || (input[index] == L'.'))
        {
            if (!resourceId.empty())
            {
                resourceId.append(L".");
            }
            if (hyphenIndex > 0)
            {
                const std::size_t hyphen = static_cast<std::size_t>(hyphenIndex);
                const std::wstring token = input.substr(segmentStart, hyphen - segmentStart);
                resourceId.append(_pIndexer->_qualifierNameTokenMap[token], 0, std::wstring::npos);
                resourceId.append(L"-");
            }
            const std::size_t valueStart = static_cast<std::size_t>(hyphenIndex + 1);
            const std::wstring value = input.substr(valueStart, index - valueStart);
            resourceId.append(value, 0, std::wstring::npos);
            hyphenIndex = 0;
            segmentStart = index + 1;
        }
    }
    return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), resourceId.c_str()), status);
}

bool CPackageInfo::_ConstructResourceIdValuesOnly(
    const wchar_t* const resourcePackName,
    IDefStatusEx* const status,
    StringResult* const result)
{
    const std::wstring input(resourcePackName);
    std::wstring resourceId;
    std::size_t segmentStart = 0;
    int hyphenIndex = -1;
    for (std::size_t index = 0; index <= input.length(); ++index)
    {
        if ((index < input.length()) && (input[index] == L'-') && (hyphenIndex <= 0))
        {
            hyphenIndex = static_cast<int>(index);
        }
        if ((index == input.length()) || (input[index] == L'.'))
        {
            if (!resourceId.empty())
            {
                resourceId.append(L".");
            }
            const std::size_t hyphen = static_cast<std::size_t>(hyphenIndex);
            const std::wstring token = input.substr(segmentStart, hyphen - segmentStart);
            static_cast<void>(token);
            const std::size_t valueStart = static_cast<std::size_t>(hyphenIndex + 1);
            const std::wstring value = input.substr(valueStart, index - valueStart);
            resourceId.append(value, 0, std::wstring::npos);
            hyphenIndex = 0;
            segmentStart = index + 1;
        }
    }
    return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), resourceId.c_str()), status);
}

bool CPackageInfo::_ConstructResourceIdValuesOnlyHashed(
    const wchar_t* const resourcePackName,
    IDefStatusEx* const status,
    StringResult* const result)
{
    StringResult resourceId;
    bool succeeded = false;
    if (_ConstructResourceIdValuesOnly(resourcePackName, status, result))
    {
        if (!Def_HrFailed0(DefStringResult_SetCopy(resourceId.GetStringResult(), result->GetRef()), status))
        {
            succeeded = _ConstructResourceIdHashed(resourceId.GetRef(), status, result);
        }
    }
    return succeeded;
}

bool CPackageInfo::_ConstructResourceIdHashed(const wchar_t* const resourceId, IDefStatusEx* const status, StringResult* const result)
{
    if (std::wcslen(resourceId) <= 30)
    {
        return !Def_HrFailed0(result->SetRef(resourceId), status);
    }

    const std::uint32_t checksum = _DefComputeStringCrc32(0xABCD, false, resourceId, static_cast<std::uint32_t>(std::wcslen(resourceId)));
    wchar_t hashedResourceId[32] {};
    if (SUCCEEDED(StringCchPrintfW(hashedResourceId, 31, L"%.22s%08I32x", resourceId, checksum)))
    {
        return !Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), hashedResourceId), status);
    }
    if (status != nullptr)
    {
        status->SetError(static_cast<HRESULT>(GetLastError()), L"" __FILE__, 1549, L"", 0);
    }
    return false;
}

} // namespace Microsoft::Resources::Indexers
