#include "StdAfx.h"

#include <MrmXml.h>

namespace Microsoft::Resources
{
namespace
{

HRESULT base64encodeU(
    const void* const data,
    const std::uint32_t dataSize,
    wchar_t* const encoded,
    const std::uint32_t encodedCapacity,
    std::uint32_t* const encodedSize)
{
    constexpr wchar_t Base64Digits[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    const std::uint32_t requiredSize = 4 * ((dataSize + 2) / 3) + 1;
    if (encodedSize != nullptr)
    {
        *encodedSize = requiredSize;
    }
    if (encodedCapacity == 0 && encoded == nullptr)
    {
        return S_OK;
    }
    if (encodedCapacity < requiredSize)
    {
        return E_OUTOFMEMORY;
    }

    const auto* const bytes = static_cast<const std::uint8_t*>(data);
    std::uint32_t dataIndex = 0;
    std::uint32_t encodedIndex = 0;
    while (dataIndex < dataSize)
    {
        const std::uint8_t byte1 = bytes[dataIndex++];
        std::uint8_t byte2 = 0;
        std::uint8_t byte3 = 0;
        if (dataIndex < dataSize)
        {
            byte2 = bytes[dataIndex++];
            if (dataIndex < dataSize)
            {
                byte3 = bytes[dataIndex++];
            }
        }

        encoded[encodedIndex] = Base64Digits[byte1 >> 2];
        encoded[encodedIndex + 1] = Base64Digits[(16 * (byte1 & 3)) | (byte2 >> 4)];
        encoded[encodedIndex + 2] = Base64Digits[(4 * (byte2 & 0xF)) | (byte3 >> 6)];
        encoded[encodedIndex + 3] = Base64Digits[byte3 & 0x3F];
        encodedIndex += 4;
    }

    if (dataSize % 3 == 1)
    {
        encoded[encodedIndex - 2] = L'=';
    }
    if (dataSize % 3 != 0)
    {
        encoded[encodedIndex - 1] = L'=';
    }
    encoded[encodedIndex] = L'\0';
    return S_OK;
}

} // namespace

bool StandalonePriFileXml::DumpPriFileToXmlFile(
    const wchar_t* const outputFileName,
    PriFile* const priFile,
    const Tools::MakePri::PriDumpType dumpType,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (outputFileName == nullptr || outputFileName[0] == L'\0')
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 159, L"outputFileName", 0);
        return false;
    }
    if (priFile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 160, L"priFile", 0);
        return false;
    }

    bool result = false;
    IXMLDOMDocument2* document = nullptr;
    if (SUCCEEDED(CXMLUtil::CreateXMLDocument(&document)))
    {
        if (document != nullptr)
        {
            result = DumpPriFile(document, priFile, dumpType, outputOptions, status);
            if (result)
            {
                const HRESULT writeResult = CXMLUtil::WriteXmlToFile(document, outputFileName);
                if (writeResult == E_ACCESSDENIED)
                {
                    status->SetError(E_DEF_ACCESS_DENIED, outputFileName, 0, nullptr, 0);
                }
                else if (FAILED(writeResult))
                {
                    status->SetError(writeResult, L"" __FILE__, 182, L"", 0);
                }
            }
        }
    }

    if (status->Failed())
    {
        result = false;
    }
    if (document != nullptr)
    {
        document->Release();
    }
    return result;
}

bool StandalonePriFileXml::DumpPriFile(
    IXMLDOMDocument2* const document,
    PriFile* const priFile,
    const Tools::MakePri::PriDumpType dumpType,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (priFile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 205, L"priFile", 0);
        return false;
    }

    bool result = false;
    if (document != nullptr)
    {
        switch (dumpType)
        {
        case Tools::MakePri::PriDumpType::Detailed:
            result = DumpDetailedXMLPriFile(document, nullptr, priFile, outputOptions, status);
            break;
        case Tools::MakePri::PriDumpType::Schema:
        {
            const IHierarchicalSchema* schema = nullptr;
            const HRESULT schemaResult = priFile->GetPrimarySchema(&schema);
            static_cast<void>(Def_HrFailed0(schemaResult, status));
            if (schema == nullptr)
            {
                if (status->Succeeded())
                {
                    status->SetError(E_MRM_NO_PRIMARY_INDEX, L"" __FILE__, 234, L"", 0);
                }
            }
            else
            {
                result = DumpResourceSchemaToXml(document, nullptr, schema, outputOptions, status);
            }
            break;
        }
        case Tools::MakePri::PriDumpType::Summary:
            result = DumpSummaryXMLPriFile(document, priFile, outputOptions, status);
            break;
        case Tools::MakePri::PriDumpType::Basic:
        default:
            result = DumpBasicXMLPriFile(document, nullptr, priFile, outputOptions, status);
            break;
        }
    }

    return status->Failed() ? false : result;
}

