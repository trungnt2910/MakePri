#include "StdAfx.h"

#include <CPriFileIndexer.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* const CPriFileIndexer::s_pszPriSchema =
    LR"xml(<xs:schema id="prifile" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigPriType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((p|P)(r|R)(i|I))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute name="type" type="IndexerConfigPriType" use="required"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

HRESULT CPriFileIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    _pEnvironment = pEnvironment;
    _pProjectRoot = pProjectRootFolder;
    _pQualApplicator = pApplicator;
    _options = options;

    if (pIndexPassNode == nullptr)
    {
        return E_FAIL;
    }

    CXmlHelper helper(pIndexPassNode);
    return helper.ValidateChildNodeAgainstChildSchema(L"indexer-config", s_pszPriSchema, L"type", L"pri", false, pStatus);
}

HRESULT CPriFileIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus,
    bool* const pbRemoveContainerFromIndex)
{
    DefStatus unusedStatus;
    HRESULT result = S_OK;

    if (pEntry->resourceItemType == MrmEnvironment::ResourceItemType_PriFile)
    {
        *pbRemoveContainerFromIndex = true;
        result = _ReadPriFileContentsFromBlob(pEntry->valueTypeName.GetRef(), pEntry, pStatus, pTraversalSink);
        return ComputeHResult(result, pStatus);
    }

    if (pbRemoveContainerFromIndex == nullptr)
    {
        return E_FAIL;
    }

    if (DefString_CompareWithOptions(pEntry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == Def_Equal)
    {
        *pbRemoveContainerFromIndex = false;
        const wchar_t* const path = pEntry->value.GetRef();
        const std::uint32_t length = static_cast<std::uint32_t>(wcslen(path));
        if ((length > 4) && (DefString_CompareWithOptions(&path[length - 4], L".pri", DefCompare_CaseInsensitive) == Def_Equal))
        {
            const wchar_t* accessiblePath = nullptr;
            result = CUtilities::GetPathInAccessibleFormat(_pProjectRoot, path, pStatus, &accessiblePath);
            if (SUCCEEDED(result))
            {
                if (PathFileExistsW(accessiblePath))
                {
                    pStatus->SetErrorLocation(accessiblePath);
                    result = Redirect(accessiblePath, pEntry, pTraversalSink, pStatus);
                    if (SUCCEEDED(result))
                    {
                        pStatus->ResetErrorLocation();
                        *pbRemoveContainerFromIndex = true;
                    }
                }
                else
                {
                    pStatus->SetError(E_DEF_FILE_NOT_FOUND, accessiblePath);
                }
            }
            ::operator delete(const_cast<wchar_t*>(accessiblePath));
        }
    }
    return result;
}

HRESULT CPriFileIndexer::Redirect(
    const wchar_t* const path,
    CItemInstanceEntry* const entry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    void* data = nullptr;
    std::size_t dataSize = 0;
    HRESULT result = BaseFile::LoadFileData(path, &dataSize, &data);
    if (Def_HrFailed0(result, status))
    {
        return ComputeHResult(S_OK, status);
    }

    BlobResult value;
    if (Def_HrFailed0(value.SetContents(data, dataSize), status))
    {
        return ComputeHResult(S_OK, status);
    }

    AutoDeletePtr<CItemInstanceEntry> redirected(
        CItemInstanceEntry::NewForEmbeddedData(
            entry->source.GetRef(),
            entry->itemName.GetRef(),
            MrmEnvironment::ResourceItemType_PriFile,
            MrmEnvironment::ResourceValueType_EmbeddedData,
            &value,
            entry->qualifierSetIndex,
            2,
            path,
            nullptr,
            status));
    if (redirected.Data() != nullptr)
    {
        result = sink->AddEntry(redirected.Data());
        if (SUCCEEDED(result))
        {
            redirected.Detach();
        }
    }
    else
    {
        result = status->GetHResult();
    }
    return result;
}

HRESULT CPriFileIndexer::_ReadNamedResult(
    NamedResourceResult* const pNamedResourceResult,
    const wchar_t* const pszParentScope,
    const wchar_t* const pszItemName,
    AtomPoolGroup* const pPoolGroup,
    CItemInstanceEntry* const pEntry,
    const wchar_t* const pszFileName,
    IDefStatusEx* const pStatus,
    CItemInstanceSink* const pTraversalSink)
{
    static_cast<void>(pEntry);
    HRESULT result = S_OK;
    const IHierarchicalSchema* linkedSchema;
    std::uint32_t linkedResourceIndex;
    if (pNamedResourceResult->TryGetResourceLink(&linkedSchema, &linkedResourceIndex))
    {
        StringResult link;
        if (linkedSchema->GetItemNames()->TryGetString(static_cast<Atom::Index>(linkedResourceIndex), &link))
        {
            AutoDeletePtr<CItemInstanceEntry> entry(
                CItemInstanceEntry::NewForLink(pszParentScope, pszItemName, link.GetRef(), 1, pszFileName, nullptr, pStatus));
            if (entry.Data() != nullptr)
            {
                result = pTraversalSink->AddEntry(entry.Data());
                if (SUCCEEDED(result))
                {
                    entry.Detach();
                }
            }
            else
            {
                result = pStatus->GetHResult();
            }
        }
        if (FAILED(result))
        {
            return result;
        }
    }

    pStatus->Reset();
    for (int candidateIndex = 0; candidateIndex < pNamedResourceResult->GetNumCandidates(); ++candidateIndex)
    {
        ResourceCandidateResult candidate;
        QualifierSetResult qualifiers;
        StringResult value;
        Def_HrFailed0(pNamedResourceResult->GetCandidate(candidateIndex, &candidate), pStatus);
        Def_HrFailed0(candidate.GetQualifiers(&qualifiers), pStatus);

        CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
        int qualifierSetIndex = 0;
        result = _pQualApplicator->GetQualifierSetBuilder(0, pStatus, &builder);
        if (SUCCEEDED(result))
        {
            const int numQualifiers = qualifiers.GetNumQualifiers();
            for (int qualifierIndex = 0; qualifierIndex < numQualifiers; ++qualifierIndex)
            {
                QualifierResult qualifier;
                StringResult unused;
                StringResult qualifierName;
                StringResult qualifierValue;
                Atom qualifierNameAtom {};
                Def_HrFailed0(qualifiers.GetQualifier(qualifierIndex, &qualifier, nullptr), pStatus);
                Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierNameAtom), pStatus);
                pPoolGroup->TryGetString(qualifierNameAtom, &qualifierName);
                Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), pStatus);

                bool applied = false;
                double fallbackScore;
                Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), pStatus);
                double score = fallbackScore;
                int priority = qualifier.GetPriority();
                result = builder->_AddQualifier(
                    qualifierName.GetRef(),
                    qualifierValue.GetRef(),
                    &score,
                    &priority,
                    CQualifierApplicator::tagTOKEN_TYPE::tokenDefault,
                    &applied,
                    pStatus);
                if (SUCCEEDED(result))
                {
                    result = _pQualApplicator->ApplyQualifierSetFromBuilder(builder, pStatus, &qualifierSetIndex);
                }
            }
        }
        delete builder;

        if (FAILED(result))
        {
            continue;
        }

        MrmEnvironment::ResourceValueType valueType;
        if (Def_HrFailed0(candidate.GetResourceValueType(&valueType), pStatus))
        {
            result = pStatus->GetHResult();
            continue;
        }

        MrmEnvironment::ResourceItemType itemType;
        const HRESULT typeResult = MrmEnvironment::GetDefaultItemTypeForValueType(valueType, &itemType);
        if (Def_HrFailed0(typeResult, pStatus))
        {
            result = pStatus->GetHResult();
            continue;
        }

        if (valueType == MrmEnvironment::ResourceValueType_EmbeddedData)
        {
            BlobResult candidateBlob;
            if (!candidate.TryGetBlobValue(&candidateBlob))
            {
                return pStatus->GetHResult();
            }

            std::size_t blobSize = 0;
            const void* const blob = candidateBlob.GetRef(&blobSize);
            BlobResult blobCopy;
            Def_HrFailed0(DefBlobResult_SetCopy(blobCopy.GetBlobResult(), blob, blobSize), pStatus);

            AutoDeletePtr<CItemInstanceEntry> entry(
                CItemInstanceEntry::NewForEmbeddedData(
                    pszParentScope,
                    pszItemName,
                    MrmEnvironment::ResourceItemType_EmbeddedData,
                    valueType,
                    &blobCopy,
                    qualifierSetIndex,
                    1,
                    pszFileName,
                    nullptr,
                    pStatus));
            if (entry.Data() == nullptr)
            {
                return pStatus->GetHResult();
            }
            result = pTraversalSink->AddEntry(entry.Data());
            if (SUCCEEDED(result))
            {
                entry.Detach();
            }
        }
        else
        {
            if (!candidate.TryGetStringValue(&value))
            {
                return pStatus->GetHResult();
            }

            MrmEnvironment::ResourceValueType normalizedValueType = valueType;
            if (MrmEnvironment::IsPathResourceValueType(valueType))
            {
                normalizedValueType = MrmEnvironment::ResourceValueType_Utf16Path;
            }
            else if (
                (valueType == MrmEnvironment::ResourceValueType_Utf16String) ||
                (valueType == MrmEnvironment::ResourceValueType_Utf8String) || (valueType == MrmEnvironment::ResourceValueType_AsciiString))
            {
                normalizedValueType = MrmEnvironment::ResourceValueType_Utf16String;
            }

            AutoDeletePtr<CItemInstanceEntry> entry(
                CItemInstanceEntry::NewForString(
                    pszParentScope,
                    pszItemName,
                    itemType,
                    normalizedValueType,
                    value.GetRef(),
                    qualifierSetIndex,
                    1,
                    pszFileName,
                    nullptr,
                    pStatus));
            if (entry.Data() == nullptr)
            {
                return pStatus->GetHResult();
            }
            result = pTraversalSink->AddEntry(entry.Data());
            if (SUCCEEDED(result))
            {
                entry.Detach();
            }
        }
    }
    return result;
}