bool StandalonePriFileXml::DumpSummaryXMLPriFile(
    IXMLDOMDocument2* const document,
    PriFile* const priFile,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 298, L"xmlDoc", 0);
        return false;
    }
    if (priFile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 299, L"priFile", 0);
        return false;
    }

    bool succeeded = false;
    IXMLDOMNode* rootNode = nullptr;
    IXMLDOMNode* sectionNode = nullptr;
    IXMLDOMNode* detailNode = nullptr;
    StringResult text;

    const BaseFile* baseFile;
    HRESULT result = priFile->GetBaseFile(&baseFile);
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddElement(document, nullptr, L"PriRaw", &rootNode);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddElement(document, rootNode, L"Header", &detailNode);
    }

    const DEFFILE_HEADER* fileHeader = nullptr;
    if (SUCCEEDED(result))
    {
        fileHeader = baseFile->GetFileHeader();
        wchar_t* magicText;
        Def_HrFailed0(text.SetEmptyContents(9, &magicText, nullptr), status);
        if (magicText == nullptr)
        {
            result = E_FAIL;
        }
        else
        {
            for (std::uint32_t characterIndex = 0; characterIndex < 8; ++characterIndex)
            {
                magicText[characterIndex] = static_cast<std::uint8_t>(fileHeader->magic.bMagic[characterIndex]);
            }
            magicText[8] = L'\0';
        }
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"magic", text.GetRef());
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"major", fileHeader->majorVersion);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"minor", fileHeader->minorVersion);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"totalSize", fileHeader->cbTotal);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"tocNumEntries", static_cast<std::uint32_t>(fileHeader->sizeToc));
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, detailNode, L"descriptorIndex", static_cast<std::uint32_t>(fileHeader->descriptorIndex));
    }
    if (SUCCEEDED(result))
    {
        CXMLUtil::CleanupNode(&detailNode);
        result = CXMLUtil::AddElement(document, rootNode, L"Toc", &sectionNode);
    }

    int tocIndex = 0;
    while (SUCCEEDED(result) && tocIndex < fileHeader->sizeToc)
    {
        const DEFFILE_TOC_ENTRY* tocEntry = nullptr;
        baseFile->GetTocEntry(tocIndex, &tocEntry);
        if (tocEntry != nullptr)
        {
            result = CXMLUtil::AddElement(document, sectionNode, L"TocEntry", &detailNode);
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, detailNode, L"index", static_cast<std::uint32_t>(tocIndex));
            }

            wchar_t* typeText;
            if (SUCCEEDED(result))
            {
                Def_HrFailed0(text.SetEmptyContents(17, &typeText, nullptr), status);
                if (typeText == nullptr)
                {
                    result = E_FAIL;
                }
                else
                {
                    for (std::uint32_t characterIndex = 0; characterIndex < 16; ++characterIndex)
                    {
                        typeText[characterIndex] = static_cast<std::uint8_t>(tocEntry->type.bType[characterIndex]);
                    }
                    typeText[16] = L'\0';
                }
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, detailNode, L"type", text.GetRef());
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, detailNode, L"flags", tocEntry->flags);
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, detailNode, L"sectionFlags", tocEntry->sectionFlags);
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, detailNode, L"size", tocEntry->cbSectionTotal);
            }
            if (SUCCEEDED(result))
            {
                CXMLUtil::CleanupNode(&detailNode);
            }
        }
        ++tocIndex;
    }

    if (SUCCEEDED(result))
    {
        CXMLUtil::CleanupNode(&sectionNode);
        const PriDescriptor* const descriptor = priFile->GetPriDescriptor();
        if (descriptor != nullptr)
        {
            const IHierarchicalSchema* schema;
            int schemaIndex = 0;
            while (SUCCEEDED(result) && schemaIndex < descriptor->GetNumSchemas())
            {
                descriptor->GetSchema(schemaIndex, true, &schema);
                if (schema != nullptr)
                {
                    result = CXMLUtil::AddElement(document, rootNode, L"Schema", &sectionNode);
                    if (SUCCEEDED(result))
                    {
                        result = CXMLUtil::AddAttribute(document, sectionNode, L"uniqueId", schema->GetUniqueId());
                    }
                    if (SUCCEEDED(result))
                    {
                        result = CXMLUtil::AddAttribute(document, sectionNode, L"simpleName", schema->GetSimpleId());
                    }

                    if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x20) == 0)
                    {
                        int versionIndex = 0;
                        while (SUCCEEDED(result) && versionIndex < schema->GetNumVersionInfos())
                        {
                            const IHierarchicalSchemaVersionInfo* const versionInfo = schema->GetVersionInfo(versionIndex);
                            if (versionInfo != nullptr)
                            {
                                result = CXMLUtil::AddElement(
                                    document, sectionNode, versionIndex == 0 ? L"Version" : L"PriorVersion", &detailNode);
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(document, detailNode, L"major", versionInfo->GetMajorVersion());
                                }
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(document, detailNode, L"minor", versionInfo->GetMinorVersion());
                                }
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(document, detailNode, L"checksum", versionInfo->GetVersionChecksum());
                                }
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(
                                        document, detailNode, L"numScopes", static_cast<std::uint32_t>(versionInfo->GetNumScopes()));
                                }
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(
                                        document, detailNode, L"numItems", static_cast<std::uint32_t>(versionInfo->GetNumItems()));
                                }
                                if (SUCCEEDED(result))
                                {
                                    CXMLUtil::CleanupNode(&detailNode);
                                }
                            }
                            ++versionIndex;
                        }
                    }

                    if (SUCCEEDED(result))
                    {
                        CXMLUtil::CleanupNode(&sectionNode);
                    }
                }
                ++schemaIndex;
            }

            if ((outputOptions.GetFlags() & 0x10) == 0)
            {
                const ResourceMapBase* resourceMap;
                int resourceMapIndex = 0;
                while (SUCCEEDED(result) && resourceMapIndex < descriptor->GetNumResourceMaps())
                {
                    descriptor->GetResourceMap(resourceMapIndex, &resourceMap);
                    if (resourceMap != nullptr)
                    {
                        const IHierarchicalSchema* const mapSchema = resourceMap->GetSchema();
                        result = CXMLUtil::AddElement(document, rootNode, L"ResourceMap", &sectionNode);
                        if (SUCCEEDED(result))
                        {
                            result = CXMLUtil::AddAttribute(document, sectionNode, L"uniqueId", mapSchema->GetUniqueId());
                        }
                        if (SUCCEEDED(result))
                        {
                            result = CXMLUtil::AddAttribute(document, sectionNode, L"simpleName", mapSchema->GetSimpleId());
                        }

                        if (SUCCEEDED(result))
                        {
                            std::uint32_t dataSize = 0;
                            const auto* const mapHeader =
                                reinterpret_cast<const MRMFILE_RESOURCE_MAP_HEADER*>(resourceMap->GetData(&dataSize));
                            if (dataSize >= sizeof(MRMFILE_RESOURCE_MAP_HEADER))
                            {
                                result = CXMLUtil::AddAttribute(document, sectionNode, L"numTotalValues", mapHeader->numValues);
                                if (SUCCEEDED(result))
                                {
                                    result = CXMLUtil::AddAttribute(
                                        document, sectionNode, L"sizeInternalValueData", mapHeader->cbInternalValueData);
                                }
                                if (SUCCEEDED(result))
                                {
                                    result =
                                        CXMLUtil::AddAttribute(document, sectionNode, L"sizeLargeValueData", mapHeader->cbLargeItemData);
                                }
                            }
                        }
                        if (SUCCEEDED(result))
                        {
                            CXMLUtil::CleanupNode(&sectionNode);
                        }
                    }
                    ++resourceMapIndex;
                }
            }
        }
    }

    if (SUCCEEDED(result))
    {
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&detailNode);
    CXMLUtil::CleanupNode(&sectionNode);
    CXMLUtil::CleanupNode(&rootNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpRecursiveSchemaSubTree(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const IHierarchicalSchema* const schema,
    const int scopeIndex,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    bool succeeded = false;
    IXMLDOMNode* scopeNode = nullptr;
    IXMLDOMNode* resourceNode = nullptr;

    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 435, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 436, L"xmlNode", 0);
        return false;
    }
    if (schema == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 437, L"schema", 0);
        return false;
    }

    StringResult scopeName;
    StringResult childName;
    int numChildren;
    if (schema->TryGetScopeInfo(scopeIndex, &scopeName, &numChildren))
    {
        int childIndex = 0;
        while (childIndex < numChildren)
        {
            int childScopeIndex = -1;
            int childItemIndex = -1;
            if (schema->TryGetScopeChild(scopeIndex, childIndex, &childScopeIndex, &childItemIndex) && childItemIndex >= 0 &&
                schema->TryGetScopeChildName(scopeIndex, childIndex, &childName))
            {
                HRESULT result = CXMLUtil::AddElement(document, parent, L"NamedResource", &resourceNode);
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddAttribute(document, resourceNode, L"name", childName.GetRef());
                }
                if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
                {
                    result = CXMLUtil::AddAttribute(document, resourceNode, L"index", static_cast<std::uint32_t>(childItemIndex));
                }
                if (FAILED(result))
                {
                    CXMLUtil::CleanupNode(&scopeNode);
                    CXMLUtil::CleanupNode(&resourceNode);
                    return false;
                }
                CXMLUtil::CleanupNode(&resourceNode);
            }

            if (!status->Succeeded())
            {
                CXMLUtil::CleanupNode(&scopeNode);
                CXMLUtil::CleanupNode(&resourceNode);
                return false;
            }
            ++childIndex;
        }

        childIndex = 0;
        while (childIndex < numChildren)
        {
            int childScopeIndex = -1;
            int childItemIndex = -1;
            if (schema->TryGetScopeChild(scopeIndex, childIndex, &childScopeIndex, &childItemIndex) && childScopeIndex > 0 &&
                schema->TryGetScopeChildName(scopeIndex, childIndex, &childName))
            {
                HRESULT result = CXMLUtil::AddElement(document, parent, L"ResourceMapSubtree", &scopeNode);
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddAttribute(document, scopeNode, L"name", childName.GetRef());
                }
                if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
                {
                    result = CXMLUtil::AddAttribute(document, scopeNode, L"index", static_cast<std::uint32_t>(childScopeIndex));
                }
                if (FAILED(result) || !DumpRecursiveSchemaSubTree(document, scopeNode, schema, childScopeIndex, outputOptions, status))
                {
                    CXMLUtil::CleanupNode(&scopeNode);
                    CXMLUtil::CleanupNode(&resourceNode);
                    return false;
                }
                CXMLUtil::CleanupNode(&scopeNode);
            }

            if (!status->Succeeded())
            {
                CXMLUtil::CleanupNode(&scopeNode);
                CXMLUtil::CleanupNode(&resourceNode);
                return false;
            }
            ++childIndex;
        }
    }

    succeeded = true;
    CXMLUtil::CleanupNode(&scopeNode);
    CXMLUtil::CleanupNode(&resourceNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpResourceSchemaToXml(
    IXMLDOMDocument2* const document,
    IXMLDOMNode*,
    const IHierarchicalSchema* const schema,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 516, L"xmlDoc", 0);
        return false;
    }
    if (schema == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 517, L"schema", 0);
        return false;
    }

    bool succeeded = false;
    IXMLDOMNode* priInfoNode = nullptr;
    IXMLDOMNode* resourceMapNode = nullptr;
    IXMLDOMNode* versionInfoNode = nullptr;
    IXMLDOMNode* unusedNode = nullptr;
    StringResult version;
    wchar_t number[12];

    HRESULT result = CXMLUtil::AddElement(document, nullptr, L"PriInfo", &priInfoNode);
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddElement(document, priInfoNode, L"ResourceMap", &resourceMapNode);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"name", schema->GetSimpleId());
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"uniqueName", schema->GetUniqueId());
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", schema->GetMajorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = DefStringResult_Concat(version.GetStringResult(), L".");
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", schema->GetMinorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"version", version.GetRef());
    }

    const IHierarchicalSchemaVersionInfo* currentVersion = nullptr;
    if (SUCCEEDED(result))
    {
        currentVersion = schema->GetVersionInfo(0);
        result = CXMLUtil::AddElement(document, resourceMapNode, L"VersionInfo", &versionInfoNode);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionInfoNode, L"version", version.GetRef());
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionInfoNode, L"checksum", currentVersion->GetVersionChecksum());
    }
    if (SUCCEEDED(result))
    {
        result =
            CXMLUtil::AddAttribute(document, versionInfoNode, L"numScopes", static_cast<std::uint32_t>(currentVersion->GetNumScopes()));
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionInfoNode, L"numItems", static_cast<std::uint32_t>(currentVersion->GetNumItems()));
    }

    int versionIndex = 1;
    while (SUCCEEDED(result) && versionIndex < schema->GetNumVersionInfos())
    {
        if (!DumpDetailedHierarchicalSchemaVersionInfo(
                document, versionInfoNode, schema->GetVersionInfo(versionIndex), versionIndex == 0, outputOptions, status))
        {
            result = E_FAIL;
        }
        ++versionIndex;
    }

    if (SUCCEEDED(result))
    {
        CXMLUtil::CleanupNode(&versionInfoNode);
        succeeded = DumpRecursiveSchemaSubTree(document, resourceMapNode, schema, 0, outputOptions, status);
    }

    CXMLUtil::CleanupNode(&unusedNode);
    CXMLUtil::CleanupNode(&resourceMapNode);
    CXMLUtil::CleanupNode(&priInfoNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpBasicQualifierSummary(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 673, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 674, L"xmlNode", 0);
        return false;
    }
    if (decisionInfo == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 675, L"decisionInfo", 0);
        return false;
    }
    if ((outputOptions.GetFlags() & 0x8) != 0)
    {
        return true;
    }

    bool succeeded = false;
    IXMLDOMNode* element = nullptr;
    std::map<std::wstring, Indexers::CUtilities::QualifierValues*> qualifierStrings;

    if (SUCCEEDED(Indexers::CUtilities::GetQualifierStringMap(decisionInfo, atomPoolGroup, nullptr, &qualifierStrings, status, true)))
    {
        auto iterator = qualifierStrings.begin();
        while (iterator != qualifierStrings.end())
        {
            if (FAILED(CXMLUtil::AddElement(document, parent, iterator->first.c_str(), &element)))
            {
                break;
            }

            const CXMLUtil::XmlUtilFlags flags = (outputOptions.GetFlags() & 0x1000) != 0 ?
                                                     CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                     CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings;
            if (FAILED(CXMLUtil::SetElementValue(element, iterator->second->wstrValues.c_str(), flags)))
            {
                break;
            }

            CXMLUtil::CleanupNode(&element);
            ++iterator;
        }

        if (iterator == qualifierStrings.end())
        {
            succeeded = true;
        }
    }

    CXMLUtil::CleanupNode(&element);
    for (auto iterator = qualifierStrings.begin(); iterator != qualifierStrings.end(); ++iterator)
    {
        delete iterator->second;
    }
    return succeeded;
}

bool StandalonePriFileXml::DumpBasicXMLPriFile(
    IXMLDOMDocument2* const document,
    IXMLDOMNode*,
    PriFile* const priFile,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 574, L"xmlDoc", 0);
        return false;
    }
    if (priFile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 575, L"priFile", 0);
        return false;
    }

    bool succeeded = false;
    IXMLDOMNode* priInfoNode = nullptr;
    if (SUCCEEDED(CXMLUtil::AddElement(document, nullptr, L"PriInfo", &priInfoNode)))
    {
        const IResourceMapBase* primaryResourceMap;
        if (SUCCEEDED(priFile->GetPrimaryResourceMap(&primaryResourceMap)) && status->Succeeded() &&
            DumpBasicResourceMap(
                document,
                priInfoNode,
                static_cast<const ResourceMapBase*>(primaryResourceMap),
                true,
                priFile->GetAtoms(),
                outputOptions,
                status))
        {
            int resourceMapIndex = 0;
            if (priFile->GetNumResourceMaps() <= 0)
            {
                succeeded = true;
            }
            else
            {
                bool resourceMapFailed = false;
                do
                {
                    const IResourceMapBase* resourceMap;
                    priFile->GetResourceMap(resourceMapIndex, &resourceMap);
                    if (resourceMap != primaryResourceMap && !DumpBasicResourceMap(
                                                                 document,
                                                                 priInfoNode,
                                                                 static_cast<const ResourceMapBase*>(resourceMap),
                                                                 false,
                                                                 priFile->GetAtoms(),
                                                                 outputOptions,
                                                                 status))
                    {
                        resourceMapFailed = true;
                        break;
                    }
                    ++resourceMapIndex;
                } while (resourceMapIndex < priFile->GetNumResourceMaps());

                if (!resourceMapFailed)
                {
                    succeeded = true;
                }
            }
        }
    }

    CXMLUtil::CleanupNode(&priInfoNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpBasicResourceMap(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const ResourceMapBase* const resourceMapBase,
    const bool primary,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 618, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 619, L"xmlNode", 0);
        return false;
    }
    if (resourceMapBase == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 620, L"resourceMapBase", 0);
        return false;
    }

    IXMLDOMNode* resourceMapNode = nullptr;
    IXMLDOMNode* qualifiersNode = nullptr;
    StringResult version;
    wchar_t number[14];

    HRESULT result = CXMLUtil::AddElement(document, parent, L"ResourceMap", &resourceMapNode);
    const IHierarchicalSchema* schema = nullptr;
    if (SUCCEEDED(result))
    {
        schema = resourceMapBase->GetSchema();
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"name", schema->GetSimpleId());
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", schema->GetMajorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = DefStringResult_Concat(version.GetStringResult(), L".");
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", schema->GetMinorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"version", version.GetRef());
    }
    if (SUCCEEDED(result) && primary)
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"primary", L"true");
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddElement(document, resourceMapNode, L"Qualifiers", &qualifiersNode);
    }

    bool succeeded = false;
    if (SUCCEEDED(result) &&
        DumpBasicQualifierSummary(document, qualifiersNode, resourceMapBase->GetDecisionInfo(), atomPoolGroup, outputOptions, status))
    {
        CXMLUtil::CleanupNode(&qualifiersNode);
        if (DumpBasicRecursiveResourceMapSubTree(
                document, resourceMapNode, resourceMapBase->GetRootSubtree(), atomPoolGroup, outputOptions, status))
        {
            CXMLUtil::CleanupNode(&resourceMapNode);
            succeeded = true;
        }
    }

    CXMLUtil::CleanupNode(&qualifiersNode);
    CXMLUtil::CleanupNode(&resourceMapNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpBasicRecursiveResourceMapSubTree(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const ResourceMapSubtree* const subtree,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    bool succeeded = false;
    IXMLDOMNode* subtreeNode = nullptr;

    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 722, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 723, L"xmlNode", 0);
        return false;
    }
    if (subtree == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 724, L"subtree", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 725, L"atomPoolGroup", 0);
        return false;
    }

    int childIndex = 0;
    if (subtree->GetNumChildren() > 0)
    {
        do
        {
            if (!subtree->ChildIsScope(childIndex))
            {
                NamedResourceResult resource;
                if (FAILED(subtree->GetChildResource(childIndex, &resource)))
                {
                    CXMLUtil::CleanupNode(&subtreeNode);
                    return false;
                }
                if (!DumpBasicNamedResource(document, parent, &resource, atomPoolGroup, outputOptions, status))
                {
                    CXMLUtil::CleanupNode(&subtreeNode);
                    return false;
                }
            }
            ++childIndex;
        } while (childIndex < subtree->GetNumChildren());
    }

    childIndex = 0;
    if (subtree->GetNumChildren() > 0)
    {
        do
        {
            if (subtree->ChildIsScope(childIndex))
            {
                StringResult childName;
                subtree->GetChildName(childIndex, &childName);

                const ResourceMapSubtree* childSubtree = nullptr;
                if (FAILED(CXMLUtil::AddElement(document, parent, L"ResourceMapSubtree", &subtreeNode)) ||
                    FAILED(CXMLUtil::AddAttribute(document, subtreeNode, L"name", childName.GetRef())) ||
                    FAILED(subtree->GetChildScopeSubtree(childIndex, &childSubtree)) ||
                    !DumpBasicRecursiveResourceMapSubTree(document, subtreeNode, childSubtree, atomPoolGroup, outputOptions, status))
                {
                    CXMLUtil::CleanupNode(&subtreeNode);
                    return false;
                }

                CXMLUtil::CleanupNode(&subtreeNode);
            }
            ++childIndex;
        } while (childIndex < subtree->GetNumChildren());
    }

    succeeded = true;
    CXMLUtil::CleanupNode(&subtreeNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpBasicNamedResource(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    NamedResourceResult* const namedResource,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 774, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 775, L"xmlNode", 0);
        return false;
    }
    if (namedResource == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 776, L"namedResource", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 777, L"atomPoolGroup", 0);
        return false;
    }

    IXMLDOMNode* resourceNode = nullptr;
    IXMLDOMNode* candidateNode = nullptr;
    IXMLDOMNode* valueNode = nullptr;
    IXMLDOMNode* resourceLinkNode = nullptr;
    StringResult itemLocalName;
    StringResult resourceName;
    StringResult resourceUri;
    bool succeeded = false;

    HRESULT result = namedResource->GetItemLocalName(&itemLocalName);
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddElement(document, parent, L"NamedResource", &resourceNode);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceNode, L"name", itemLocalName.GetRef());
    }
    if (SUCCEEDED(result))
    {
        result = namedResource->GetResourceName(&resourceName);
    }
    if (SUCCEEDED(result) && !ConstructUri(namedResource->GetParentSchema()->GetSimpleId(), resourceName.GetRef(), status, &resourceUri))
    {
        result = E_FAIL;
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceNode, L"uri", resourceUri.GetRef());
    }

    if (SUCCEEDED(result))
    {
        const IHierarchicalSchema* linkedSchema;
        std::uint32_t linkedResourceIndex;
        if (namedResource->TryGetResourceLink(&linkedSchema, &linkedResourceIndex))
        {
            StringResult linkedResourceName;
            if (!linkedSchema->GetItemNames()->TryGetString(static_cast<Atom::Index>(linkedResourceIndex), &linkedResourceName))
            {
                result = E_FAIL;
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddElement(document, resourceNode, L"Link", &resourceLinkNode);
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::SetElementValue(
                    resourceLinkNode,
                    linkedResourceName.GetRef(),
                    (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                               CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
            }
            if (SUCCEEDED(result))
            {
                CXMLUtil::CleanupNode(&resourceLinkNode);
            }
        }
    }

    int candidateIndex = 0;
    while (SUCCEEDED(result) && candidateIndex < namedResource->GetNumCandidates())
    {
        ResourceCandidateResult candidate;
        StringResult valueTypeText;
        StringResult qualifierList;
        QualifierSetResult qualifierSet;
        StringResult valueText;
        bool allFallback = true;

        result = namedResource->GetCandidate(candidateIndex, &candidate);
        if (SUCCEEDED(result))
        {
            result = candidate.GetQualifiers(&qualifierSet);
        }

        int qualifierIndex = 0;
        int numQualifiers = 0;
        if (SUCCEEDED(result))
        {
            numQualifiers = qualifierSet.GetNumQualifiers();
        }
        while (SUCCEEDED(result) && qualifierIndex < numQualifiers)
        {
            QualifierResult qualifier;
            StringResult qualifierPair;
            StringResult qualifierName;
            StringResult qualifierValue;
            DEFSTATUS qualifierDefStatus;
            DefStatusWrapper qualifierStatus(&qualifierDefStatus);
            Atom qualifierNameAtom {};

            result = qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr);
            if (SUCCEEDED(result))
            {
                result = qualifier.GetOperand1Attribute(&qualifierNameAtom);
            }
            if (SUCCEEDED(result) && !atomPoolGroup->TryGetString(qualifierNameAtom, &qualifierName))
            {
                result = E_FAIL;
            }
            if (SUCCEEDED(result))
            {
                result = qualifier.GetOperand2Literal(&qualifierValue);
            }
            if (SUCCEEDED(result))
            {
                result = DefStringResult_Concat(qualifierPair.GetStringResult(), qualifierName.GetRef());
            }
            if (SUCCEEDED(result))
            {
                result = DefStringResult_Concat(qualifierPair.GetStringResult(), L"-");
            }
            if (SUCCEEDED(result))
            {
                result = DefStringResult_Concat(qualifierPair.GetStringResult(), qualifierValue.GetRef());
            }
            if (SUCCEEDED(result))
            {
                if (qualifierList.GetRef() == nullptr)
                {
                    result = DefStringResult_Concat(qualifierList.GetStringResult(), qualifierPair.GetRef());
                }
                else if (wcsstr(qualifierList.GetRef(), qualifierPair.GetRef()) == nullptr)
                {
                    std::size_t qualifierListLength;
                    result = DefStringResult_GetLength(qualifierList.GetStringResult(), &qualifierListLength);
                    if (SUCCEEDED(result) && qualifierListLength != 0)
                    {
                        result = DefStringResult_Concat(qualifierList.GetStringResult(), L", ");
                    }
                    if (SUCCEEDED(result))
                    {
                        result = DefStringResult_Concat(qualifierList.GetStringResult(), qualifierPair.GetRef());
                    }
                }
            }
            if (SUCCEEDED(result))
            {
                if (!allFallback || qualifier.GetFallbackScoreAsScaledInt() <= 0)
                {
                    allFallback = false;
                }
                ++qualifierIndex;
            }
        }

        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddElement(document, resourceNode, L"Candidate", &candidateNode);
        }
        if (SUCCEEDED(result) && qualifierList.GetRef() != nullptr)
        {
            result = CXMLUtil::AddAttribute(document, candidateNode, L"qualifiers", qualifierList.GetRef());
        }
        if (SUCCEEDED(result) && allFallback && numQualifiers > 0)
        {
            result = CXMLUtil::AddAttribute(document, candidateNode, L"isDefault", L"true");
        }

        MrmEnvironment::ResourceValueType valueType;
        if (SUCCEEDED(result))
        {
            result = candidate.GetResourceValueType(&valueType);
        }
        if (SUCCEEDED(result))
        {
            result = MrmEnvironment::GetExternalStringForResourceValueType(valueType, &valueTypeText);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, candidateNode, L"type", valueTypeText.GetRef());
        }
        if (SUCCEEDED(result))
        {
            if (wcscmp(valueTypeText.GetRef(), L"EmbeddedData") == 0)
            {
                result = DefStringResult_SetCopy(valueText.GetStringResult(), L"[Embedded Binary Data]");
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddElement(document, candidateNode, L"Base64Value", &valueNode);
                }
            }
            else
            {
                if (!candidate.TryGetStringValue(&valueText))
                {
                    result = E_FAIL;
                }
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddElement(document, candidateNode, L"Value", &valueNode);
                }
            }
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::SetElementValue(
                valueNode,
                valueText.GetRef(),
                (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                           CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            CXMLUtil::CleanupNode(&candidateNode);
            ++candidateIndex;
        }
    }

    if (SUCCEEDED(result))
    {
        CXMLUtil::CleanupNode(&resourceNode);
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&valueNode);
    CXMLUtil::CleanupNode(&candidateNode);
    CXMLUtil::CleanupNode(&resourceNode);
    CXMLUtil::CleanupNode(&resourceLinkNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpDetailedQualifier(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    QualifierResult* const qualifier,
    AtomPoolGroup* const atomPoolGroup,
    bool,
    bool,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1131, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1132, L"xmlNode", 0);
        return false;
    }
    if (qualifier == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1133, L"qualifier", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1134, L"atomPoolGroup", 0);
        return false;
    }

    bool succeeded = false;
    IXMLDOMNode* qualifierNode = nullptr;
    if (SUCCEEDED(CXMLUtil::AddElement(document, parent, L"Qualifier", &qualifierNode)))
    {
        HRESULT result;
        {
            StringResult qualifierName;
            StringResult qualifierValue;
            double scoreAsDefault = 0.0;
            Atom qualifierNameAtom;

            result = qualifier->GetFallbackScore(&scoreAsDefault);
            if (SUCCEEDED(result))
            {
                result = qualifier->GetOperand1Attribute(&qualifierNameAtom);
            }
            if (SUCCEEDED(result) && !atomPoolGroup->TryGetString(qualifierNameAtom, &qualifierName))
            {
                result = E_FAIL;
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, qualifierNode, L"name", qualifierName.GetRef());
            }
            if (SUCCEEDED(result))
            {
                result = qualifier->GetOperand2Literal(&qualifierValue);
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, qualifierNode, L"value", qualifierValue.GetRef());
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, qualifierNode, L"priority", static_cast<std::uint32_t>(qualifier->GetPriority()));
            }
            if (SUCCEEDED(result))
            {
                wchar_t scoreText[5];
                const wchar_t* const format = (scoreAsDefault == 0.0 || scoreAsDefault == 1.0) ? L"%.1f" : L"%.2g";
                StringCchPrintfW(scoreText, 5, format, scoreAsDefault);
                result = CXMLUtil::AddAttribute(document, qualifierNode, L"scoreAsDefault", scoreText);
            }
        }

        if (SUCCEEDED(result))
        {
            if ((outputOptions.GetFlags() & 0x2) != 0)
            {
                succeeded = true;
            }
            else
            {
                int qualifierIndex;
                result = qualifier->GetQualifierIndex(&qualifierIndex);
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddAttribute(document, qualifierNode, L"index", static_cast<std::uint32_t>(qualifierIndex));
                }
                if (SUCCEEDED(result))
                {
                    succeeded = true;
                }
            }

            if (succeeded)
            {
                CXMLUtil::CleanupNode(&qualifierNode);
            }
        }
    }

    CXMLUtil::CleanupNode(&qualifierNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpDetailedQualifierInfo(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1032, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1033, L"xmlNode", 0);
        return false;
    }
    if (decisionInfo == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1034, L"decisionInfo", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1035, L"atomPoolGroup", 0);
        return false;
    }

    bool succeeded = false;
    bool failed = false;
    IXMLDOMNode* containerNode = nullptr;
    IXMLDOMNode* qualifierSetNode = nullptr;
    IXMLDOMNode* decisionNode = nullptr;

    if (FAILED(CXMLUtil::AddElement(document, parent, L"Qualifiers", &containerNode)))
    {
        failed = true;
    }

    for (int qualifierIndex = 1; !failed && qualifierIndex < decisionInfo->GetNumQualifiers(); ++qualifierIndex)
    {
        QualifierResult qualifier;
        if (FAILED(decisionInfo->GetQualifier(qualifierIndex, &qualifier)) ||
            !DumpDetailedQualifier(document, containerNode, &qualifier, atomPoolGroup, false, false, outputOptions, status))
        {
            failed = true;
        }
    }

    if (!failed)
    {
        CXMLUtil::CleanupNode(&containerNode);
        if (FAILED(CXMLUtil::AddElement(document, parent, L"QualifierSets", &containerNode)))
        {
            failed = true;
        }
    }

    for (int qualifierSetIndex = 1; !failed && qualifierSetIndex < decisionInfo->GetNumQualifierSets(); ++qualifierSetIndex)
    {
        QualifierSetResult qualifierSet;
        if (FAILED(decisionInfo->GetQualifierSet(qualifierSetIndex, &qualifierSet)) ||
            FAILED(CXMLUtil::AddElement(document, containerNode, L"QualifierSet", &qualifierSetNode)))
        {
            failed = true;
            continue;
        }

        if ((outputOptions.GetFlags() & 0x2) == 0 &&
            FAILED(CXMLUtil::AddAttribute(document, qualifierSetNode, L"index", static_cast<std::uint32_t>(qualifierSet.GetIndex()))))
        {
            failed = true;
            continue;
        }

        for (int qualifierIndex = 0; !failed && qualifierIndex < qualifierSet.GetNumQualifiers(); ++qualifierIndex)
        {
            QualifierResult qualifier;
            if (FAILED(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr)) ||
                !DumpDetailedQualifier(document, qualifierSetNode, &qualifier, atomPoolGroup, false, false, outputOptions, status))
            {
                failed = true;
            }
        }

        if (!failed)
        {
            CXMLUtil::CleanupNode(&qualifierSetNode);
        }
    }

    if (!failed)
    {
        CXMLUtil::CleanupNode(&containerNode);
        if (FAILED(CXMLUtil::AddElement(document, parent, L"Decisions", &containerNode)))
        {
            failed = true;
        }
    }

    for (int decisionIndex = 2; !failed && decisionIndex < decisionInfo->GetNumDecisions(); ++decisionIndex)
    {
        DecisionResult decision;
        if (FAILED(decisionInfo->GetDecision(decisionIndex, &decision)) ||
            FAILED(CXMLUtil::AddElement(document, containerNode, L"Decision", &decisionNode)))
        {
            failed = true;
            continue;
        }

        if ((outputOptions.GetFlags() & 0x2) == 0 &&
            FAILED(CXMLUtil::AddAttribute(document, decisionNode, L"index", static_cast<std::uint32_t>(decision.GetIndex()))))
        {
            failed = true;
            continue;
        }

        for (int qualifierSetIndex = 0; !failed && qualifierSetIndex < decision.GetNumQualifierSets(); ++qualifierSetIndex)
        {
            QualifierSetResult qualifierSet;
            if (FAILED(decision.GetQualifierSet(qualifierSetIndex, &qualifierSet, nullptr)) ||
                FAILED(CXMLUtil::AddElement(document, decisionNode, L"QualifierSet", &qualifierSetNode)))
            {
                failed = true;
                continue;
            }

            if ((outputOptions.GetFlags() & 0x2) == 0 &&
                FAILED(CXMLUtil::AddAttribute(document, qualifierSetNode, L"index", static_cast<std::uint32_t>(qualifierSet.GetIndex()))))
            {
                failed = true;
                continue;
            }

            const int numQualifiers = qualifierSet.GetNumQualifiers();
            for (int qualifierIndex = 0; !failed && qualifierIndex < numQualifiers; ++qualifierIndex)
            {
                QualifierResult qualifier;
                if (FAILED(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr)) ||
                    !DumpDetailedQualifier(document, qualifierSetNode, &qualifier, atomPoolGroup, false, false, outputOptions, status))
                {
                    failed = true;
                }
            }

            if (!failed)
            {
                CXMLUtil::CleanupNode(&qualifierSetNode);
            }
        }

        if (!failed)
        {
            CXMLUtil::CleanupNode(&decisionNode);
        }
    }

    if (!failed)
    {
        CXMLUtil::CleanupNode(&containerNode);
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&qualifierSetNode);
    CXMLUtil::CleanupNode(&decisionNode);
    CXMLUtil::CleanupNode(&containerNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpDetailedXMLPriFile(
    IXMLDOMDocument2* const document,
    IXMLDOMNode*,
    PriFile* const priFile,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 906, L"xmlDoc", 0);
        return false;
    }
    if (priFile == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 907, L"priFile", 0);
        return false;
    }

    bool succeeded = false;
    AutoDeletePtr<EnvironmentVersionInfo> environmentVersion;
    IXMLDOMNode* priInfoNode = nullptr;
    IXMLDOMNode* sectionNode = nullptr;
    IXMLDOMNode* valueNode = nullptr;
    StringResult environmentName;
    StringResult environmentVersionText;
    StringResult targetPlatform;
    StringResult targetVersion;
    wchar_t number[12];

    if (!status->Succeeded() || FAILED(EnvironmentVersionInfo::CreateEmpty(&environmentVersion)) ||
        FAILED(CXMLUtil::AddElement(document, nullptr, L"PriInfo", &priInfoNode)))
    {
        CXMLUtil::CleanupNode(&valueNode);
        CXMLUtil::CleanupNode(&sectionNode);
        CXMLUtil::CleanupNode(&priInfoNode);
        return false;
    }

    const IResourceMapBase* primaryResourceMapInterface;
    if (FAILED(priFile->GetPrimaryResourceMap(&primaryResourceMapInterface)))
    {
        CXMLUtil::CleanupNode(&valueNode);
        CXMLUtil::CleanupNode(&sectionNode);
        CXMLUtil::CleanupNode(&priInfoNode);
        return false;
    }
    const auto* const primaryResourceMap = static_cast<const ResourceMapBase*>(primaryResourceMapInterface);

    bool failed = false;
    if ((outputOptions.GetFlags() & 0x1) == 0)
    {
        HRESULT result = CXMLUtil::AddElement(document, priInfoNode, L"PriHeader", &sectionNode);
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddElement(document, sectionNode, L"WindowsEnvironment", &valueNode);
        }
        if (SUCCEEDED(result))
        {
            result = priFile->GetFileDefaultEnvironment(&environmentName, environmentVersion);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, valueNode, L"name", environmentName.GetRef());
        }
        if (SUCCEEDED(result))
        {
            StringCchPrintfW(number, 6, L"%d", environmentVersion->GetMajorVersion());
            result = DefStringResult_Concat(environmentVersionText.GetStringResult(), number);
        }
        if (SUCCEEDED(result))
        {
            result = DefStringResult_Concat(environmentVersionText.GetStringResult(), L".");
        }
        if (SUCCEEDED(result))
        {
            StringCchPrintfW(number, 6, L"%d", environmentVersion->GetMinorVersion());
            result = DefStringResult_Concat(environmentVersionText.GetStringResult(), number);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, valueNode, L"version", environmentVersionText.GetRef());
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, valueNode, L"checksum", environmentVersion->GetVersionChecksum());
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            result = CXMLUtil::AddElement(document, sectionNode, L"AutoMerge", &valueNode);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::SetElementValue(
                valueNode,
                priFile->GetAutoMergeEnabled() ? L"true" : L"false",
                (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                           CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            result = CXMLUtil::AddElement(document, sectionNode, L"IsDeploymentMergeable", &valueNode);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::SetElementValue(
                valueNode,
                priFile->GetIsDeploymentMergeable() ? L"true" : L"false",
                (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                           CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            result = CXMLUtil::AddElement(document, sectionNode, L"TargetOS", &valueNode);
        }

        const BaseFile* baseFile = nullptr;
        if (SUCCEEDED(result))
        {
            result = priFile->GetBaseFile(&baseFile);
        }
        if (SUCCEEDED(result))
        {
            result = priFile->GetProfile()->GetTargetPlatformAndVersionForFileMagic(
                baseFile->GetFileHeader()->magic, &targetPlatform, &targetVersion);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, valueNode, L"version", targetVersion.GetRef());
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            result = CXMLUtil::AddElement(document, sectionNode, L"ReverseMap", &valueNode);
        }
        if (SUCCEEDED(result))
        {
            const ReverseFileMap* reverseFileMap = nullptr;
            result = CXMLUtil::SetElementValue(
                valueNode,
                priFile->TryGetReverseFileMap(&reverseFileMap) ? L"true" : L"false",
                (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                           CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            if (!status->Succeeded())
            {
                result = E_FAIL;
            }
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&valueNode);
            CXMLUtil::CleanupNode(&sectionNode);
        }
        else
        {
            failed = true;
        }
    }

    if (!failed && (outputOptions.GetFlags() & 0x8) == 0)
    {
        HRESULT result = CXMLUtil::AddElement(document, priInfoNode, L"QualifierInfo", &sectionNode);
        if (SUCCEEDED(result))
        {
            AtomPoolGroup* const atomPoolGroup = priFile->GetAtoms();
            const IDecisionInfo* const decisionInfo = priFile->GetDefaultDecisionInfo();
            if (!DumpDetailedQualifierInfo(document, sectionNode, decisionInfo, atomPoolGroup, outputOptions, status))
            {
                result = E_FAIL;
            }
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&sectionNode);
        }
        else
        {
            failed = true;
        }
    }

    if (!failed && (outputOptions.GetFlags() & 0x10) == 0)
    {
        AtomPoolGroup* const atomPoolGroup = priFile->GetAtoms();
        if (!DumpDetailedResourceMap(document, priInfoNode, primaryResourceMap, true, atomPoolGroup, outputOptions, status))
        {
            failed = true;
        }
        else
        {
            int resourceMapIndex = 0;
            if (priFile->GetNumResourceMaps() > 0)
            {
                do
                {
                    const IResourceMapBase* resourceMapInterface;
                    if (FAILED(priFile->GetResourceMap(resourceMapIndex, &resourceMapInterface)))
                    {
                        failed = true;
                        break;
                    }

                    const auto* const resourceMap = static_cast<const ResourceMapBase*>(resourceMapInterface);
                    if (resourceMap != primaryResourceMap)
                    {
                        if (!DumpDetailedResourceMap(document, priInfoNode, resourceMap, false, priFile->GetAtoms(), outputOptions, status))
                        {
                            failed = true;
                            break;
                        }
                    }
                    ++resourceMapIndex;
                } while (resourceMapIndex < priFile->GetNumResourceMaps());
            }
        }
    }

    if (!failed)
    {
        CXMLUtil::CleanupNode(&priInfoNode);
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&valueNode);
    CXMLUtil::CleanupNode(&sectionNode);
    CXMLUtil::CleanupNode(&priInfoNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpDetailedResourceMap(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const ResourceMapBase* const resourceMap,
    const bool primary,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1193, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1194, L"xmlNode", 0);
        return false;
    }
    if (resourceMap == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1195, L"resourceMap", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1196, L"atomPoolGroup", 0);
        return false;
    }

    bool succeeded = false;
    IXMLDOMNode* resourceMapNode = nullptr;
    IXMLDOMNode* versionInfoNode = nullptr;
    StringResult version;
    wchar_t number[12];

    HRESULT result = CXMLUtil::AddElement(document, parent, L"ResourceMap", &resourceMapNode);
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"name", resourceMap->GetSchema()->GetSimpleId());
    }
    if (SUCCEEDED(result) && primary)
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"primary", L"true");
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"uniqueName", resourceMap->GetSchema()->GetUniqueId());
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", resourceMap->GetSchema()->GetMajorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = DefStringResult_Concat(version.GetStringResult(), L".");
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", resourceMap->GetSchema()->GetMinorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceMapNode, L"version", version.GetRef());
    }

    if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x20) == 0)
    {
        const IHierarchicalSchemaVersionInfo* const currentVersion = resourceMap->GetSchema()->GetVersionInfo(0);
        result = CXMLUtil::AddElement(document, resourceMapNode, L"VersionInfo", &versionInfoNode);
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, versionInfoNode, L"version", version.GetRef());
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, versionInfoNode, L"checksum", currentVersion->GetVersionChecksum());
        }
        if (SUCCEEDED(result))
        {
            result =
                CXMLUtil::AddAttribute(document, versionInfoNode, L"numScopes", static_cast<std::uint32_t>(currentVersion->GetNumScopes()));
        }
        if (SUCCEEDED(result))
        {
            result =
                CXMLUtil::AddAttribute(document, versionInfoNode, L"numItems", static_cast<std::uint32_t>(currentVersion->GetNumItems()));
        }

        int versionIndex = 1;
        while (SUCCEEDED(result) && versionIndex < resourceMap->GetSchema()->GetNumVersionInfos())
        {
            if (!DumpDetailedHierarchicalSchemaVersionInfo(
                    document,
                    versionInfoNode,
                    resourceMap->GetSchema()->GetVersionInfo(versionIndex),
                    versionIndex == 0,
                    outputOptions,
                    status))
            {
                result = E_FAIL;
            }
            ++versionIndex;
        }

        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&versionInfoNode);
        }
    }

    if (SUCCEEDED(result) &&
        DumpDetailedRecursiveScopeTree(document, resourceMapNode, resourceMap->GetRootSubtree(), atomPoolGroup, outputOptions, status))
    {
        CXMLUtil::CleanupNode(&resourceMapNode);
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&versionInfoNode);
    CXMLUtil::CleanupNode(&resourceMapNode);
    return succeeded;
}