HRESULT CPriFileIndexer::_ReadPriFileContentsFromBlob(
    const wchar_t* const valueTypeName,
    CItemInstanceEntry* const entry,
    IDefStatusEx* const status,
    CItemInstanceSink* const sink)
{
    HRESULT result = S_OK;
    std::size_t dataSize = 0;
    const auto* const data = static_cast<const std::uint8_t*>(entry->blob.GetRef(&dataSize));

    MrmProfile* profileValue = nullptr;
    Def_HrFailed0(
        MrmProfile::ChooseDefaultProfile(
            MrmProfile::ProfileType::EmptyInit,
            MrmPlatformVersionInternal::DefaultPlatformVersion,
            nullptr,
            nullptr,
            nullptr,
            &profileValue),
        status);
    AutoDeletePtr<MrmProfile> profile(profileValue);

    StandalonePriFile* fileValue = nullptr;
    Def_HrFailed0(StandalonePriFile::CreateInstance(0, data, dataSize, profile.Data(), &fileValue), status);
    AutoDeletePtr<StandalonePriFile> file(fileValue);

    if (status->Succeeded())
    {
        result = _ReadPriFileObject(file.Data(), profile.Data(), valueTypeName, entry, status, sink);
    }
    else if (status->GetWhat() == E_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILES)
    {
        status->SetError(E_MRM_SCHEMALESS_PRI_LOAD_FAILED, L"" __FILE__, 262, valueTypeName, 0);
    }

    return ComputeHResult(result, status);
}