bool StandalonePriFileXml::DumpDetailedRecursiveScopeTree(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const ResourceMapSubtree* const resourceMapSubtree,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1313, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1314, L"xmlNode", 0);
        return false;
    }
    if (resourceMapSubtree == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1315, L"resMapSubtree", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1316, L"atomPoolGroup", 0);
        return false;
    }

    IXMLDOMNode* subtreeNode = nullptr;
    if ((outputOptions.GetFlags() & 0x40) != 0 && GetSubtreeIsEmpty(resourceMapSubtree, outputOptions, status))
    {
        return true;
    }

    int childIndex = 0;
    while (childIndex < resourceMapSubtree->GetNumChildren())
    {
        bool failed = false;
        if (resourceMapSubtree->ChildIsScope(childIndex))
        {
            StringResult childName;
            const ResourceMapSubtree* childSubtree = nullptr;

            if (FAILED(resourceMapSubtree->GetChildName(childIndex, &childName)) ||
                FAILED(resourceMapSubtree->GetChildScopeSubtree(childIndex, &childSubtree)) || childSubtree == nullptr)
            {
                failed = true;
            }
            else if ((outputOptions.GetFlags() & 0x40) == 0 || !GetSubtreeIsEmpty(childSubtree, outputOptions, status))
            {
                HRESULT result = CXMLUtil::AddElement(document, parent, L"ResourceMapSubtree", &subtreeNode);
                if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
                {
                    result = CXMLUtil::AddAttribute(
                        document, subtreeNode, L"index", static_cast<std::uint32_t>(childSubtree->GetSubtreeRootIndex()));
                }
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddAttribute(document, subtreeNode, L"name", childName.GetRef());
                }
                if (FAILED(result) ||
                    !DumpDetailedRecursiveScopeTree(document, subtreeNode, childSubtree, atomPoolGroup, outputOptions, status))
                {
                    failed = true;
                }
                else
                {
                    delete childSubtree;
                    CXMLUtil::CleanupNode(&subtreeNode);
                }
            }
        }

        if (failed)
        {
            CXMLUtil::CleanupNode(&subtreeNode);
            return false;
        }
        ++childIndex;
    }

    childIndex = 0;
    while (childIndex < resourceMapSubtree->GetNumChildren())
    {
        bool failed = false;
        if (!resourceMapSubtree->ChildIsScope(childIndex))
        {
            StringResult childName;
            if (FAILED(resourceMapSubtree->GetChildName(childIndex, &childName)))
            {
                failed = true;
            }
            else
            {
                NamedResourceResult resource;
                if (FAILED(resourceMapSubtree->GetChildResource(childIndex, &resource)) ||
                    !DumpDetailedItem(document, parent, &resource, &childName, atomPoolGroup, outputOptions, status))
                {
                    failed = true;
                }
            }
        }

        if (failed)
        {
            CXMLUtil::CleanupNode(&subtreeNode);
            return false;
        }
        ++childIndex;
    }

    CXMLUtil::CleanupNode(&subtreeNode);
    CXMLUtil::CleanupNode(&subtreeNode);
    return true;
}

bool StandalonePriFileXml::DumpDetailedItem(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    NamedResourceResult* const resource,
    StringResult* const nameResult,
    AtomPoolGroup* const atomPoolGroup,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1506, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1507, L"xmlNode", 0);
        return false;
    }
    if (resource == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1508, L"resource", 0);
        return false;
    }
    if (nameResult == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1509, L"nameResult", 0);
        return false;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1510, L"atomPoolGroup", 0);
        return false;
    }

    DecisionResult decision;
    IXMLDOMNode* resourceNode = nullptr;
    IXMLDOMNode* candidateNode = nullptr;
    IXMLDOMNode* qualifierSetNode = nullptr;
    IXMLDOMNode* valueNode = nullptr;
    StringResult resourceUri;
    StringResult resourceName;
    bool succeeded = false;

    if ((outputOptions.GetFlags() & 0x80) != 0 && GetNamedResourceIsEmpty(resource, outputOptions, status))
    {
        return true;
    }

    HRESULT result = CXMLUtil::AddElement(document, parent, L"NamedResource", &resourceNode);
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, resourceNode, L"name", nameResult->GetRef());
    }
    if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
    {
        result = CXMLUtil::AddAttribute(document, resourceNode, L"index", static_cast<std::uint32_t>(resource->GetResourceIndexInSchema()));
    }
    if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x4) == 0)
    {
        result = resource->GetResourceName(&resourceName);
        if (SUCCEEDED(result) && !ConstructUri(resource->GetParentSchema()->GetSimpleId(), resourceName.GetRef(), status, &resourceUri))
        {
            result = E_FAIL;
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, resourceNode, L"uri", resourceUri.GetRef());
        }
    }

    const IHierarchicalSchema* linkedSchema;
    std::uint32_t linkedResourceIndex;
    bool hasResourceLink = false;
    if (SUCCEEDED(result))
    {
        hasResourceLink = resource->TryGetResourceLink(&linkedSchema, &linkedResourceIndex);
        if (hasResourceLink && (outputOptions.GetFlags() & 0x100) == 0)
        {
            StringResult linkedResourceName;
            if (!linkedSchema->GetItemNames()->TryGetString(static_cast<Atom::Index>(linkedResourceIndex), &linkedResourceName))
            {
                result = E_FAIL;
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddElement(document, resourceNode, L"Link", &valueNode);
            }
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::SetElementValue(
                    valueNode,
                    linkedResourceName.GetRef(),
                    (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                               CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
            }
            if (SUCCEEDED(result))
            {
                CXMLUtil::CleanupNode(&valueNode);
            }
        }
    }

    if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x200) == 0)
    {
        result = resource->GetDecision(&decision);
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddElement(document, resourceNode, L"Decision", &candidateNode);
        }
        if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
        {
            int decisionIndex;
            result = decision.GetIndex(&decisionIndex);
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, candidateNode, L"index", static_cast<std::uint32_t>(decisionIndex));
            }
        }

        int qualifierSetIndex = 0;
        while (SUCCEEDED(result) && qualifierSetIndex < decision.GetNumQualifierSets())
        {
            QualifierSetResult qualifierSet;
            result = decision.GetQualifierSet(qualifierSetIndex, &qualifierSet, nullptr);
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddElement(document, candidateNode, L"QualifierSet", &qualifierSetNode);
            }
            if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
            {
                int qualifierSetPoolIndex;
                result = qualifierSet.GetIndex(&qualifierSetPoolIndex);
                if (SUCCEEDED(result))
                {
                    result =
                        CXMLUtil::AddAttribute(document, qualifierSetNode, L"index", static_cast<std::uint32_t>(qualifierSetPoolIndex));
                }
            }

            int qualifierIndex = 0;
            int numQualifiers = 0;
            if (SUCCEEDED(result))
            {
                numQualifiers = qualifierSet.GetNumQualifiers();
            }
            while (SUCCEEDED(result) && qualifierIndex < numQualifiers)
            {
                QualifierResult qualifier;
                result = qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr);
                if (SUCCEEDED(result) &&
                    !DumpDetailedQualifier(document, qualifierSetNode, &qualifier, atomPoolGroup, false, false, outputOptions, status))
                {
                    result = E_FAIL;
                }
                ++qualifierIndex;
            }
            if (SUCCEEDED(result))
            {
                CXMLUtil::CleanupNode(&qualifierSetNode);
                ++qualifierSetIndex;
            }
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&candidateNode);
        }
    }

    if (SUCCEEDED(result) &&
        ((hasResourceLink && (outputOptions.GetFlags() & 0x800) == 0) || (!hasResourceLink && (outputOptions.GetFlags() & 0x400) != 0)))
    {
        CXMLUtil::CleanupNode(&resourceNode);
        succeeded = true;
    }

    int candidateIndex = 0;
    while (SUCCEEDED(result) && !succeeded && candidateIndex < resource->GetNumCandidates())
    {
        ResourceCandidateResult candidate;
        QualifierSetResult qualifierSet;
        StringResult valueTypeText;

        result = resource->GetCandidate(candidateIndex, &candidate);
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddElement(document, resourceNode, L"Candidate", &candidateNode);
        }

        MrmEnvironment::ResourceValueType valueType;
        if (SUCCEEDED(result))
        {
            result = candidate.GetResourceValueType(&valueType);
        }
        if (SUCCEEDED(result))
        {
            result = MrmEnvironment::GetExternalStringForResourceValueType(valueType, &valueTypeText);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddAttribute(document, candidateNode, L"type", valueTypeText.GetRef());
        }
        if (SUCCEEDED(result))
        {
            result = candidate.GetQualifiers(&qualifierSet);
        }
        if (SUCCEEDED(result))
        {
            result = CXMLUtil::AddElement(document, candidateNode, L"QualifierSet", &qualifierSetNode);
        }
        if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2) == 0)
        {
            int qualifierSetIndex;
            result = qualifierSet.GetIndex(&qualifierSetIndex);
            if (SUCCEEDED(result))
            {
                result = CXMLUtil::AddAttribute(document, qualifierSetNode, L"index", static_cast<std::uint32_t>(qualifierSetIndex));
            }
        }

        int qualifierIndex = 0;
        int numQualifiers = 0;
        if (SUCCEEDED(result))
        {
            numQualifiers = qualifierSet.GetNumQualifiers();
        }
        while (SUCCEEDED(result) && qualifierIndex < numQualifiers)
        {
            QualifierResult qualifier;
            result = qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr);
            if (SUCCEEDED(result) &&
                !DumpDetailedQualifier(document, qualifierSetNode, &qualifier, atomPoolGroup, false, false, outputOptions, status))
            {
                result = E_FAIL;
            }
            ++qualifierIndex;
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&qualifierSetNode);
        }

        if (SUCCEEDED(result))
        {
            StringResult valueText;
            if (wcscmp(valueTypeText.GetRef(), L"EmbeddedData") == 0)
            {
                BlobResult blob;
                if (!candidate.TryGetBlobValue(&blob))
                {
                    result = E_FAIL;
                }

                std::size_t blobSize;
                const void* blobData = nullptr;
                if (SUCCEEDED(result))
                {
                    blobData = blob.GetRef(&blobSize);
                    if (blobSize == 0 || !status->Succeeded())
                    {
                        result = E_FAIL;
                    }
                }

                std::uint32_t encodedSize;
                if (SUCCEEDED(result))
                {
                    result = base64encodeU(blobData, static_cast<std::uint32_t>(blobSize), nullptr, 0, &encodedSize);
                }

                wchar_t* encodedBuffer = nullptr;
                if (SUCCEEDED(result))
                {
                    result = valueText.SetEmptyContents(encodedSize, &encodedBuffer, nullptr);
                    if (SUCCEEDED(result) && encodedBuffer == nullptr)
                    {
                        result = E_FAIL;
                    }
                }
                if (SUCCEEDED(result) && !status->Succeeded())
                {
                    result = E_FAIL;
                }

                std::uint32_t actualEncodedSize;
                if (SUCCEEDED(result))
                {
                    result = base64encodeU(blobData, static_cast<std::uint32_t>(blobSize), encodedBuffer, encodedSize, &actualEncodedSize);
                    if (SUCCEEDED(result) && actualEncodedSize != encodedSize)
                    {
                        result = E_FAIL;
                    }
                }
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddElement(document, candidateNode, L"Base64Value", &qualifierSetNode);
                }
                if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2000) != 0)
                {
                    StringResult rawLocator;
                    if (GetRawLocatorText(&candidate, status, &rawLocator))
                    {
                        result = CXMLUtil::AddAttribute(document, qualifierSetNode, L"rawLocator", rawLocator.GetRef());
                    }
                }
            }
            else
            {
                if (!candidate.TryGetStringValue(&valueText))
                {
                    result = E_FAIL;
                }
                if (SUCCEEDED(result))
                {
                    result = CXMLUtil::AddElement(document, candidateNode, L"Value", &qualifierSetNode);
                }
                if (SUCCEEDED(result) && (outputOptions.GetFlags() & 0x2000) != 0)
                {
                    StringResult rawLocator;
                    if (GetRawLocatorText(&candidate, status, &rawLocator))
                    {
                        result = CXMLUtil::AddAttribute(document, qualifierSetNode, L"rawLocator", rawLocator.GetRef());
                    }
                }
            }

            if (SUCCEEDED(result))
            {
                result = CXMLUtil::SetElementValue(
                    qualifierSetNode,
                    valueText.GetRef(),
                    (outputOptions.GetFlags() & 0x1000) != 0 ? CXMLUtil::XmlUtilFlags::XmlUtil_SanitizeStrings :
                                                               CXMLUtil::XmlUtilFlags::XmlUtil_PreserveStrings);
            }
        }
        if (SUCCEEDED(result))
        {
            CXMLUtil::CleanupNode(&qualifierSetNode);
            CXMLUtil::CleanupNode(&candidateNode);
            ++candidateIndex;
        }
    }

    if (SUCCEEDED(result) && !succeeded)
    {
        CXMLUtil::CleanupNode(&resourceNode);
        succeeded = true;
    }

    CXMLUtil::CleanupNode(&valueNode);
    CXMLUtil::CleanupNode(&qualifierSetNode);
    CXMLUtil::CleanupNode(&candidateNode);
    CXMLUtil::CleanupNode(&resourceNode);
    return succeeded;
}