HRESULT CPriFileIndexer::_ReadPriFileObject(
    StandalonePriFile* const file,
    MrmProfile* const profile,
    const wchar_t* const valueTypeName,
    CItemInstanceEntry* const parentEntry,
    IDefStatusEx* const status,
    CItemInstanceSink* const sink)
{
    HRESULT result = S_OK;
    UnifiedResourceView* resourceViewValue = nullptr;
    if (profile != nullptr)
    {
        Def_HrFailed0(UnifiedResourceView::CreateInstance(profile, &resourceViewValue), status);
    }
    AutoDeletePtr<UnifiedResourceView> resourceView(resourceViewValue);

    const PriFile* priFile = static_cast<PriFile*>(file);
    if (file->GetPriDescriptor()->GetNumSchemas() > 0)
    {
        if (resourceView.Data() != nullptr)
        {
            Def_HrFailed0(resourceView.Data()->SetApplicationPriFile(valueTypeName, nullptr, &priFile), status);
        }
        else
        {
            priFile = nullptr;
        }
    }

    if (priFile != nullptr)
    {
        const IResourceMapBase* primaryMapInterface = nullptr;
        Def_HrFailed0(priFile->GetPrimaryResourceMap(&primaryMapInterface), status);
        const auto* const primaryMap = static_cast<const ResourceMapBase*>(primaryMapInterface);
        if ((primaryMapInterface == nullptr) || (primaryMap == nullptr))
        {
            if (status->Succeeded())
            {
                status->SetError(E_DEFFILE_FORMAT_ERROR, L"" __FILE__, 331, L"", 0);
            }
        }
        else
        {
            result = _ReadRecursiveScopeTree(
                primaryMap->GetRootSubtree(), priFile->GetAtoms(), parentEntry, 0, nullptr, nullptr, valueTypeName, status, sink);
            for (int mapIndex = 0; (mapIndex < priFile->GetNumResourceMaps()) && SUCCEEDED(result); ++mapIndex)
            {
                const IResourceMapBase* mapInterface = nullptr;
                Def_HrFailed0(priFile->GetResourceMap(mapIndex, &mapInterface), status);
                const auto* const map = static_cast<const ResourceMapBase*>(mapInterface);
                if ((mapInterface == nullptr) || (map == nullptr))
                {
                    if (status->Succeeded() || (status->GetWhat() == E_DEF_INVALID_ARG))
                    {
                        status->SetError(E_DEFFILE_FORMAT_ERROR, L"" __FILE__, 316, L"", 0);
                    }
                    break;
                }
                if (map != primaryMap)
                {
                    result = _ReadRecursiveScopeTree(
                        map->GetRootSubtree(), priFile->GetAtoms(), parentEntry, 0, nullptr, nullptr, valueTypeName, status, sink);
                }
            }
        }
    }
    return ComputeHResult(result, status);
}