bool StandalonePriFileXml::GetNamedResourceIsEmpty(
    NamedResourceResult* const resource,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    static_cast<void>(status);

    const int candidateCount = resource->GetNumCandidates();
    const IHierarchicalSchema* linkedSchema;
    std::uint32_t linkedResourceIndex;
    const bool hasLink = resource->TryGetResourceLink(&linkedSchema, &linkedResourceIndex);
    const std::uint64_t flags = outputOptions.GetFlags();

    bool includeCandidates;
    if (hasLink)
    {
        includeCandidates = (flags & 0x800) != 0;
        if ((flags & 0x100) == 0)
        {
            return false;
        }
    }
    else
    {
        includeCandidates = (flags & 0x400) == 0;
    }

    return (candidateCount <= 0) || !includeCandidates;
}

bool StandalonePriFileXml::GetSubtreeIsEmpty(
    const ResourceMapSubtree* const subtree,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    int childIndex = 0;
    if (subtree->GetNumChildren() <= 0)
    {
        return true;
    }

    do
    {
        if (subtree->ChildIsScope(childIndex))
        {
            const ResourceMapSubtree* childSubtree = nullptr;
            subtree->GetChildScopeSubtree(childIndex, &childSubtree);
            if ((childSubtree != nullptr) && !GetSubtreeIsEmpty(childSubtree, outputOptions, status))
            {
                return false;
            }
        }
        else
        {
            NamedResourceResult resource;
            if (SUCCEEDED(subtree->GetChildResource(childIndex, &resource)) && !GetNamedResourceIsEmpty(&resource, outputOptions, status))
            {
                return false;
            }
        }

        ++childIndex;
    } while (childIndex < subtree->GetNumChildren());

    return true;
}