HRESULT CPriFileIndexer::_ReadRecursiveScopeTree(
    const ResourceMapSubtree* const pResourceMapSubtree,
    AtomPoolGroup* const pPoolGroup,
    CItemInstanceEntry* const pEntry,
    const std::uint32_t nDepth,
    const wchar_t* pszParentScope,
    const wchar_t* const pszScopedItemName,
    const wchar_t* const pszFileName,
    IDefStatusEx* const pStatus,
    CItemInstanceSink* const pTraversalSink)
{
    HRESULT result = S_OK;
    HRESULT recursiveResult = S_OK;
    for (int childIndex = 0; childIndex < pResourceMapSubtree->GetNumChildren(); ++childIndex)
    {
        if (pResourceMapSubtree->ChildIsScope(childIndex))
        {
            const wchar_t* childSource = pszParentScope;
            StringResult childPath;
            StringResult childName;
            if (!Def_HrFailed0(pResourceMapSubtree->GetChildName(childIndex, &childName), pStatus))
            {
                const ResourceMapSubtree* childSubtree = nullptr;
                Def_HrFailed0(pResourceMapSubtree->GetChildScopeSubtree(childIndex, &childSubtree), pStatus);
                if (pStatus->Succeeded())
                {
                    if (nDepth != 0)
                    {
                        HRESULT pathResult;
                        if (nDepth == 1)
                        {
                            pathResult = DefStringResult_Concat(childPath.GetStringResult(), childName.GetRef());
                        }
                        else
                        {
                            Def_HrFailed0(DefStringResult_Concat(childPath.GetStringResult(), pszScopedItemName), pStatus);
                            pathResult = DefStringResult_ConcatPathElement(childPath.GetStringResult(), childName.GetRef(), L'\\');
                        }
                        Def_HrFailed0(pathResult, pStatus);
                    }
                    else
                    {
                        childSource = childName.GetRef();
                    }
                    recursiveResult = _ReadRecursiveScopeTree(
                        childSubtree,
                        pPoolGroup,
                        pEntry,
                        nDepth + 1,
                        childSource,
                        childPath.GetRef(),
                        pszFileName,
                        pStatus,
                        pTraversalSink);
                    delete childSubtree;
                }
                result = recursiveResult;
            }
        }
        else
        {
            StringResult childName;
            StringResult childPath;
            if (!Def_HrFailed0(pResourceMapSubtree->GetChildName(childIndex, &childName), pStatus))
            {
                NamedResourceResult namedResult;
                Def_HrFailed0(pResourceMapSubtree->GetChildResource(childIndex, &namedResult), pStatus);
                if (pStatus->Succeeded())
                {
                    HRESULT pathResult;
                    if (nDepth == 1)
                    {
                        pathResult = DefStringResult_Concat(childPath.GetStringResult(), childName.GetRef());
                    }
                    else
                    {
                        Def_HrFailed0(DefStringResult_Concat(childPath.GetStringResult(), pszScopedItemName), pStatus);
                        pathResult = DefStringResult_ConcatPathElement(childPath.GetStringResult(), childName.GetRef(), L'\\');
                    }
                    Def_HrFailed0(pathResult, pStatus);
                    result = _ReadNamedResult(
                        &namedResult, pszParentScope, childPath.GetRef(), pPoolGroup, pEntry, pszFileName, pStatus, pTraversalSink);
                    recursiveResult = result;
                }
                else
                {
                    result = recursiveResult;
                }
            }
        }
    }
    return ComputeHResult(result, pStatus);
}
} // namespace Microsoft::Resources::Indexers