bool StandalonePriFileXml::GetRawLocatorText(ResourceCandidateResult* const candidate, IDefStatus* const status, StringResult* const result)
{
    MRMFILE_MAP_VALUE_LOCATOR locatorType;
    std::uint32_t data;
    std::uint16_t extraData;
    std::uint16_t detail;
    const HRESULT locationResult = candidate->GetValueLocation(&locatorType, &data, &extraData, &detail);
    if (!Def_HrFailed0(locationResult, status))
    {
        wchar_t locatorText[MAX_PATH];
        HRESULT formatResult;
        if (locatorType == MRMFILE_MAP_VALUE_LOCATOR_INTERNAL)
        {
            formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"internal:%d@%d", detail, data);
        }
        else if (locatorType == MRMFILE_MAP_VALUE_LOCATOR_DATA_ITEM)
        {
            const std::uint32_t itemIndex = static_cast<std::uint16_t>(data) | (static_cast<std::uint32_t>(extraData) << 16);
            if (detail != 0)
            {
                formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"%d_%d[%d]", detail, HIWORD(data), itemIndex);
            }
            else
            {
                formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"%d[%d]", HIWORD(data), itemIndex);
            }
        }
        else if (locatorType == MRMFILE_MAP_VALUE_LOCATOR_FILE_ITEM)
        {
            const std::uint32_t itemIndex = static_cast<std::uint16_t>(data) | (static_cast<std::uint32_t>(extraData) << 16);
            if (detail != 0)
            {
                formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"file:%d_%d[%d]", detail, HIWORD(data), itemIndex);
            }
            else
            {
                formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"file:%d[%d]", HIWORD(data), itemIndex);
            }
        }
        else
        {
            formatResult = StringCchPrintfW(locatorText, MAX_PATH, L"unknown:%d_0x%x_0x%x_%d", locatorType, detail, data, extraData);
        }

        if (FAILED(formatResult))
        {
            if (status != nullptr)
            {
                status->SetError(formatResult, L"" __FILE__, 1488, L"", 0);
            }
        }
        else
        {
            Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), locatorText), status);
        }
    }

    return status->Succeeded();
}

bool StandalonePriFileXml::ConstructUri(
    const wchar_t* const authority,
    const wchar_t* const path,
    IDefStatus* const status,
    StringResult* const result)
{
    std::wstring normalizedPath(path);

    Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), L"ms-resource://"), status);
    Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), authority), status);
    Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), L"/"), status);

    std::replace(normalizedPath.begin(), normalizedPath.end(), L'\\', L'/');
    Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), normalizedPath.c_str()), status);

    std::uint32_t encodedLength = static_cast<std::uint32_t>(3 * result->GetLength());
    auto* const encoded = new (std::nothrow) wchar_t[encodedLength];
    if (encoded == nullptr)
    {
        return false;
    }

    const HRESULT conversionResult = Runtime::CResourceIndexInternal::s_ConvertToPercentEncoding(result->GetRef(), encoded, &encodedLength);
    Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), encoded), status);
    delete[] encoded;

    if (conversionResult != S_OK)
    {
        return false;
    }
    return status->Succeeded();
}

bool StandalonePriFileXml::DumpDetailedHierarchicalSchemaVersionInfo(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const IHierarchicalSchemaVersionInfo* const versionInfo,
    const bool currentVersion,
    const OutputOptions& outputOptions,
    IDefStatus* const status)
{
    if (status == nullptr)
    {
        return false;
    }
    if (document == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1258, L"xmlDoc", 0);
        return false;
    }
    if (parent == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1259, L"xmlNode", 0);
        return false;
    }
    if (versionInfo == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 1260, L"versionInfo", 0);
        return false;
    }
    if ((outputOptions.GetFlags() & 0x20) != 0)
    {
        return true;
    }

    IXMLDOMNode* versionNode = nullptr;
    StringResult version;
    wchar_t number[6];

    HRESULT result = CXMLUtil::AddElement(document, parent, currentVersion ? L"Version" : L"PriorVersion", &versionNode);
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", versionInfo->GetMajorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = DefStringResult_Concat(version.GetStringResult(), L".");
    }
    if (SUCCEEDED(result))
    {
        StringCchPrintfW(number, 6, L"%d", versionInfo->GetMinorVersion());
        result = DefStringResult_Concat(version.GetStringResult(), number);
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionNode, L"version", version.GetRef());
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionNode, L"checksum", versionInfo->GetVersionChecksum());
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionNode, L"numScopes", static_cast<std::uint32_t>(versionInfo->GetNumScopes()));
    }
    if (SUCCEEDED(result))
    {
        result = CXMLUtil::AddAttribute(document, versionNode, L"numItems", static_cast<std::uint32_t>(versionInfo->GetNumItems()));
    }

    if (FAILED(result))
    {
        CXMLUtil::CleanupNode(&versionNode);
        return false;
    }

    CXMLUtil::CleanupNode(&versionNode);
    CXMLUtil::CleanupNode(&versionNode);
    return true;
}

} // namespace Microsoft::Resources

namespace Microsoft::Resources
{
HRESULT PriConfigurationXml::GetDefaultQualifierMap(
    const MrmProfile* const profile,
    const UnifiedEnvironment* const environment,
    IDefStatusEx* const status,
    Runtime::MrtMap<const wchar_t*, const wchar_t*>* const qualifiers)
{
    static_cast<void>(status);
    if (qualifiers == nullptr)
    {
        return E_INVALIDARG;
    }

    Runtime::MrtMap<const wchar_t*, const wchar_t*> defaults;
    const int tokenCount = profile->GetNumSupportedTokens();
    for (int index = 0; index < tokenCount; ++index)
    {
        StringResult token;
        if (SUCCEEDED(profile->GetToken(index, &token)))
        {
            QualifierBuildInfo info {};
            if (SUCCEEDED(profile->GetQualifierBuildInfoByToken(token.GetRef(), environment, &info)))
            {
                StringResult name;
                if (SUCCEEDED(environment->GetName(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, info.qualifier.name, &name)))
                {
                    defaults.Insert(name.GetRef(), info.pDefaultValue);
                }
            }
        }
    }

    struct Iterator
    {
        const Runtime::MrtMap<const wchar_t*, const wchar_t*>* map;
        std::uint32_t index;
    };

    auto* iterator = new (std::nothrow) Iterator {&defaults, static_cast<std::uint32_t>(-1)};
    if (iterator == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT iterationResult = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    ++iterator->index;
    while (iterator->index < iterator->map->GetCount())
    {
        const auto& entry = iterator->map->GetEntry(iterator->index);
        std::uint32_t existingIndex;
        if (qualifiers->FindKeyIndex(entry.key, &existingIndex) == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            qualifiers->Insert(entry.key, entry.value);
        }
        ++iterator->index;
    }
    delete iterator;

    if (iterationResult != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
    {
        return iterationResult;
    }
    return S_OK;
}

bool PriConfigurationXml::GeneratePriConfigToFile(
    const MrmPlatformVersionInternal platformVersion,
    const wchar_t* const outputFile,
    const wchar_t* const defaultQualifiers,
    IDefStatusEx* const status)
{
    if (status == nullptr || outputFile == nullptr || defaultQualifiers == nullptr)
    {
        return false;
    }

    bool result = false;
    IXMLDOMDocument2* document = nullptr;
    if (GeneratePriConfig(platformVersion, defaultQualifiers, status, &document))
    {
        const HRESULT writeResult = CXMLUtil::WriteXmlToFile(document, outputFile);
        if (writeResult == E_ACCESSDENIED)
        {
            status->SetError(E_DEF_ACCESS_DENIED, outputFile, 0, nullptr);
        }
        else if (SUCCEEDED(writeResult))
        {
            result = true;
        }
        else
        {
            status->SetError(writeResult, L"" __FILE__, 1799, L"", 0);
        }
    }

    if (document != nullptr)
    {
        document->Release();
    }
    return result;
}

bool PriConfigurationXml::GeneratePriConfig(
    MrmPlatformVersionInternal platformVersion,
    const wchar_t* const defaultQualifiers,
    IDefStatusEx* const status,
    IXMLDOMDocument2** const document)
{
    bool succeeded = true;
    HRESULT result = S_OK;
    IXMLDOMDocument2* generatedDocument = nullptr;
    IXMLDOMNode* resources = nullptr;
    IXMLDOMNode* packaging = nullptr;
    IXMLDOMNode* autoResourcePackage = nullptr;
    IXMLDOMNode* index = nullptr;
    IXMLDOMNode* defaults = nullptr;
    IXMLDOMNode* qualifier = nullptr;
    IXMLDOMNode* indexerConfig = nullptr;
    MrmProfile* profile = nullptr;
    AtomPoolGroup* atomPoolGroup = nullptr;
    UnifiedEnvironment* environment = nullptr;
    Indexers::CQualifierApplicator* applicator = nullptr;

    const auto profileType = static_cast<MrmProfile::ProfileType>(platformVersion);
    if (platformVersion == MrmPlatformVersionInternal::DefaultPlatformVersion)
    {
        platformVersion = MrmPlatformVersionInternal::WindowsCore;
    }

    if (FAILED(MrmProfile::ChooseDefaultProfile(profileType, platformVersion, nullptr, nullptr, nullptr, &profile)) ||
        FAILED(AtomPoolGroup::CreateInstance(0, &atomPoolGroup)) ||
        FAILED(UnifiedEnvironment::CreateInstance(profile, atomPoolGroup, &environment)))
    {
        *document = generatedDocument;
        goto Cleanup;
    }

    if (environment == nullptr)
    {
        if (!status->Failed())
        {
            result = E_OUTOFMEMORY;
            goto Error;
        }
        result = status->GetHResult();
        if (FAILED(result))
        {
            goto Error;
        }
    }

    applicator = new (std::nothrow) Indexers::CQualifierApplicator(profile, environment);
    if (applicator == nullptr)
    {
        result = E_OUTOFMEMORY;
        goto Error;
    }

    {
        Runtime::MrtMap<const wchar_t*, const wchar_t*> qualifiers;
        result = applicator->GetQualifierMapFromToken(defaultQualifiers, &qualifiers, status);
        if (FAILED(result))
        {
            goto Error;
        }

        result = GetDefaultQualifierMap(profile, environment, status, &qualifiers);
        if (FAILED(result))
        {
            goto Error;
        }

        result = CXMLUtil::CreateXMLDocument(&generatedDocument);
        if (FAILED(result))
        {
            goto Error;
        }

        if (generatedDocument == nullptr || FAILED(CXMLUtil::AddElement(generatedDocument, nullptr, L"resources", &resources)))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        StringResult targetPlatform;
        StringResult targetVersion;
        if (SUCCEEDED(profile->GetTargetPlatformAndVersion(&targetPlatform, &targetVersion)) &&
            FAILED(CXMLUtil::AddAttribute(generatedDocument, resources, L"targetOsVersion", targetVersion.GetRef())))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        if (FAILED(CXMLUtil::AddAttribute(generatedDocument, resources, L"majorVersion", L"1")) ||
            FAILED(CXMLUtil::AddElement(generatedDocument, resources, L"packaging", &packaging)) ||
            FAILED(CXMLUtil::AddElement(generatedDocument, packaging, L"autoResourcePackage", &autoResourcePackage)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, autoResourcePackage, L"qualifier", L"Language")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&autoResourcePackage);
        if (FAILED(CXMLUtil::AddElement(generatedDocument, packaging, L"autoResourcePackage", &autoResourcePackage)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, autoResourcePackage, L"qualifier", L"Scale")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&autoResourcePackage);
        if (FAILED(CXMLUtil::AddElement(generatedDocument, packaging, L"autoResourcePackage", &autoResourcePackage)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, autoResourcePackage, L"qualifier", L"DXFeatureLevel")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }
        CXMLUtil::CleanupNode(&autoResourcePackage);
        CXMLUtil::CleanupNode(&packaging);

        if (FAILED(CXMLUtil::AddElement(generatedDocument, resources, L"index", &index)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, index, L"root", L"\\")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, index, L"startIndexAt", L"\\")) ||
            FAILED(CXMLUtil::AddElement(generatedDocument, index, L"default", &defaults)))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        struct Iterator
        {
            const Runtime::MrtMap<const wchar_t*, const wchar_t*>* map;
            std::uint32_t index;
        };
        auto* iterator = new (std::nothrow) Iterator {&qualifiers, static_cast<std::uint32_t>(-1)};
        if (iterator == nullptr)
        {
            result = E_OUTOFMEMORY;
            goto Error;
        }

        ++iterator->index;
        while (iterator->index < iterator->map->GetCount())
        {
            const auto& entry = iterator->map->GetEntry(iterator->index);
            if (FAILED(CXMLUtil::AddElement(generatedDocument, defaults, L"qualifier", &qualifier)) ||
                FAILED(CXMLUtil::AddAttribute(generatedDocument, qualifier, L"name", entry.key)) ||
                FAILED(CXMLUtil::AddAttribute(generatedDocument, qualifier, L"value", entry.value)))
            {
                break;
            }
            CXMLUtil::CleanupNode(&qualifier);
            ++iterator->index;
        }
        delete iterator;
        CXMLUtil::CleanupNode(&defaults);

        if (FAILED(CXMLUtil::AddElement(generatedDocument, index, L"indexer-config", &indexerConfig)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"type", L"folder")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"foldernameAsQualifier", L"true")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"filenameAsQualifier", L"true")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"qualifierDelimiter", L".")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&indexerConfig);
        if (FAILED(CXMLUtil::AddElement(generatedDocument, index, L"indexer-config", &indexerConfig)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"type", L"resw")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"convertDotsToSlashes", L"true")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"initialPath", L"")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&indexerConfig);
        if (FAILED(CXMLUtil::AddElement(generatedDocument, index, L"indexer-config", &indexerConfig)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"type", L"resjson")) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"initialPath", L"")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&indexerConfig);
        if (FAILED(CXMLUtil::AddElement(generatedDocument, index, L"indexer-config", &indexerConfig)) ||
            FAILED(CXMLUtil::AddAttribute(generatedDocument, indexerConfig, L"type", L"PRI")))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }

        CXMLUtil::CleanupNode(&indexerConfig);
        CXMLUtil::CleanupNode(&index);
        if (FAILED(
                CXMLUtil::AddComment(
                    generatedDocument, resources, L"<index startIndexAt=\"Start Index Here\" root=\"Root Here\">", &index)) ||
            (CXMLUtil::CleanupNode(&index),
             FAILED(
                 CXMLUtil::AddComment(
                     generatedDocument, resources, L"        <indexer-config type=\"resfiles\" qualifierDelimiter=\".\"/>", &index))) ||
            (CXMLUtil::CleanupNode(&index),
             FAILED(
                 CXMLUtil::AddComment(
                     generatedDocument,
                     resources,
                     L"        <indexer-config type=\"priinfo\" emitStrings=\"true\" emitPaths=\"true\" emitEmbeddedData=\"true\"/>",
                     &index))) ||
            (CXMLUtil::CleanupNode(&index), FAILED(CXMLUtil::AddComment(generatedDocument, resources, L"</index>", &index))))
        {
            *document = generatedDocument;
            generatedDocument = nullptr;
            goto Cleanup;
        }
        CXMLUtil::CleanupNode(&index);
        CXMLUtil::CleanupNode(&resources);
    }

    *document = generatedDocument;
    generatedDocument = nullptr;
    goto Cleanup;

Error:
    succeeded = false;
    if (status->Succeeded())
    {
        status->SetError(result, L"" __FILE__, 1994, L"", 0);
    }

Cleanup:
    CXMLUtil::CleanupNode(&qualifier);
    CXMLUtil::CleanupNode(&autoResourcePackage);
    CXMLUtil::CleanupNode(&packaging);
    CXMLUtil::CleanupNode(&indexerConfig);
    CXMLUtil::CleanupNode(&index);
    CXMLUtil::CleanupNode(&defaults);
    CXMLUtil::CleanupNode(&resources);
    delete applicator;
    delete environment;
    delete atomPoolGroup;
    delete profile;
    if (generatedDocument != nullptr)
    {
        generatedDocument->Release();
    }
    return succeeded;
}
} // namespace Microsoft::Resources
