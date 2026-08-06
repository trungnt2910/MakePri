#include "StdAfx.h"

#include <IndexerBase.h>

namespace Microsoft::Resources::Indexers
{
namespace
{
// clang-format off
constexpr wchar_t ConfigurationSchema[] =
    LR"xml(<?xml version="1.0" encoding="utf-8"?>)xml"
    LR"xml(<xs:schema attributeFormDefault="unqualified" elementFormDefault="qualified" xmlns:xs="http://www.w3.org/2001/XMLSchema">)xml"
        LR"xml(<xs:element name="resources">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:sequence>)xml"
                    LR"xml(<xs:element name="packaging" maxOccurs="1" minOccurs="0">)xml"
                        LR"xml(<xs:complexType>)xml"
                            LR"xml(<xs:sequence>)xml"
                                LR"xml(<xs:element name="autoResourcePackage" maxOccurs="unbounded" minOccurs="0">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:sequence>)xml"
                                            LR"xml(<xs:element name="autoPackages" minOccurs="0" maxOccurs="unbounded">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:sequence minOccurs="1" maxOccurs="unbounded">)xml"
                                                        LR"xml(<xs:any processContents="skip" />)xml"
                                                    LR"xml(</xs:sequence>)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                        LR"xml(</xs:sequence>)xml"
                                        LR"xml(<xs:attribute name="qualifier" type="xs:string" use="required" />)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                                LR"xml(<xs:element name="resourcePackage" maxOccurs="unbounded" minOccurs="0">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:sequence>)xml"
                                            LR"xml(<xs:element name="qualifierSet" maxOccurs="unbounded" minOccurs="0">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:attribute name="definition" type="xs:string" use="required" />)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                        LR"xml(</xs:sequence>)xml"
                                        LR"xml(<xs:attribute name="name" type="xs:string" use="required" />)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                                LR"xml(<xs:element name="omitSchemaFromResourcePacks" maxOccurs="1" minOccurs="0" />)xml"
                                LR"xml(<xs:element name="splitLanguageVariants" maxOccurs="1" minOccurs="0" />)xml"
                                LR"xml(<xs:element name="granularSplit" maxOccurs="1" minOccurs="0" />)xml"
                                LR"xml(<xs:element name="packagingOptions" maxOccurs="1" minOccurs="0">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:attribute name="packageBy" type="xs:string" use="required" />)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                            LR"xml(</xs:sequence>)xml"
                        LR"xml(</xs:complexType>)xml"
                    LR"xml(</xs:element>)xml"
                    LR"xml(<xs:element maxOccurs="unbounded" name="index">)xml"
                        LR"xml(<xs:complexType>)xml"
                            LR"xml(<xs:sequence>)xml"
                                LR"xml(<xs:element name="qualifiers" minOccurs="0" maxOccurs="unbounded">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:sequence>)xml"
                                            LR"xml(<xs:element minOccurs="1" maxOccurs="unbounded" name="qualifier">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:attribute name="name" type="xs:string" use="required" />)xml"
                                                    LR"xml(<xs:attribute name="value" type="xs:string" use="required" />)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                        LR"xml(</xs:sequence>)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                                LR"xml(<xs:element name="default" minOccurs="0" maxOccurs="unbounded">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:sequence>)xml"
                                            LR"xml(<xs:element minOccurs="1" maxOccurs="unbounded" name="qualifier">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:sequence>)xml"
                                                        LR"xml(<xs:element name="allowed" minOccurs="0" maxOccurs="1">)xml"
                                                            LR"xml(<xs:complexType>)xml"
                                                                LR"xml(<xs:sequence minOccurs="1" maxOccurs="unbounded">)xml"
                                                                    LR"xml(<xs:any processContents="skip" />)xml"
                                                                LR"xml(</xs:sequence>)xml"
                                                            LR"xml(</xs:complexType>)xml"
                                                        LR"xml(</xs:element>)xml"
                                                    LR"xml(</xs:sequence>)xml"
                                                    LR"xml(<xs:attribute name="name" type="xs:string" use="required" />)xml"
                                                    LR"xml(<xs:attribute name="value" type="xs:string" use="required" />)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                        LR"xml(</xs:sequence>)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                                LR"xml(<xs:element name="indexer-config" minOccurs="0" maxOccurs="unbounded">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:sequence>)xml"
                                            LR"xml(<xs:any minOccurs="0" maxOccurs="unbounded" processContents="skip"/>)xml"
                                        LR"xml(</xs:sequence>)xml"
                                        LR"xml(<xs:attribute name="type" type="xs:string" use="required" />)xml"
                                        LR"xml(<xs:anyAttribute processContents="skip"/>)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                            LR"xml(</xs:sequence>)xml"
                            LR"xml(<xs:attribute name="root" type="xs:string" use="required" />)xml"
                            LR"xml(<xs:attribute name="startIndexAt" type="xs:string" use="required" />)xml"
                        LR"xml(</xs:complexType>)xml"
                    LR"xml(</xs:element>)xml"
                LR"xml(</xs:sequence>)xml"
                LR"xml(<xs:attribute name="targetOsVersion" type="xs:string" use="optional" />)xml"
                LR"xml(<xs:attribute name="targetPlatform" type="xs:string" use="optional" />)xml"
                LR"xml(<xs:attribute name="majorVersion" type="xs:positiveInteger" use="optional" />)xml"
                LR"xml(<xs:attribute name="isDeploymentMergeable" type="xs:boolean" use="optional" />)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

template<typename Container>
Container WstringContainerFromDelimitedString(const std::wstring& source, const wchar_t delimiter)
{
    Container result;
    auto current = source.begin();
    while (current != source.end())
    {
        auto delimiterPosition = current;
        while ((delimiterPosition != source.end()) && (*delimiterPosition != delimiter))
        {
            ++delimiterPosition;
        }
        if (current != delimiterPosition)
        {
            result.push_back(std::wstring(current, delimiterPosition));
        }
        if (delimiterPosition == source.end())
        {
            break;
        }
        current = delimiterPosition + 1;
    }
    return result;
}

template<typename Iterator>
Iterator RemoveIfDuplicate(Iterator begin, const Iterator end)
{
    Iterator result = end;
    Iterator current = begin;
    if (current != end)
    {
        do
        {
            const Iterator next = current + 1;
            Iterator duplicate = next;
            while ((duplicate != result) && (duplicate->compare(*current) != 0))
            {
                ++duplicate;
            }
            if (duplicate != result)
            {
                Iterator destination = duplicate;
                Iterator source = duplicate + 1;
                while (source != result)
                {
                    if (source->compare(*current) != 0)
                    {
                        destination->assign(*source, 0, std::wstring::npos);
                        ++destination;
                    }
                    ++source;
                }
                result = destination;
            }
            current = next;
        } while (current != result);
    }
    return result;
}

template<typename Iterator>
std::wstring DelimitedStringFromItems(Iterator begin, const Iterator end)
{
    std::wstring result;
    bool first = true;
    while (begin != end)
    {
        if (!begin->empty())
        {
            if (!first)
            {
                result.append(1, L',');
            }
            result.append(*begin, 0, std::wstring::npos);
            first = false;
        }
        ++begin;
    }
    return result;
}

} // namespace

CHIndexerBase::CHIndexerBase() :
    _unknown14(nullptr),
    _pAtomPoolGroup(nullptr),
    _pMrmProfile(nullptr),
    _pMrmProfileForResourcePackGeneration(nullptr),
    _pUnifiedEnvironment(nullptr),
    _pDecisionInfoBuilder(nullptr),
    _platformVersion(MrmPlatformVersionInternal::WindowsClient8),
    _traversalSink(false),
    _indexSink(false),
    _pInputFileReader(nullptr),
    _pSchemaReader(nullptr),
    _pPreviousSchema(nullptr),
    _ulMajorVersion(1),
    _eSchemaPermission(SchemaPermission::None),
    _eRpMode(ResourcePackageMode::FatPack),
    _unknownD4(0),
    _resourceIdCompressionLevel(0),
    _useLegacyPriFileName(1),
    _pMainPackage(nullptr),
    _pFatPackage(nullptr),
    _unknown194(false)
{
    _options.m_shouldBuildDeploymentMergeablePri = true;
}

CHIndexerBase::~CHIndexerBase()
{
    delete _pDecisionInfoBuilder;

    while (!_indexPassList.empty())
    {
        delete _indexPassList.back();
        if (!_indexPassList.empty())
        {
            _indexPassList.pop_back();
        }
    }

    delete _pInputFileReader;
    delete _pMainPackage;
    delete _pFatPackage;

    for (const auto& resourcePackage : _resourcePackages)
    {
        delete resourcePackage.second;
    }

    while (!_indexSink.empty())
    {
        delete _indexSink.PopEntry();
    }

    for (CItemInstanceEntry* const entry : _disposalList)
    {
        delete entry;
    }
    _disposalList.clear();

    delete _pUnifiedEnvironment;
    delete _pAtomPoolGroup;
    delete _pMrmProfile;
    delete _pMrmProfileForResourcePackGeneration;
    delete _pSchemaReader;

    for (void* const allocation : _allocations)
    {
        ::operator delete(allocation);
    }
}

HRESULT CHIndexerBase::NewForNew(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    const wchar_t* const simpleId,
    const ULONG majorVersion,
    IDefStatusEx* const status,
    CHIndexerBase** const result)
{
    HRESULT operationResult = E_OUTOFMEMORY;
    AutoDeletePtr<CHIndexerBase> indexer(new (std::nothrow) CHIndexerBase());
    if (indexer.Data() != nullptr)
    {
        operationResult =
            indexer.Data()->InitForNew(root, projectRoot, outputDirectory, indexOptions, configurationFile, simpleId, majorVersion, status);
        if (SUCCEEDED(operationResult))
        {
            *result = indexer.Detach();
        }
    }
    return operationResult;
}

HRESULT CHIndexerBase::NewForVersioned(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    const wchar_t* const inputPriFile,
    IDefStatusEx* const status,
    CHIndexerBase** const result)
{
    HRESULT operationResult = E_OUTOFMEMORY;
    AutoDeletePtr<CHIndexerBase> indexer(new (std::nothrow) CHIndexerBase());
    if (indexer.Data() != nullptr)
    {
        indexer.Data()->_eSchemaPermission = SchemaPermission::Writable;
        HRESULT initResult = indexer.Data()->_InitializeIbc(root, projectRoot, outputDirectory, indexOptions, configurationFile, status);
        indexer.Data()->_options.m_shouldValidateDefaultQualifiers = false;
        if (SUCCEEDED(initResult))
        {
            initResult = indexer.Data()->_InitSchemaInfoFromInputFile(inputPriFile, status);
        }
        operationResult = ComputeHResult(initResult, status);
        if (SUCCEEDED(operationResult))
        {
            *result = indexer.Detach();
        }
    }
    return operationResult;
}

HRESULT CHIndexerBase::NewForResourcePack(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    const wchar_t* const unused,
    const wchar_t* const inputPriFile,
    IDefStatusEx* const status,
    CHIndexerBase** const result)
{
    HRESULT operationResult = E_OUTOFMEMORY;
    AutoDeletePtr<CHIndexerBase> indexer(new (std::nothrow) CHIndexerBase());
    if (indexer.Data() != nullptr)
    {
        operationResult = indexer.Data()->InitForResourcePack(
            root, projectRoot, outputDirectory, indexOptions, configurationFile, unused, inputPriFile, status);
        if (SUCCEEDED(operationResult))
        {
            *result = indexer.Detach();
        }
    }
    return operationResult;
}

StandalonePriFile* CHIndexerBase::GetBuiltPriFile(IDefStatusEx* const status)
{
    if (_pMainPackage != nullptr)
    {
        return _pMainPackage->GetReader(status);
    }
    if (_pFatPackage != nullptr)
    {
        return _pFatPackage->GetReader(status);
    }
    if (status != nullptr)
    {
        status->SetError(E_DEF_NOT_READY, L"" __FILE__, 501, L"", 0);
    }
    return nullptr;
}

HRESULT CHIndexerBase::GenerateMappingFiles(
    const MappingFileFormat format,
    const wchar_t* const priFileOutputLocation,
    IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    std::vector<std::wstring>* const defaultLanguages = _defaultLanguages.size() != 0 ? &_defaultLanguages : nullptr;
    if (_eRpMode == ResourcePackageMode::FatPack)
    {
        _pMainPackage->GenerateMappingFile(format, _outputFolder.GetRef(), priFileOutputLocation, nullptr, defaultLanguages, status);
    }
    else
    {
        const wchar_t* resourcePackName = _options.GetShouldUseGranularResourceSplitting() ? L"other" : nullptr;
        if (_pFatPackage->GenerateMappingFile(
                format, _outputFolder.GetRef(), priFileOutputLocation, resourcePackName, defaultLanguages, status) &&
            status->Succeeded())
        {
            for (const auto& resourcePackage : _resourcePackages)
            {
                resourcePackName = _options.GetShouldUseGranularResourceSplitting() ? L"other" : nullptr;
                resourcePackage.second->GenerateMappingFile(
                    format, _outputFolder.GetRef(), priFileOutputLocation, resourcePackName, nullptr, status);
                if (!status->Succeeded())
                {
                    break;
                }
            }
        }
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::CheckIfOutputUnderRoot(const wchar_t* outputDirectory, IDefStatusEx* const status, bool* const isUnderRoot)
{
    StringResult indexablePath;
    if (outputDirectory == nullptr)
    {
        outputDirectory = _outputFolder.GetRef();
    }
    *isUnderRoot = false;

    StringResult absoluteOutputPath;
    HRESULT result = CUtilities::GetAbsolutePath(outputDirectory, status, absoluteOutputPath);
    std::wstring normalizedOutputPath(absoluteOutputPath.GetRef());
    std::transform(normalizedOutputPath.begin(), normalizedOutputPath.end(), normalizedOutputPath.begin(), std::towlower);

    for (auto indexPass = _indexPassList.begin(); (indexPass != _indexPassList.end()) && SUCCEEDED(result); ++indexPass)
    {
        Def_HrFailed0(DefStringResult_SetCopy(indexablePath.GetStringResult(), L""), status);
        result = (*indexPass)->GetIndexablePath(status, &indexablePath);
        if (SUCCEEDED(result))
        {
            bool isFile = false;
            result = CUtilities::CheckIfFileOrFolder(indexablePath.GetRef(), status, &isFile);
            const HRESULT pathResult = result;
            if (SUCCEEDED(result) && !isFile)
            {
                std::wstring normalizedIndexablePath(indexablePath.GetRef());
                std::transform(
                    normalizedIndexablePath.begin(), normalizedIndexablePath.end(), normalizedIndexablePath.begin(), std::towlower);
                if (normalizedOutputPath.find(normalizedIndexablePath.c_str(), 0, normalizedIndexablePath.length()) == 0)
                {
                    *isUnderRoot = true;
                    break;
                }
            }

            if (status->GetWhat() == E_DEF_FSI_INVALID_FILE_TYPE)
            {
                result = S_OK;
                status->Reset();
            }
            else
            {
                result = pathResult;
            }
        }
    }
    return result;
}

HRESULT CHIndexerBase::Process(
    const wchar_t* const indexName,
    const CContentChecksumData::NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
    const std::uint32_t flags,
    IDefStatusEx* const status)
{
    HRESULT result = S_OK;
    if ((_unknownD4 == 0) && !_indexPassList.empty())
    {
        auto indexPass = _indexPassList.begin();
        while (SUCCEEDED(result))
        {
            result = (*indexPass)->Process(&_traversalSink, &_indexSink, status);
            ++indexPass;
            if (indexPass == _indexPassList.end())
            {
                if (FAILED(result))
                {
                    return result;
                }
                break;
            }
        }
        if (FAILED(result))
        {
            return result;
        }
    }

    result = _ProcessIndexSink(indexName, status);
    if (SUCCEEDED(result))
    {
        result = _ValidateDefaultLanguageQualifiers(status);
        if (SUCCEEDED(result))
        {
            result = _WriteIndexFiles(neutralLanguageCandidateCreation, flags, status);
            if (SUCCEEDED(result))
            {
                result = _ValidateFullPackage(status);
                if (SUCCEEDED(result) && (_metaDataFilePath.GetRef() != nullptr))
                {
                    result = _CreateMetaDataFile(status);
                }
            }
        }
    }
    return result;
}

HRESULT CHIndexerBase::InitForNew(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    const wchar_t* const simpleId,
    const ULONG majorVersion,
    IDefStatusEx* const status)
{
    _ulMajorVersion = majorVersion;
    HRESULT result = DefStringResult_SetCopy(_simpleId.GetStringResult(), simpleId);
    if (Def_HrFailed0(result, status))
    {
        result = status->GetHResult();
        if (FAILED(result))
        {
            // Original line: 659
            RETURN_HR(result);
        }
    }

    result = _InitializeIbc(root, projectRoot, outputDirectory, indexOptions, configurationFile, status);
    if (FAILED(result))
    {
        // Original line: 668
        RETURN_HR(result);
    }
    return S_OK;
}

HRESULT CHIndexerBase::InitForResourcePack(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    const wchar_t* const unused,
    const wchar_t* const inputPriFile,
    IDefStatusEx* const status)
{
    static_cast<void>(unused);
    _eSchemaPermission = SchemaPermission::ReadOnly;
    HRESULT result = DefStringResult_SetCopy(_indexName.GetStringResult(), L"_MRT_Dummy_IndexName_");
    if (Def_HrFailed0(result, status))
    {
        result = status->GetHResult();
        if (FAILED(result))
        {
            // Original line: 720
            RETURN_HR(result);
        }
    }

    result = _InitializeIbc(root, projectRoot, outputDirectory, indexOptions, configurationFile, status);
    if (FAILED(result))
    {
        // Original line: 729
        RETURN_HR(result);
    }

    result = _InitSchemaInfoFromInputFile(inputPriFile, status);
    if (FAILED(result))
    {
        // Original line: 731
        RETURN_HR(result);
    }

    _options.m_shouldValidateDefaultQualifiers = false;
    return status->GetHResult();
}

HRESULT CHIndexerBase::_InitSchemaInfoFromInputFile(const wchar_t* const inputPriFile, IDefStatusEx* const status)
{
    std::wstring input(inputPriFile);
    std::wstring extension;
    const std::size_t extensionPosition = input.rfind(L".", std::wstring::npos, 1);
    if (extensionPosition != std::wstring::npos)
    {
        extension.assign(input.substr(extensionPosition + 1, std::wstring::npos), 0, std::wstring::npos);
    }

    bool recognized = false;
    if (!extension.empty())
    {
        if (DefString_CompareWithOptions(extension.c_str(), L"pri", DefCompare_CaseInsensitive) == 0)
        {
            recognized = true;
            Def_HrFailed0(
                StandalonePriFile::CreateInstance(0, inputPriFile, _pMrmProfileForResourcePackGeneration, &_pInputFileReader), status);
            if (_pInputFileReader != nullptr)
            {
                Def_HrFailed0(_pInputFileReader->GetSchema(0, &_pPreviousSchema), status);
            }
        }
        else if (DefString_CompareWithOptions(extension.c_str(), L"xml", DefCompare_CaseInsensitive) == 0)
        {
            recognized = true;
            delete _pSchemaReader;
            _pSchemaReader = new (std::nothrow) CPriSchemaReader();
            if (_pSchemaReader == nullptr)
            {
                const HRESULT result = E_OUTOFMEMORY;
                if (status != nullptr)
                {
                    status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 903, L"", 0);
                }
                // Original line: 906
                RETURN_HR(result);
            }

            const HRESULT result = _pSchemaReader->InitializeFromFile(inputPriFile, status);
            if (FAILED(result))
            {
                // Original line: 907
                RETURN_HR(result);
            }
            _pPreviousSchema = _pSchemaReader->GetSchema();
        }
    }

    if (_pPreviousSchema != nullptr)
    {
        if (Def_HrFailed0(DefStringResult_SetCopy(_simpleId.GetStringResult(), _pPreviousSchema->GetSimpleId()), status))
        {
            const HRESULT result = status->GetHResult();
            if (FAILED(result))
            {
                // Original line: 917
                RETURN_HR(result);
            }
        }
    }

    if (!recognized && (status != nullptr))
    {
        status->SetError(E_DEFFILE_FORMAT_ERROR, L"" __FILE__, 923, L"", 0);
    }
    return ComputeHResult(S_OK, status);
}

HRESULT CHIndexerBase::_InitializeIbc(
    IXMLDOMNode* const root,
    const wchar_t* const projectRoot,
    const wchar_t* const outputDirectory,
    const IIndexOptions* const indexOptions,
    const wchar_t* const configurationFile,
    IDefStatusEx* const status)
{

    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    _options.Set(indexOptions);
    if (configurationFile != nullptr)
    {
        if (Def_HrFailed0(DefStringResult_SetCopy(_metaDataFilePath.GetStringResult(), configurationFile), status))
        {
            result = ComputeHResult(S_OK, status);
            if (FAILED(result))
            {
                goto Cleanup;
            }
        }
    }

    if (Def_HrFailed0(DefStringResult_SetCopy(_projectRootFolder.GetStringResult(), projectRoot), status))
    {
        result = ComputeHResult(result, status);
    }
    if (SUCCEEDED(result))
    {
        if (Def_HrFailed0(DefStringResult_SetCopy(_outputFolder.GetStringResult(), outputDirectory), status))
        {
            result = ComputeHResult(result, status);
            if (FAILED(result))
            {
                goto Cleanup;
            }
        }

        result = _ValidateConfigurationSchema(root, status);
        if (SUCCEEDED(result))
        {
            result = _InitializeEnvironment(root, status);
            if (SUCCEEDED(result))
            {
                result = ParsePackagingNode(root, status);
                if (SUCCEEDED(result))
                {
                    result = _ParseIndexNodes(root, status);
                    if (SUCCEEDED(result))
                    {
                        result = _InitializeQualifierNameTokenMap(status);
                    }
                }
            }
        }
    }

Cleanup:
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::ParsePackagingNode(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(root);
    IXMLDOMNode* packagingNode = nullptr;
    HRESULT result = xml.TryGetChildNode(L"packaging", status, &packagingNode);
    if (FAILED(result) || (packagingNode == nullptr))
    {
        result = S_OK;
        status->Reset();
    }
    else if (_platformVersion == MrmPlatformVersionInternal::WindowsClient8)
    {
        status->SetError(E_DEF_PRICONFIG_PACKAGING_INVALID_OS, L"");
    }
    else
    {
        if (_options.GetShouldBuildAutoMergePri())
        {
            status->SetError(E_DEF_PRICONFIG_INVALID_IDM_AM, L"");
        }
        else if (!_options.GetShouldBuildDeploymentMergeablePri())
        {
            status->SetError(E_DEF_PRICONFIG_PACKAGING_INVALID_IDM, L"");
        }

        if (status->Succeeded())
        {
            DefStatusEx childStatus;
            _eRpMode = ResourcePackageMode::FatPack;
            LONG numberOfNodes = 0;
            IXMLDOMNodeList* packageNodes = nullptr;
            CXmlHelper packagingXml(packagingNode);
            result = packagingXml.TryGetChildren(L"autoResourcePackage", &childStatus, &packageNodes);
            packageNodes->get_length(&numberOfNodes);
            if (static_cast<int>(numberOfNodes) > 0)
            {
                _eRpMode = ResourcePackageMode::AutoQualifier;
                result = _ParseAutoResourcePackageNodeList(packageNodes, status);
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(packageNodes));

            if (SUCCEEDED(result))
            {
                numberOfNodes = 0;
                result = packagingXml.TryGetChildren(L"resourcePackage", &childStatus, &packageNodes);
                if (SUCCEEDED(result))
                {
                    packageNodes->get_length(&numberOfNodes);
                    if (static_cast<int>(numberOfNodes) > 0)
                    {
                        if (_eRpMode != ResourcePackageMode::FatPack)
                        {
                            status->SetError(E_DEF_PRICONFIG_INVALID_MRT_ARP, L"");
                            result = status->GetHResult();
                        }
                        else
                        {
                            _eRpMode = ResourcePackageMode::Manual;
                            result = _ParseManualResourcePackageNodeList(packageNodes, status);
                        }
                    }
                }
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(packageNodes));
                if (SUCCEEDED(result) && (_eRpMode == ResourcePackageMode::FatPack))
                {
                    status->AddWarning(E_DEF_PRICONFIG_EMPTY_PACKAGING, L"");
                }
            }

            IXMLDOMNode* optionNode = nullptr;
            if (SUCCEEDED(packagingXml.TryGetChildNode(L"omitSchemaFromResourcePacks", &childStatus, &optionNode)) &&
                (optionNode != nullptr))
            {
                SetOmitSchemaFromResourcePacks(status);
                SAFE_RELEASE(optionNode);
            }

            optionNode = nullptr;
            if (SUCCEEDED(packagingXml.TryGetChildNode(L"splitLanguageVariants", &childStatus, &optionNode)) && (optionNode != nullptr))
            {
                SetSplitLanguageVariants(status);
                SAFE_RELEASE(optionNode);
            }

            optionNode = nullptr;
            if (SUCCEEDED(packagingXml.TryGetChildNode(L"granularSplit", &childStatus, &optionNode)) && (optionNode != nullptr))
            {
                if (_platformVersion == MrmPlatformVersionInternal::WindowsCoreVNext)
                {
                    _options.m_shouldUseGranularResourceSplitting = true;
                    _pMrmProfile->GetBuildConfiguration()->SetFlags(_pMrmProfile->GetBuildConfiguration()->GetFlags() | 0x100);
                    _pMrmProfileForResourcePackGeneration->GetBuildConfiguration()->SetFlags(
                        _pMrmProfileForResourcePackGeneration->GetBuildConfiguration()->GetFlags() | 0x100);
                }
                else
                {
                    status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, L"granularSplit");
                }
                SAFE_RELEASE(optionNode);
            }

            optionNode = nullptr;
            if (SUCCEEDED(packagingXml.TryGetChildNode(L"packagingOptions", &childStatus, &optionNode)) && (optionNode != nullptr))
            {
                if (_platformVersion != MrmPlatformVersionInternal::WindowsCoreVNext)
                {
                    status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, L"packagingOptions");
                }
                else
                {
                    CXmlHelper packagingOptionsXml(optionNode);
                    wchar_t* packageBy = nullptr;
                    result = packagingOptionsXml.GetAttributeValue(L"packageBy", status, &packageBy);
                    if (status->GetWhat() == E_DEF_XML_ATTRIB_NOT_FOUND)
                    {
                        status->Reset();
                        result = S_OK;
                        _resourceIdCompressionLevel = 1;
                    }
                    else if (DefString_CompareWithOptions(packageBy, L"eachQualifier", DefCompare_CaseInsensitive) == 0)
                    {
                        _resourceIdCompressionLevel = 0;
                    }
                    else if (DefString_CompareWithOptions(packageBy, L"highestPriorityQualifier", DefCompare_CaseInsensitive) == 0)
                    {
                        _resourceIdCompressionLevel = 1;
                    }
                    else
                    {
                        status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, packageBy);
                    }
                }
                SAFE_RELEASE(optionNode);
            }
        }
    }

    SAFE_RELEASE(packagingNode);
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ParseAutoResourcePackageNodeList(IXMLDOMNodeList* const nodes, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    LONG numberOfNodes = 0;
    nodes->get_length(&numberOfNodes);
    for (LONG index = 0; index < numberOfNodes; ++index)
    {
        IXMLDOMNode* node = nullptr;
        result = nodes->get_item(index, &node);
        if (SUCCEEDED(result) && (node != nullptr))
        {
            wchar_t* qualifier = nullptr;
            CXmlHelper xml(node);
            result = xml.GetAttributeValue(L"qualifier", status, &qualifier);
            if (SUCCEEDED(result))
            {
                StringResult qualifierName;
                Atom qualifierNameAtom {};
                const IBuildQualifierType* qualifierType = nullptr;
                result = CQualifierApplicator::GetQualifierNameFromNameOrToken(
                    qualifier, _pMrmProfileForResourcePackGeneration, _pUnifiedEnvironment, status, &qualifierName);
                if (SUCCEEDED(result))
                {
                    if (Def_HrFailed0(
                            _pUnifiedEnvironment->GetQualifierNameAtom(qualifierName.GetRef(), &qualifierNameAtom, nullptr), status) ||
                        Def_HrFailed0(_pUnifiedEnvironment->GetTypeOfQualifier(qualifierName.GetRef(), &qualifierType), status))
                    {
                        status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, qualifierName.GetRef());
                    }
                    else if ((qualifierType->GetDefaultPackagingFlags() & IBuildQualifierType::PackagingAllowResourcePackage) != 0)
                    {
                        bool qualifierAlreadyPresent = false;
                        for (auto atom = _arpQualifierList.begin(); atom != _arpQualifierList.end(); ++atom)
                        {
                            if ((atom->GetPoolIndex() == qualifierNameAtom.GetPoolIndex()) &&
                                (atom->GetIndex() == qualifierNameAtom.GetIndex()))
                            {
                                qualifierAlreadyPresent = true;
                                break;
                            }
                        }
                        if (!qualifierAlreadyPresent)
                        {
                            _arpQualifierList.push_back(qualifierNameAtom);
                            result = _ParseAutoPackagesNode(node, qualifierName.GetRef(), status);
                        }
                    }
                    else
                    {
                        status->SetError(E_DEF_PRICONFIG_NON_ARP_QUALIFIER, qualifierName.GetRef());
                    }
                }
            }
            delete[] qualifier;
        }
        SAFE_RELEASE(node);
        if (FAILED(result))
        {
            break;
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ParseAutoPackagesNode(IXMLDOMNode* const node, const wchar_t* const qualifierName, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(node);
    IXMLDOMNode* autoPackagesNode = nullptr;
    HRESULT result = xml.TryGetChildNode(L"autoPackages", status, &autoPackagesNode);
    if (SUCCEEDED(result) && (autoPackagesNode != nullptr))
    {
        const std::wstring qualifierNameString(qualifierName);
        const std::wstring valueNodeName = qualifierNameString + L"Value";
        CXmlHelper autoPackagesXml(autoPackagesNode);
        IXMLDOMNodeList* valueNodes = nullptr;
        result = autoPackagesXml.TryGetChildren(valueNodeName.c_str(), status, &valueNodes);
        if (SUCCEEDED(result) && (valueNodes != nullptr))
        {
            LONG numberOfNodes = 0;
            result = valueNodes->get_length(&numberOfNodes);
            if (static_cast<int>(numberOfNodes) <= 0)
            {
                status->SetError(E_DEF_XML_NODE_NOT_FOUND, valueNodeName.c_str());
            }
            else
            {
                std::set<std::wstring> qualifierValues;
                for (LONG index = 0; SUCCEEDED(result) && index < numberOfNodes; ++index)
                {
                    IXMLDOMNode* valueNode = nullptr;
                    result = valueNodes->get_item(index, &valueNode);
                    if (SUCCEEDED(result) && (valueNode != nullptr))
                    {
                        wchar_t* valueText = nullptr;
                        CXmlHelper valueXml(valueNode);
                        result = valueXml.GetNodeText(status, &valueText);
                        if (SUCCEEDED(result) && (valueText != nullptr))
                        {
                            const std::wstring value(valueText);
                            int qualifierSetIndex;
                            bool isDefault = false;
                            if (_GetQsiFromQualNameValue(
                                    qualifierNameString.c_str(), value.c_str(), status, &qualifierSetIndex, &isDefault))
                            {
                                if (!isDefault)
                                {
                                    const std::wstring description = qualifierNameString + L"-" + value;
                                    status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, description.c_str());
                                    result = status->GetHResult();
                                }
                                if (SUCCEEDED(result))
                                {
                                    qualifierValues.insert(value);
                                }
                            }
                            delete[] valueText;
                        }
                    }
                    SAFE_RELEASE(valueNode);
                }

                _autoPackages.insert(std::make_pair(std::wstring(qualifierName), qualifierValues));
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(valueNodes));
        }
        SAFE_RELEASE(autoPackagesNode);
    }
    else
    {
        result = S_OK;
        status->Reset();
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ParseManualResourcePackageNodeList(IXMLDOMNodeList* const nodes, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    LONG numberOfNodes = 0;
    nodes->get_length(&numberOfNodes);
    for (LONG index = 0; index < numberOfNodes; ++index)
    {
        IXMLDOMNode* node = nullptr;
        result = nodes->get_item(index, &node);
        if (SUCCEEDED(result) && (node != nullptr))
        {
            wchar_t* packageName = nullptr;
            CXmlHelper xml(node);
            result = xml.GetAttributeValue(L"name", status, &packageName);
            if (SUCCEEDED(result))
            {
                if (_resourcePackages.find(std::wstring(packageName)) != _resourcePackages.end())
                {
                    status->SetError(E_DEF_PRICONFIG_INVALID_MRP_DUP_NAME, packageName);
                }
                else
                {
                    LONG numberOfQualifierSets = 0;
                    IXMLDOMNodeList* qualifierSetNodes = nullptr;
                    CXmlHelper packageXml(node);
                    result = packageXml.TryGetChildren(L"qualifierSet", status, &qualifierSetNodes);
                    if (SUCCEEDED(result))
                    {
                        nodes->get_length(&numberOfQualifierSets);
                        if (static_cast<int>(numberOfQualifierSets) > 0)
                        {
                            result = _ParseManualQualifierSetNodeList(qualifierSetNodes, packageName, status);
                            if (SUCCEEDED(result))
                            {
                                _resourcePackages.insert(std::make_pair(std::wstring(packageName), static_cast<CPackageInfo*>(nullptr)));
                            }
                        }
                    }
                    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(qualifierSetNodes));
                }
            }
            delete[] packageName;
        }
        SAFE_RELEASE(node);
        if (FAILED(result))
        {
            break;
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ParseManualQualifierSetNodeList(
    IXMLDOMNodeList* const nodes,
    const wchar_t* const packageName,
    IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    LONG numberOfNodes = 0;
    nodes->get_length(&numberOfNodes);
    for (LONG index = 0; index < numberOfNodes; ++index)
    {
        IXMLDOMNode* node = nullptr;
        result = nodes->get_item(index, &node);
        if (SUCCEEDED(result) && (node != nullptr))
        {
            wchar_t* definition = nullptr;
            CXmlHelper xml(node);
            result = xml.GetAttributeValue(L"definition", status, &definition);
            if (SUCCEEDED(result))
            {
                std::map<std::wstring, std::wstring> qualifiers;
                const std::wstring definitionString(definition);
                result = CUtilities::GetQualifierMapFromQualifierTag(std::wstring(definitionString), &qualifiers);
                if (FAILED(result))
                {
                    status->SetError(result, definitionString.c_str());
                }
                else
                {
                    if (qualifiers.size() > 1)
                    {
                        status->SetError(E_DEF_PRICONFIG_INVALID_MRP_MULTIQUAL, definitionString.c_str());
                        result = status->GetHResult();
                    }
                    if (SUCCEEDED(result))
                    {
                        const auto qualifier = qualifiers.begin();
                        int qualifierSetIndex;
                        bool isDefault = false;
                        if (_GetQsiFromQualNameValue(
                                qualifier->first.c_str(), qualifier->second.c_str(), status, &qualifierSetIndex, &isDefault))
                        {
                            if (!isDefault)
                            {
                                std::wstring description = qualifier->first + L"-";
                                description.append(qualifier->second, 0, std::wstring::npos);
                                status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, description.c_str());
                                result = status->GetHResult();
                            }
                            if (SUCCEEDED(result))
                            {
                                if (_manualQsiToRpNameMap.find(qualifierSetIndex) == _manualQsiToRpNameMap.end())
                                {
                                    _manualQsiToRpNameMap.insert(std::make_pair(qualifierSetIndex, std::wstring(packageName)));
                                }
                                else
                                {
                                    status->SetError(E_DEF_PRICONFIG_INVALID_MRP_DUP_QSI, definitionString.c_str());
                                }
                            }
                        }
                    }
                }
            }
            delete[] definition;
        }
        SAFE_RELEASE(node);
        if (FAILED(result))
        {
            break;
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

bool CHIndexerBase::_GetQsiFromQualNameValue(
    const wchar_t* const qualifierName,
    const wchar_t* const qualifierValue,
    IDefStatusEx* const status,
    int* const qualifierSetIndex,
    bool* const isDefault)
{
    bool result = false;
    *qualifierSetIndex = 0;
    *isDefault = false;
    StringResult normalizedQualifierName;
    if (SUCCEEDED(
            CQualifierApplicator::GetQualifierNameFromNameOrToken(
                qualifierName, _pMrmProfileForResourcePackGeneration, _pUnifiedEnvironment, status, &normalizedQualifierName)))
    {
        CQualifierApplicator* applicator = new (std::nothrow)
            CQualifierApplicator(nullptr, _pMrmProfileForResourcePackGeneration, _pUnifiedEnvironment, _pDecisionInfoBuilder, nullptr);
        if ((status != nullptr) && status->Failed())
        {
            delete applicator;
        }
        else if (applicator != nullptr)
        {
            CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
            if (SUCCEEDED(applicator->GetQualifierSetBuilder(0, status, &builder)))
            {
                if (SUCCEEDED(builder->_AddQualifier(
                        normalizedQualifierName.GetRef(),
                        qualifierValue,
                        nullptr,
                        nullptr,
                        CQualifierApplicator::tagTOKEN_TYPE::tokenDefault,
                        isDefault,
                        status)))
                {
                    applicator->ApplyQualifierSetFromBuilder(builder, status, qualifierSetIndex);
                }
            }
            delete builder;
            delete applicator;
            result = status->Succeeded();
        }
        else if (status != nullptr)
        {
            status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 2221, L"", 0);
        }
    }
    else
    {
        status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, qualifierName);
    }
    return result;
}

bool CHIndexerBase::_GetQsiFromQualifierResult(QualifierResult qualifier, IDefStatusEx* const status, int* const qualifierSetIndex)
{
    bool result = false;
    HRESULT addResult = S_OK;
    *qualifierSetIndex = 0;
    CQualifierApplicator* applicator = new (std::nothrow)
        CQualifierApplicator(nullptr, _pMrmProfileForResourcePackGeneration, _pUnifiedEnvironment, _pDecisionInfoBuilder, nullptr);
    if ((status != nullptr) && status->Failed())
    {
        delete applicator;
    }
    else if (applicator != nullptr)
    {
        CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
        if (SUCCEEDED(applicator->GetQualifierSetBuilder(0, status, &builder)))
        {
            double fallbackScore;
            StringResult qualifierName;
            StringResult qualifierValue;
            Atom qualifierNameAtom {};
            if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierNameAtom), status) &&
                _pAtomPoolGroup->TryGetString(qualifierNameAtom, &qualifierName))
            {
                if (!Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status))
                {
                    Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), status);
                    bool applied = false;
                    int priority = qualifier.GetPriority();
                    addResult = builder->_AddQualifier(
                        qualifierName.GetRef(),
                        qualifierValue.GetRef(),
                        &fallbackScore,
                        &priority,
                        CQualifierApplicator::tagTOKEN_TYPE::tokenDefault,
                        &applied,
                        status);
                }
            }

            if (status->Succeeded() && SUCCEEDED(addResult))
            {
                applicator->ApplyQualifierSetFromBuilder(builder, status, qualifierSetIndex);
            }
        }
        delete builder;
        delete applicator;
        result = status->Succeeded();
    }
    else if (status != nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 2319, L"", 0);
    }
    return result;
}

bool CHIndexerBase::_GetQsiFromQualifierResultArray(
    std::vector<QualifierResult>* const qualifiers,
    IDefStatusEx* const status,
    int* const qualifierSetIndex)
{
    HRESULT addResult = S_OK;
    *qualifierSetIndex = 0;
    CQualifierApplicator* applicator = new (std::nothrow)
        CQualifierApplicator(nullptr, _pMrmProfileForResourcePackGeneration, _pUnifiedEnvironment, _pDecisionInfoBuilder, nullptr);
    if ((status != nullptr) && status->Failed())
    {
        delete applicator;
        return false;
    }
    if (applicator == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 2259, L"", 0);
        }
        return false;
    }

    CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
    if (SUCCEEDED(applicator->GetQualifierSetBuilder(0, status, &builder)))
    {
        auto qualifierIterator = qualifiers->begin();
        while (status->Succeeded() && (qualifierIterator != qualifiers->end()))
        {
            QualifierResult qualifier(*qualifierIterator);
            Atom qualifierNameAtom {};
            StringResult qualifierName;
            StringResult qualifierValue;
            if (Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierNameAtom), status) ||
                !_pAtomPoolGroup->TryGetString(qualifierNameAtom, &qualifierName) ||
                Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status))
            {
                break;
            }

            double fallbackScore;
            Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), status);
            bool applied = false;
            int priority = qualifier.GetPriority();
            addResult = builder->_AddQualifier(
                qualifierName.GetRef(),
                qualifierValue.GetRef(),
                &fallbackScore,
                &priority,
                CQualifierApplicator::tagTOKEN_TYPE::tokenDefault,
                &applied,
                status);
            ++qualifierIterator;
        }

        if (status->Succeeded() && SUCCEEDED(addResult))
        {
            applicator->ApplyQualifierSetFromBuilder(builder, status, qualifierSetIndex);
        }
    }
    delete builder;
    delete applicator;
    return status->Succeeded();
}

bool CHIndexerBase::_FindDominantQsi(std::vector<int>* const qualifierSetIndices, IDefStatusEx* const status, int* const qualifierSetIndex)
{
    QualifierSetResult qualifierSet;
    *qualifierSetIndex = 0;
    QualifierResult dominantQualifier;
    ResourceQualifier dominantResourceQualifier;
    dominantResourceQualifier.defaultBuildPriority = 0;
    auto qualifierSetIterator = qualifierSetIndices->begin();
    while (status->Succeeded() && (qualifierSetIterator != qualifierSetIndices->end()))
    {
        if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(*qualifierSetIterator, &qualifierSet), status))
        {
            QualifierResult qualifier;
            if (!Def_HrFailed0(qualifierSet.GetQualifier(0, &qualifier, nullptr), status))
            {
                Atom qualifierName {};
                if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierName), status))
                {
                    ResourceQualifier resourceQualifier;
                    if (!Def_HrFailed0(_pUnifiedEnvironment->GetResourceQualifier(qualifierName, &resourceQualifier), status) &&
                        (resourceQualifier.defaultBuildPriority > dominantResourceQualifier.defaultBuildPriority))
                    {
                        dominantQualifier = qualifier;
                        dominantResourceQualifier = resourceQualifier;
                    }
                }
            }
        }
        ++qualifierSetIterator;
    }

    _GetQsiFromQualifierResult(dominantQualifier, status, qualifierSetIndex);
    return status->Succeeded();
}

bool CHIndexerBase::_FindEffectiveQsi(const int qualifierSetIndex, IDefStatusEx* const status, int* const effectiveQualifierSetIndex)
{
    *effectiveQualifierSetIndex = 0;
    const auto cachedQualifierSet = _multiToSingleQsiMap.lower_bound(qualifierSetIndex);
    if ((cachedQualifierSet != _multiToSingleQsiMap.end()) && !(qualifierSetIndex < cachedQualifierSet->first) &&
        (cachedQualifierSet != _multiToSingleQsiMap.end()))
    {
        *effectiveQualifierSetIndex = cachedQualifierSet->second;
        return true;
    }

    QualifierSetResult qualifierSet;
    QualifierResult dominantQualifier;
    ResourceQualifier dominantResourceQualifier;
    dominantResourceQualifier.defaultBuildPriority = 0;
    if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
    {
        std::vector<QualifierResult> arpQualifiers;
        for (int qualifierIndex = 0; qualifierIndex < qualifierSet.GetNumQualifiers(); ++qualifierIndex)
        {
            QualifierResult qualifier;
            if (!Def_HrFailed0(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr), status))
            {
                Atom qualifierName {};
                if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierName), status) && _IsArpQualifier(qualifierName))
                {
                    ResourceQualifier resourceQualifier;
                    if (!Def_HrFailed0(_pUnifiedEnvironment->GetResourceQualifier(qualifierName, &resourceQualifier), status) &&
                        (resourceQualifier.defaultBuildPriority > dominantResourceQualifier.defaultBuildPriority))
                    {
                        dominantQualifier = qualifier;
                        dominantResourceQualifier = resourceQualifier;
                    }
                    arpQualifiers.push_back(qualifier);
                }
            }
        }

        bool found;
        if ((_platformVersion == MrmPlatformVersionInternal::WindowsCoreVNext) && (_resourceIdCompressionLevel == 0))
        {
            found = _GetQsiFromQualifierResultArray(&arpQualifiers, status, effectiveQualifierSetIndex);
        }
        else
        {
            found = _GetQsiFromQualifierResult(dominantQualifier, status, effectiveQualifierSetIndex);
        }
        if (found)
        {
            _multiToSingleQsiMap.insert(std::make_pair(qualifierSetIndex, *effectiveQualifierSetIndex));
        }
    }
    return status->Succeeded();
}

bool CHIndexerBase::_HasDefaultOrNeutralQualifier(const int qualifierSetIndex, IDefStatusEx* const status)
{
    QualifierSetResult qualifierSet;
    if (Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
    {
        return false;
    }
    if (qualifierSet.GetNumQualifiers() == 0)
    {
        return true;
    }

    Atom scaleQualifier {};
    Def_HrFailed0(_pUnifiedEnvironment->GetQualifierNameAtom(L"Scale", &scaleQualifier, nullptr), status);
    for (int index = 0; index < qualifierSet.GetNumQualifiers(); ++index)
    {
        QualifierResult qualifier;
        if (!Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, nullptr), status))
        {
            Atom qualifierName {};
            if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierName), status))
            {
                if ((qualifierName.GetPoolIndex() == scaleQualifier.GetPoolIndex()) &&
                    (qualifierName.GetIndex() == scaleQualifier.GetIndex()))
                {
                    double fallbackScore;
                    Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), status);
                    if (fallbackScore == 1.0)
                    {
                        return true;
                    }
                }
                else
                {
                    double fallbackScore;
                    Def_HrFailed0(qualifier.GetFallbackScore(&fallbackScore), status);
                    if (fallbackScore > 0.0)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool CHIndexerBase::_GetOverlappingQsiMap(
    const int qualifierSetIndex,
    IDefStatusEx* const status,
    std::map<int, int>* const overlappingQualifierSets)
{
    QualifierSetResult qualifierSet;
    if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
    {
        for (int qualifierIndex = 0; status->Succeeded() && (qualifierIndex < qualifierSet.GetNumQualifiers()); ++qualifierIndex)
        {
            QualifierResult qualifier;
            if (!Def_HrFailed0(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr), status))
            {
                Atom qualifierNameAtom {};
                StringResult qualifierName;
                StringResult qualifierValue;
                Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierNameAtom), status);
                _pAtomPoolGroup->TryGetString(qualifierNameAtom, &qualifierName);
                Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status);

                int manualQualifierSetIndex;
                bool isDefault;
                if (_GetQsiFromQualNameValue(qualifierName.GetRef(), qualifierValue.GetRef(), status, &manualQualifierSetIndex, &isDefault))
                {
                    if (_manualQsiToRpNameMap.find(manualQualifierSetIndex) != _manualQsiToRpNameMap.end())
                    {
                        int singleQualifierSetIndex;
                        if (_GetQsiFromQualifierResult(qualifier, status, &singleQualifierSetIndex))
                        {
                            overlappingQualifierSets->insert(std::make_pair(singleQualifierSetIndex, manualQualifierSetIndex));
                        }
                    }
                }
            }
        }
    }
    return status->Succeeded();
}

bool CHIndexerBase::_GetPackagingAffinityForQualifier(
    QualifierResult* const qualifier,
    IDefStatusEx* const status,
    IBuildQualifierType::PackagingFlags* const packagingFlags,
    StringResult* const affinity)
{
    bool result = false;
    Atom qualifierNameAtom = Atom::NullAtom;
    const IBuildQualifierType* qualifierType = nullptr;
    StringResult packagingAffinity;
    if (Def_HrFailed0(qualifier->GetOperand1Attribute(&qualifierNameAtom), status) ||
        Def_HrFailed0(_pUnifiedEnvironment->GetTypeOfQualifier(qualifierNameAtom, &qualifierType), status))
    {
        return false;
    }

    const wchar_t** autoPackageValues = nullptr;
    std::uint32_t numberOfAutoPackageValues = 0;
    StringResult qualifierName;
    Def_HrFailed0(
        _pUnifiedEnvironment->GetName(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, qualifierNameAtom, &qualifierName), status);
    const auto configuredValues = _autoPackages.find(std::wstring(qualifierName.GetRef()));
    if (configuredValues != _autoPackages.end())
    {
        std::set<std::wstring> values(configuredValues->second);
        autoPackageValues = new (std::nothrow) const wchar_t*[values.size()]();
        if (autoPackageValues == nullptr)
        {
            if (status != nullptr)
            {
                status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 2689, L"", 0);
            }
            return false;
        }

        for (auto value = values.begin(); value != values.end(); ++value)
        {
            const std::size_t valueLength = std::wcslen(value->c_str()) + 1;
            wchar_t* const valueCopy = new (std::nothrow) wchar_t[valueLength];
            autoPackageValues[numberOfAutoPackageValues] = valueCopy;
            if (valueCopy == nullptr)
            {
                if (status != nullptr)
                {
                    status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 2703, L"", 0);
                }
                break;
            }
            wcscpy_s(valueCopy, valueLength, value->c_str());
            ++numberOfAutoPackageValues;
        }
    }

    if (status->Succeeded())
    {
        if (!Def_HrFailed0(
                qualifierType->GetPackagingInfo(
                    qualifier,
                    _pMrmProfile->GetBuildConfiguration()->GetFlags(),
                    autoPackageValues,
                    numberOfAutoPackageValues,
                    packagingFlags,
                    &packagingAffinity),
                status))
        {
            if (DefString_CompareWithOptions(packagingAffinity.GetRef(), L"default", DefCompare_Default) == 0)
            {
                result = !Def_HrFailed0(affinity->SetContentsFromOther(&packagingAffinity), status);
            }
            else if (
                !Def_HrFailed0(affinity->SetRef(qualifierName.GetRef()), status) &&
                !Def_HrFailed0(DefStringResult_Concat(affinity->GetStringResult(), L"-"), status) &&
                !Def_HrFailed0(DefStringResult_Concat(affinity->GetStringResult(), packagingAffinity.GetRef()), status))
            {
                wchar_t* writableAffinity;
                Def_HrFailed0(affinity->GetWritableRef(&writableAffinity, nullptr), status);
                if (*writableAffinity != L'\0')
                {
                    wchar_t* current = writableAffinity;
                    do
                    {
                        *current = static_cast<wchar_t>(std::towlower(*current));
                        ++current;
                    } while (*current != L'\0');
                }
                result = true;
            }
        }
    }

    if (autoPackageValues != nullptr)
    {
        for (std::uint32_t index = 0; index < numberOfAutoPackageValues; ++index)
        {
            if (autoPackageValues[index] != nullptr)
            {
                delete[] autoPackageValues[index];
            }
        }
        delete[] autoPackageValues;
    }
    return result;
}

bool CHIndexerBase::_GetPackagingAffinityForQualifierSet(
    const int qualifierSetIndex,
    IDefStatusEx* const status,
    StringResult* const affinity)
{
    QualifierSetResult qualifierSet;
    IBuildQualifierType::PackagingFlags packagingFlags = IBuildQualifierType::PackagingDefaultFlags;
    bool result = true;
    if (Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
    {
        return false;
    }

    bool hasDefaultAffinity = false;
    for (int qualifierIndex = 0; (qualifierIndex < qualifierSet.GetNumQualifiers()) && status->Succeeded(); ++qualifierIndex)
    {
        QualifierResult qualifier;
        if (!Def_HrFailed0(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr), status))
        {
            StringResult qualifierAffinity;
            if (!_GetPackagingAffinityForQualifier(&qualifier, status, &packagingFlags, &qualifierAffinity))
            {
                result = false;
                break;
            }
            if ((DefString_CompareWithOptions(qualifierAffinity.GetRef(), L"default", DefCompare_Default) != 0) &&
                ((packagingFlags & IBuildQualifierType::PackagingAllowResourcePackage) != 0))
            {
                if (affinity->GetLength() != 0)
                {
                    Def_HrFailed0(DefStringResult_Concat(affinity->GetStringResult(), L"."), status);
                }
                Def_HrFailed0(DefStringResult_Concat(affinity->GetStringResult(), qualifierAffinity.GetRef()), status);
            }
            else
            {
                hasDefaultAffinity = true;
            }
        }
    }

    if ((affinity->GetLength() == 0) && hasDefaultAffinity)
    {
        Def_HrFailed0(DefStringResult_Concat(affinity->GetStringResult(), L"default"), status);
    }
    return result;
}

HRESULT CHIndexerBase::_AddEntriesToManualResourcePacks(IDefStatusEx* const status)
{

    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    std::map<int, int> effectiveQualifierSets;
    while (!_indexSink.empty() && status->Succeeded())
    {
        int effectiveQualifierSetIndex = -1;
        CItemInstanceEntry* const entry = _indexSink.PopEntry();
        const int qualifierSetIndex = entry->qualifierSetIndex;
        const auto cachedQualifierSet = effectiveQualifierSets.lower_bound(qualifierSetIndex);
        if ((cachedQualifierSet == effectiveQualifierSets.end()) || (qualifierSetIndex < cachedQualifierSet->first))
        {
            std::map<int, int> overlappingQualifierSets;
            if (_GetOverlappingQsiMap(qualifierSetIndex, status, &overlappingQualifierSets))
            {
                if (!overlappingQualifierSets.empty())
                {
                    if (overlappingQualifierSets.size() == 1)
                    {
                        effectiveQualifierSetIndex = overlappingQualifierSets.begin()->first;
                    }
                    else
                    {
                        std::vector<int> qualifierSetIndices;
                        for (auto qualifierSet = overlappingQualifierSets.begin(); qualifierSet != overlappingQualifierSets.end();
                             ++qualifierSet)
                        {
                            qualifierSetIndices.push_back(qualifierSet->first);
                        }
                        _FindDominantQsi(&qualifierSetIndices, status, &effectiveQualifierSetIndex);
                    }

                    if (status->Succeeded())
                    {
                        if ((effectiveQualifierSetIndex > 0) && _HasDefaultOrNeutralQualifier(effectiveQualifierSetIndex, status))
                        {
                            std::wstring qualifierTag;
                            CUtilities::GetQualifierTagFromQualifierSetIndex(
                                _pDecisionInfoBuilder, _pAtomPoolGroup, effectiveQualifierSetIndex, status, qualifierTag);
                            status->AddWarning(E_DEF_PRICONFIG_INVALID_MRP_DEFAULT, qualifierTag.c_str());
                        }

                        auto overlappingQualifierSet = overlappingQualifierSets.lower_bound(effectiveQualifierSetIndex);
                        if ((overlappingQualifierSet == overlappingQualifierSets.end()) ||
                            (effectiveQualifierSetIndex < overlappingQualifierSet->first))
                        {
                            overlappingQualifierSet =
                                overlappingQualifierSets.insert(overlappingQualifierSet, std::make_pair(effectiveQualifierSetIndex, 0));
                        }
                        effectiveQualifierSetIndex = overlappingQualifierSet->second;
                    }
                }

                effectiveQualifierSets.insert(std::make_pair(qualifierSetIndex, effectiveQualifierSetIndex));
            }
        }
        else
        {
            effectiveQualifierSetIndex = cachedQualifierSet->second;
        }

        if (status->Succeeded())
        {
            if (effectiveQualifierSetIndex >= 0)
            {
                const auto packageName = _manualQsiToRpNameMap.find(effectiveQualifierSetIndex);
                result = _AddEntryToPackageWithName(
                    effectiveQualifierSetIndex, _priFileName.GetRef(), packageName->second.c_str(), entry, status);
            }
            else
            {
                _pFatPackage->AddEntry(entry, status);
            }
        }
    }

    auto resourcePackage = _resourcePackages.begin();
    if (status->Succeeded())
    {
        while (resourcePackage != _resourcePackages.end())
        {
            if (resourcePackage->second != nullptr)
            {
                ++resourcePackage;
            }
            else
            {
                status->AddWarning(E_DEF_PRICONFIG_INVALID_MRP_EMPTY, resourcePackage->first.c_str());
                const auto unusedResourcePackage = resourcePackage;
                ++resourcePackage;
                _resourcePackages.erase(unusedResourcePackage);
            }
            if (!status->Succeeded())
            {
                break;
            }
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_AddEntryToPackageWithName(
    const int qualifierSetIndex,
    const wchar_t* const mainPackageName,
    const wchar_t* const resourcePackageName,
    CItemInstanceEntry* const entry,
    IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    const auto resourcePackage = _resourcePackages.find(std::wstring(resourcePackageName));
    if ((resourcePackage == _resourcePackages.end()) || (resourcePackage->second == nullptr))
    {
        CPackageInfo* const package = CPackageInfo::New(
            mainPackageName,
            resourcePackageName,
            &_options,
            _pDecisionInfoBuilder,
            _pUnifiedEnvironment,
            _pAtomPoolGroup,
            _pMrmProfileForResourcePackGeneration,
            this,
            status);
        if (package != nullptr)
        {
            if (package->AddEntry(qualifierSetIndex, entry, status))
            {
                if (resourcePackage == _resourcePackages.end())
                {
                    _resourcePackages.insert(std::make_pair(std::wstring(resourcePackageName), package));
                }
                else
                {
                    resourcePackage->second = package;
                }
            }
            else
            {
                delete package;
            }
        }
    }
    else
    {
        resourcePackage->second->AddEntry(qualifierSetIndex, entry, status);
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_GenerateArpsAndAddEntries(const wchar_t* const mainPackageName, IDefStatusEx* const status)
{

    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    while (!_indexSink.empty())
    {
        CItemInstanceEntry* const entry = _indexSink.PopEntry();
        const int qualifierSetIndex = entry->qualifierSetIndex;
        QualifierSetResult qualifierSet;
        if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
        {
            if (_HasArpQualifier(qualifierSet, status))
            {
                int effectiveQualifierSetIndex = 0;
                int packageQualifierSetIndex = qualifierSetIndex;
                if (qualifierSet.GetNumQualifiers() > 1)
                {
                    _FindEffectiveQsi(qualifierSetIndex, status, &effectiveQualifierSetIndex);
                    packageQualifierSetIndex = effectiveQualifierSetIndex;
                }

                StringResult affinity;
                if (_GetPackagingAffinityForQualifierSet(packageQualifierSetIndex, status, &affinity))
                {
                    if (DefString_CompareWithOptions(affinity.GetRef(), L"default", DefCompare_Default) != 0)
                    {
                        result = _AddEntryToPackageWithName(packageQualifierSetIndex, mainPackageName, affinity.GetRef(), entry, status);
                    }
                    else
                    {
                        _pFatPackage->AddEntry(entry, status);
                    }
                }
            }
            else if (_options.GetShouldUseGranularResourceSplitting())
            {
                result = _AddEntryToPackageWithName(_pDecisionInfoBuilder->GetNumQualifierSets(), mainPackageName, L"other", entry, status);
            }
            else
            {
                _pFatPackage->AddEntry(entry, status);
            }
        }
    }

    CalibrateResourceIdCompressionLevel(status);
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

void CHIndexerBase::CalibrateResourceIdCompressionLevel(IDefStatusEx* const status)
{
    auto resourcePackage = _resourcePackages.begin();
    if (status->Succeeded())
    {
        while (resourcePackage != _resourcePackages.end())
        {
            CPackageInfo* const package = resourcePackage->second;
            if (std::wstring(package->_strResourcePackName.GetRef()).length() > 30)
            {
                _resourceIdCompressionLevel = 1;
                break;
            }
            ++resourcePackage;
            if (!status->Succeeded())
            {
                break;
            }
        }
    }

    while (status->Succeeded() && (resourcePackage != _resourcePackages.end()))
    {
        CPackageInfo* const package = resourcePackage->second;
        StringResult resourceId;
        if (package->_ConstructResourceIdTokenAndValues(package->_strResourcePackName.GetRef(), status, &resourceId))
        {
            std::size_t resourceIdLength;
            DefStringResult_GetLength(resourceId.GetStringResult(), &resourceIdLength);
            if (resourceIdLength > 30)
            {
                _resourceIdCompressionLevel = 2;
                break;
            }
        }
        ++resourcePackage;
    }

    if (status->Succeeded())
    {
        while (resourcePackage != _resourcePackages.end())
        {
            CPackageInfo* const package = resourcePackage->second;
            StringResult resourceId;
            if (package->_ConstructResourceIdValuesOnly(package->_strResourcePackName.GetRef(), status, &resourceId))
            {
                std::size_t resourceIdLength;
                DefStringResult_GetLength(resourceId.GetStringResult(), &resourceIdLength);
                if (resourceIdLength > 30)
                {
                    _resourceIdCompressionLevel = 3;
                    return;
                }
            }
            ++resourcePackage;
            if (!status->Succeeded())
            {
                return;
            }
        }
    }
}

HRESULT CHIndexerBase::_ProcessIndexSink(const wchar_t* const indexName, IDefStatusEx* const status)
{

    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    const wchar_t* const effectiveIndexName = indexName != nullptr ? indexName : L"resources";
    Def_HrFailed0(DefStringResult_SetCopy(_priFileName.GetStringResult(), effectiveIndexName), status);

    MrmProfile* packageProfile;
    if ((_eSchemaPermission == SchemaPermission::ReadOnly) && _options.GetShouldOmitSchemaFromResourcePacks())
    {
        packageProfile = _pMrmProfileForResourcePackGeneration;
    }
    else
    {
        packageProfile = _pMrmProfile;
    }

    _pMainPackage = CPackageInfo::New(
        effectiveIndexName, nullptr, &_options, _pDecisionInfoBuilder, _pUnifiedEnvironment, _pAtomPoolGroup, packageProfile, this, status);
    if (_pMainPackage != nullptr)
    {
        std::uint32_t entryIndex = 0;
        if (_indexSink.GetNumberOfEntries() != 0)
        {
            while (status->Succeeded())
            {
                CItemInstanceEntry* const entry = _indexSink.GetEntry(entryIndex);
                _pMainPackage->AddReferencedEntry(entry, status);

                ++entryIndex;
                if (entryIndex >= _indexSink.GetNumberOfEntries())
                {
                    break;
                }
            }
        }

        if (_eRpMode != ResourcePackageMode::FatPack)
        {
            _pFatPackage = CPackageInfo::New(
                effectiveIndexName,
                nullptr,
                &_options,
                _pDecisionInfoBuilder,
                _pUnifiedEnvironment,
                _pAtomPoolGroup,
                _pMrmProfile,
                this,
                status);
            if (_pFatPackage != nullptr)
            {
                if (_eRpMode == ResourcePackageMode::AutoQualifier)
                {
                    result = _GenerateArpsAndAddEntries(effectiveIndexName, status);
                }
                else if (_eRpMode == ResourcePackageMode::Manual)
                {
                    result = _AddEntriesToManualResourcePacks(status);
                }
            }
        }
        else
        {
            _pMainPackage->ComputeReportedQualifierSetIndices(status);
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_CollectDefaultLanguages(IDefStatusEx* const status)
{

    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    auto indexPass = _indexPassList.begin();
    while ((indexPass != _indexPassList.end()) && SUCCEEDED(result))
    {
        StringResult defaultQualifierValues;
        result = (*indexPass)->GetDefaultQualifierValues(nullptr, status, &defaultQualifierValues);
        if (SUCCEEDED(result) && (defaultQualifierValues.GetRef() != nullptr) && (*defaultQualifierValues.GetRef() != L'\0'))
        {
            std::vector<std::wstring> languages =
                WstringContainerFromDelimitedString<std::vector<std::wstring>>(std::wstring(defaultQualifierValues.GetRef()), L';');
            CUtilities::NormalizeAllLanguageTags(languages);
            _defaultLanguages.insert(_defaultLanguages.end(), languages.begin(), languages.end());
        }
        ++indexPass;
    }

    const auto uniqueEnd = RemoveIfDuplicate(_defaultLanguages.begin(), _defaultLanguages.end());
    _defaultLanguages.erase(uniqueEnd, _defaultLanguages.end());
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ValidateDefaultLanguageQualifiers(IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = _CollectDefaultLanguages(status);
    if (SUCCEEDED(result) && _options.GetShouldValidateDefaultQualifiers())
    {
        std::list<int> unusedQualifiers;
        result = CUtilities::GetListOfUnusedQualifiers(_pDecisionInfoBuilder, &_indexSink, &unusedQualifiers, status);
        std::vector<std::wstring> remainingLanguages(_defaultLanguages);
        Atom languageQualifier;
        if (SUCCEEDED(result))
        {
            if (!Def_HrFailed0(
                    _pUnifiedEnvironment->GetAtom(
                        UnifiedEnvironment::EnvironmentNamesType::QualifierNames, L"Language", &languageQualifier),
                    status))
            {
                StringResult qualifierValue;
                bool foundLanguageQualifier = false;
                int qualifierIndex = 1;
                if (_pDecisionInfoBuilder->GetNumQualifiers() > 1)
                {
                    while (qualifierIndex < _pDecisionInfoBuilder->GetNumQualifiers())
                    {
                        if (!status->Succeeded())
                        {
                            break;
                        }

                        QualifierResult qualifier;
                        bool isUnused = false;
                        if (!unusedQualifiers.empty())
                        {
                            auto unusedQualifier = unusedQualifiers.begin();
                            while ((unusedQualifier != unusedQualifiers.end()) && (*unusedQualifier != qualifierIndex))
                            {
                                ++unusedQualifier;
                            }
                            isUnused = unusedQualifier != unusedQualifiers.end();
                        }
                        if (!isUnused)
                        {
                            Atom qualifierName;
                            if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifier(qualifierIndex, &qualifier), status) &&
                                !Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierName), status) &&
                                (qualifierName.GetPoolIndex() == languageQualifier.GetPoolIndex()) &&
                                (qualifierName.GetIndex() == languageQualifier.GetIndex()))
                            {
                                foundLanguageQualifier = true;
                                if (!Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status))
                                {
                                    _TryRemoveLanguage(remainingLanguages, std::wstring(qualifierValue.GetRef()));
                                }
                            }

                            if (foundLanguageQualifier && remainingLanguages.empty())
                            {
                                break;
                            }
                        }
                        ++qualifierIndex;
                    }
                }

                if (!foundLanguageQualifier || remainingLanguages.empty())
                {
                    result = S_OK;
                }
                else
                {
                    std::wstring languageQualifierValues;
                    std::map<std::wstring, CUtilities::QualifierValues*> qualifierValues;
                    if (SUCCEEDED(
                            CUtilities::GetQualifierStringMap(
                                _pDecisionInfoBuilder, _pAtomPoolGroup, &_indexSink, &qualifierValues, status, true)))
                    {
                        auto qualifierValueEntry = qualifierValues.begin();
                        while (qualifierValueEntry != qualifierValues.end())
                        {
                            CUtilities::QualifierValues* const values = qualifierValueEntry->second;
                            if ((values->qualifierNameAtom.GetPoolIndex() == languageQualifier.GetPoolIndex()) &&
                                (values->qualifierNameAtom.GetIndex() == languageQualifier.GetIndex()))
                            {
                                std::wstring sourceValues(values->wstrValues);
                                if (sourceValues.length() > languageQualifierValues.length())
                                {
                                    languageQualifierValues.append(sourceValues.length() - languageQualifierValues.length(), L'\0');
                                }
                                else
                                {
                                    languageQualifierValues.erase(sourceValues.length(), std::wstring::npos);
                                }
                                std::transform(sourceValues.begin(), sourceValues.end(), languageQualifierValues.begin(), std::towlower);
                                break;
                            }
                            ++qualifierValueEntry;
                        }
                    }

                    const std::wstring missingLanguages = DelimitedStringFromItems(remainingLanguages.begin(), remainingLanguages.end());
                    status->AddWarning(E_DEF_QUALAPPL_MISSING_DEFAULT_LANG, missingLanguages.c_str(), 0, languageQualifierValues.c_str());
                    for (const auto& qualifierValueEntry : qualifierValues)
                    {
                        delete qualifierValueEntry.second;
                    }
                }
            }
        }
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, status));
    return ComputeHResult(result, status);
}

HRESULT CHIndexerBase::_ValidateFullPackage(IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (_options.GetShouldValidateDefaultQualifiers())
    {
        StandalonePriFile* const reader = _pMainPackage->GetReader(status);
        if (reader != nullptr)
        {
            const IDecisionInfo* const decisionInfo = reader->GetDefaultDecisionInfo();
            DecisionResult decision;
            std::vector<int> decisionsWithoutDefaults;
            int decisionIndex = 0;
            if (status->Succeeded())
            {
                while (decisionIndex < decisionInfo->GetNumDecisions())
                {
                    if (!Def_HrFailed0(decisionInfo->GetDecision(decisionIndex, &decision), status) && !decision.GetAlwaysYieldsResult())
                    {
                        decisionsWithoutDefaults.push_back(decisionIndex);
                    }
                    ++decisionIndex;
                    if (!status->Succeeded())
                    {
                        break;
                    }
                }
            }

            NamedResourceResult resource;
            StringResult resourceName;
            int numberOfWarnings = 0;
            int resourceMapIndex = 0;
            if (status->Succeeded())
            {
                while (resourceMapIndex < reader->GetNumResourceMaps())
                {
                    const IResourceMapBase* resourceMap;
                    Def_HrFailed0(reader->GetResourceMap(resourceMapIndex, &resourceMap), status);
                    if (resourceMap != nullptr)
                    {
                        int resourceIndex = 0;
                        if (status->Succeeded())
                        {
                            while ((resourceIndex < resourceMap->GetNumResources()) && (numberOfWarnings < 100))
                            {
                                if (!Def_HrFailed0(resourceMap->GetResourceByIndex(resourceIndex, &resource), status) &&
                                    !Def_HrFailed0(resource.GetDecision(&decision), status))
                                {
                                    int resourceDecisionIndex;
                                    Def_HrFailed0(decision.GetIndex(&resourceDecisionIndex), status);
                                    auto missingDecision = decisionsWithoutDefaults.begin();
                                    while ((missingDecision != decisionsWithoutDefaults.end()) &&
                                           (*missingDecision != resourceDecisionIndex))
                                    {
                                        ++missingDecision;
                                    }
                                    if (missingDecision != decisionsWithoutDefaults.end())
                                    {
                                        Def_HrFailed0(resource.GetResourceName(&resourceName), status);
                                        status->AddWarning(E_MRM_NO_DEFAULT_OR_NEUTRAL_VALUE, resourceName.GetRef());
                                        ++numberOfWarnings;
                                    }
                                }
                                ++resourceIndex;
                                if (!status->Succeeded())
                                {
                                    break;
                                }
                            }
                        }
                    }
                    ++resourceMapIndex;
                    if (!status->Succeeded())
                    {
                        break;
                    }
                }
            }
            delete reader;
        }
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_CreateMetaDataFile(IDefStatusEx* const status)
{

    IXMLDOMDocument2* document = nullptr;
    IXMLDOMNode* rootNode = nullptr;
    IXMLDOMNode* packageNode = nullptr;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = CXMLUtil::CreateXMLDocument(&document);
    bool writeDocument = FAILED(result);
    if (SUCCEEDED(result) && SUCCEEDED(CXMLUtil::AddElement(document, nullptr, L"root", &rootNode)))
    {
        if (_eRpMode == ResourcePackageMode::FatPack)
        {
            _pMainPackage->AddContentsToLogNode(document, rootNode, false, status);
            CXMLUtil::CleanupNode(&rootNode);
            writeDocument = true;
        }
        else if (SUCCEEDED(CXMLUtil::AddElement(document, rootNode, L"package", &packageNode)))
        {
            _pFatPackage->AddContentsToLogNode(document, packageNode, true, status);
            CXMLUtil::CleanupNode(&packageNode);

            bool addPackageFailed = false;
            auto resourcePackage = _resourcePackages.begin();
            if (status->Succeeded())
            {
                while (resourcePackage != _resourcePackages.end())
                {
                    if (FAILED(CXMLUtil::AddElement(document, rootNode, L"package", &packageNode)))
                    {
                        addPackageFailed = true;
                        break;
                    }
                    resourcePackage->second->AddContentsToLogNode(document, packageNode, true, status);
                    CXMLUtil::CleanupNode(&packageNode);
                    ++resourcePackage;
                    if (!status->Succeeded())
                    {
                        break;
                    }
                }
            }
            if (!addPackageFailed)
            {
                CXMLUtil::CleanupNode(&rootNode);
                writeDocument = true;
            }
        }
    }

    if (writeDocument)
    {
        result = CXMLUtil::WriteXmlToFile(document, _metaDataFilePath.GetRef());
    }
    SAFE_RELEASE(document);
    CXMLUtil::CleanupNode(&packageNode);
    CXMLUtil::CleanupNode(&rootNode);
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_AddReverseMapSection(IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CPackageInfo* const package = _eRpMode != ResourcePackageMode::FatPack ? _pFatPackage : _pMainPackage;
    Build::FileBuilder* const fileBuilder = package->_pPriFileBuilder;
    if (fileBuilder != nullptr)
    {
        Build::PriSectionBuilder* const priSectionBuilder = package->_pPriFileBuilder->GetDescriptor();
        if (priSectionBuilder != nullptr)
        {
            Build::ReverseFileMapSectionBuilder* reverseFileMap;
            Def_HrFailed0(
                Build::ReverseFileMapSectionBuilder::CreateInstance(priSectionBuilder, _pUnifiedEnvironment, &reverseFileMap), status);
            if (reverseFileMap != nullptr)
            {
                Microsoft::Resources::DynamicArray<Build::ResourceMapSectionBuilder*> resourceMaps;
                Build::ResourceMapSectionBuilder* resourceMap;
                int resourceMapIndex;
                Def_HrFailed0(priSectionBuilder->GetOrAddPrimaryResourceMapBuilder(&resourceMap), status);
                if (!Def_HrFailed0(resourceMaps.Add(resourceMap, &resourceMapIndex), status) && (_eRpMode != ResourcePackageMode::FatPack))
                {
                    auto resourcePackage = _resourcePackages.begin();
                    if (status->Succeeded())
                    {
                        while (resourcePackage != _resourcePackages.end())
                        {
                            Build::ResourceMapSectionBuilder* packageResourceMap;
                            Def_HrFailed0(
                                resourcePackage->second->_pPriFileBuilder->GetDescriptor()->GetOrAddPrimaryResourceMapBuilder(
                                    &packageResourceMap),
                                status);
                            Def_HrFailed0(resourceMaps.Add(packageResourceMap, &resourceMapIndex), status);
                            ++resourcePackage;
                            if (!status->Succeeded())
                            {
                                break;
                            }
                        }
                    }
                }
                if (status->Succeeded() && !Def_HrFailed0(reverseFileMap->GenerateMap(&resourceMaps), status))
                {
                    Def_HrFailed0(fileBuilder->AddSection(reverseFileMap), status);
                }
            }
        }
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_WriteIndexFiles(
    const CContentChecksumData::NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
    std::uint32_t contentChecksumValue,
    IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    constexpr std::uint32_t DefaultContentChecksum = 0xEDEDEDED;

    bool checksumValueProvided;
    if (contentChecksumValue != 0)
    {
        checksumValueProvided = true;
        status->DiagnosticLogA("Content checksum value initialized with [%u]", contentChecksumValue);
    }
    else
    {
        contentChecksumValue = DefaultContentChecksum;
        checksumValueProvided = false;
        status->DiagnosticLogA(
            "Content checksum value initialized with "
            "default value [%u]",
            contentChecksumValue);
    }

    std::vector<std::wstring>* const defaultLanguages = !_defaultLanguages.empty() ? &_defaultLanguages : nullptr;
    CContentChecksumData* const contentChecksumData = CContentChecksumData::New(
        _options.GetShouldCreateContentChecksum(),
        CContentChecksumData::ContentChecksumOperation::MainPackage,
        neutralLanguageCandidateCreation,
        defaultLanguages,
        contentChecksumValue,
        checksumValueProvided,
        status);

    if (_eSchemaPermission == SchemaPermission::None)
    {
        _pMainPackage->Build(_simpleId.GetRef(), static_cast<std::uint16_t>(_ulMajorVersion), contentChecksumData, status);
    }
    else if (_eSchemaPermission == SchemaPermission::ReadOnly)
    {
        _pMainPackage->Build(_indexName.GetRef(), _pPreviousSchema, contentChecksumData, status);
    }
    else if (_eSchemaPermission == SchemaPermission::Writable)
    {
        _pMainPackage->Build(_pPreviousSchema, contentChecksumData, status);
    }

    if (status->Succeeded() && (_eRpMode != ResourcePackageMode::FatPack))
    {
        StandalonePriFile* const reader = _pMainPackage->GetReader(status);
        if (reader != nullptr)
        {
            const IHierarchicalSchema* schema;
            Def_HrFailed0(reader->GetPrimarySchema(&schema), status);
            if (schema != nullptr)
            {
                contentChecksumData->operation = CContentChecksumData::ContentChecksumOperation::AutoMergePackage;
                if (_pFatPackage->Build(schema, contentChecksumData, status))
                {
                    contentChecksumData->operation = CContentChecksumData::ContentChecksumOperation::ResourcePackage;
                    auto resourcePackage = _resourcePackages.begin();
                    if (status->Succeeded())
                    {
                        while (resourcePackage != _resourcePackages.end())
                        {
                            resourcePackage->second->Build(schema, contentChecksumData, status);
                            ++resourcePackage;
                            if (!status->Succeeded())
                            {
                                break;
                            }
                        }
                    }
                }
            }
            delete reader;
        }
    }

    HRESULT result = ComputeHResult(S_OK, status);
    if (SUCCEEDED(result))
    {
        if (_options.GetShouldGenerateReverseMap())
        {
            result = _AddReverseMapSection(status);
        }
        if (SUCCEEDED(result) && !_unknown194)
        {
            const wchar_t* const outputDirectory = _outputFolder.GetRef();
            if (_eRpMode != ResourcePackageMode::FatPack)
            {
                if (_pFatPackage->WriteToFile(outputDirectory, status))
                {
                    auto resourcePackage = _resourcePackages.begin();
                    if (status->Succeeded())
                    {
                        while (resourcePackage != _resourcePackages.end())
                        {
                            resourcePackage->second->WriteToFile(_outputFolder.GetRef(), status);
                            ++resourcePackage;
                            if (!status->Succeeded())
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                _pMainPackage->WriteToFile(outputDirectory, status);
            }
        }
    }

    if (status->Succeeded())
    {
        _WriteStatusToStream(status);
    }
    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    delete contentChecksumData;
    return result;
}

HRESULT CHIndexerBase::_WriteStatusToStream(IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    StandalonePriFile* const reader = _pMainPackage->GetReader(status);
    if (reader != nullptr)
    {
        if (!_options.GetShouldSuppressConsoleOutput() && (reader->GetNumResourceMaps() > 0))
        {
            const IResourceMapBase* firstResourceMap;
            Def_HrFailed0(reader->GetResourceMap(0, &firstResourceMap), status);
            const IHierarchicalSchema* const schema = firstResourceMap->GetOriginalSchema();
            const std::uint16_t majorVersion = schema->GetMajorVersion();
            const std::uint16_t minorVersion = schema->GetMinorVersion();
            const wchar_t* const resourceMapName = schema->GetSimpleId();

            int numberOfNamedResources = 0;
            for (int resourceMapIndex = 0; resourceMapIndex < reader->GetNumResourceMaps(); ++resourceMapIndex)
            {
                const IResourceMapBase* resourceMap;
                Def_HrFailed0(reader->GetResourceMap(resourceMapIndex, &resourceMap), status);
                numberOfNamedResources += resourceMap->GetNumResources();
            }

            std::uint32_t numberOfCandidates = 0;
            for (int resourceMapIndex = 0; resourceMapIndex < reader->GetNumResourceMaps(); ++resourceMapIndex)
            {
                const IResourceMapBase* resourceMap;
                Def_HrFailed0(reader->GetResourceMap(resourceMapIndex, &resourceMap), status);
                numberOfCandidates += static_cast<std::uint32_t>(resourceMap->GetTotalNumResourceValues());
            }

            HRESULT result = LogInfo(L"Finished building");
            if (FAILED(result))
            {
                // Original line: 3216
                RETURN_HR(result);
            }
            result = LogInfo(L"Version: %d.%d", majorVersion, minorVersion);
            if (FAILED(result))
            {
                // Original line: 3217
                RETURN_HR(result);
            }
            result = LogInfo(L"Resource Map Name: %s", resourceMapName);
            if (FAILED(result))
            {
                // Original line: 3218
                RETURN_HR(result);
            }
            result = LogInfo(L"Named Resources: %d", numberOfNamedResources);
            if (FAILED(result))
            {
                // Original line: 3219
                RETURN_HR(result);
            }

            if (_eRpMode == ResourcePackageMode::FatPack)
            {
                result = LogInfo(L"Resource Candidates: %d", numberOfCandidates);
                if (FAILED(result))
                {
                    // Original line: 3223
                    RETURN_HR(result);
                }
            }
            else
            {
                result = LogInfo(L"");
                if (FAILED(result))
                {
                    // Original line: 3227
                    RETURN_HR(result);
                }
                if (_pFatPackage->WriteStatusToStream(status))
                {
                    result = LogInfo(L"");
                    if (FAILED(result))
                    {
                        // Original line: 3230
                        RETURN_HR(result);
                    }

                    auto resourcePackage = _resourcePackages.begin();
                    if (status->Succeeded())
                    {
                        while (resourcePackage != _resourcePackages.end())
                        {
                            result = LogInfo(L"");
                            if (FAILED(result))
                            {
                                // Original line: 3233
                                RETURN_HR(result);
                            }
                            if (_options.GetShouldOmitSchemaFromResourcePacks())
                            {
                                resourcePackage->second->_overrideSchemaFile = reader;
                            }
                            resourcePackage->second->WriteStatusToStream(status);
                            ++resourcePackage;
                            if (!status->Succeeded())
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
        delete reader;
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ParseIndexNodes(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(root);
    IXMLDOMNodeList* indexNodes;
    HRESULT result = xml.TryGetChildren(L"index", status, &indexNodes);
    if (SUCCEEDED(result))
    {
        LONG numberOfIndexNodes = 0;
        result = indexNodes->get_length(&numberOfIndexNodes);
        if (SUCCEEDED(result))
        {
            for (LONG index = 0; index < numberOfIndexNodes; ++index)
            {
                IXMLDOMNode* indexNode;
                result = indexNodes->get_item(index, &indexNode);
                if (SUCCEEDED(result))
                {
                    CIndexPass* indexPass = nullptr;
                    result = CIndexPass::New(
                        indexNode,
                        _pMrmProfile,
                        _pUnifiedEnvironment,
                        _projectRootFolder.GetRef(),
                        _pDecisionInfoBuilder,
                        &_options,
                        &_logItems,
                        status,
                        &indexPass);
                    if (SUCCEEDED(result) && (indexPass != nullptr))
                    {
                        _indexPassList.push_back(indexPass);
                    }
                    SAFE_RELEASE(indexNode);
                }
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(indexNodes));
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_InitializeQualifierNameTokenMap(IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    const IEnvironment* environment;
    Def_HrFailed0(_pUnifiedEnvironment->GetEnvironment(0, &environment), status);
    if (status->Succeeded())
    {
        const QUALIFIER_INFO* const qualifierInfo = environment->GetQualifierInfo();
        for (int index = 0; index < qualifierInfo->numQualifiers; ++index)
        {
            std::wstring qualifierName(qualifierInfo->pQualifierNames[index]);
            std::wstring qualifierToken(qualifierInfo->pQualifiers[index].pQualifierToken);
            std::transform(qualifierName.begin(), qualifierName.end(), qualifierName.begin(), std::towlower);
            _qualifierNameTokenMap.insert(std::make_pair(qualifierName, qualifierToken));
        }
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_ValidateConfigurationSchema(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(root);
    const HRESULT result = xml.ValidateAgainstSchema(ConfigurationSchema, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

bool CHIndexerBase::SetOmitSchemaFromResourcePacks(IDefStatusEx* const status)
{
    if (IsPlatformAtLeastTH1(_platformVersion))
    {
        _options.m_shouldOmitSchemaFromResourcePacks = true;
        _pMrmProfileForResourcePackGeneration->GetBuildConfiguration()->SetFlags(
            MrmBuildConfiguration::UniversalConfigurationFlagsWithSchemaReference);
        return true;
    }

    status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, L"omitSchemaFromResourcePacks");
    return false;
}

bool CHIndexerBase::SetSplitLanguageVariants(IDefStatusEx* const status)
{
    if ((_platformVersion == MrmPlatformVersionInternal::WindowsClient8) ||
        (_platformVersion == MrmPlatformVersionInternal::WindowsClientBlue) ||
        (_platformVersion == MrmPlatformVersionInternal::WindowsPhoneBlue) ||
        (_platformVersion == MrmPlatformVersionInternal::WindowsCore) ||
        (_platformVersion == MrmPlatformVersionInternal::TestIncompatiblePlatformVersion))
    {
        status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, L"splitLanguageVariants");
        return false;
    }

    _options.m_shouldSplitLanguageVariants = true;
    _pMrmProfile->GetBuildConfiguration()->SetFlags(_pMrmProfile->GetBuildConfiguration()->GetFlags() | 0x200);
    _pMrmProfileForResourcePackGeneration->GetBuildConfiguration()->SetFlags(
        _pMrmProfileForResourcePackGeneration->GetBuildConfiguration()->GetFlags() | 0x200);
    return true;
}

HRESULT CHIndexerBase::_InitializeEnvironment(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = _SetPlatformVersionAndProfile(root, status);
    if (SUCCEEDED(result))
    {
        Def_HrFailed0(AtomPoolGroup::CreateInstance(0, &_pAtomPoolGroup), status);
        Def_HrFailed0(UnifiedEnvironment::CreateInstance(_pMrmProfile, _pAtomPoolGroup, &_pUnifiedEnvironment), status);
        Def_HrFailed0(Build::DecisionInfoBuilder::CreateInstance(0, _pUnifiedEnvironment, nullptr, &_pDecisionInfoBuilder), status);
        result = ComputeHResult(result, status);
        if (SUCCEEDED(result))
        {
            result = _SetMajorVersionNumber(root, status);
            if (SUCCEEDED(result))
            {
                result = _SetDeploymentMergeableFlag(root, status);
            }
        }
    }

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_SetPlatformVersionAndProfile(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(root);
    wchar_t* targetPlatformValue = nullptr;
    wchar_t* targetOsVersion = nullptr;
    std::wstring targetPlatform;
    HRESULT result = xml.GetAttributeValue(L"targetPlatform", status, &targetPlatformValue);
    if (status->GetWhat() == E_DEF_XML_ATTRIB_NOT_FOUND)
    {
        targetPlatform.assign(L"client", 6);
        status->Reset();
    }
    else
    {
        targetPlatform.assign(targetPlatformValue);
        if (FAILED(result))
        {
            targetPlatformValue = nullptr;
            status->Reset();
            result = S_OK;
            goto SelectProfiles;
        }
    }

    result = xml.GetAttributeValue(L"targetOsVersion", status, &targetOsVersion);
    if (FAILED(result) || (targetOsVersion == nullptr))
    {
        targetPlatformValue = nullptr;
        status->Reset();
        result = S_OK;
    }
    else if (!CUtilities::GetVersionFromString(targetOsVersion, targetPlatform.c_str(), &_platformVersion))
    {
        status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, targetOsVersion);
    }

SelectProfiles:
    result = ComputeHResult(result, status);
    if (SUCCEEDED(result))
    {
        Def_HrFailed0(
            MrmProfile::ChooseDefaultProfile(
                MrmProfile::ProfileType::EmptyInit, _platformVersion, nullptr, nullptr, nullptr, &_pMrmProfileForResourcePackGeneration),
            status);
        if (status->Failed() && (status->GetWhat() == E_MRM_UNSUPPORTED_PLATFORM))
        {
            status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, targetOsVersion);
        }
    }

    result = ComputeHResult(result, status);
    if (SUCCEEDED(result))
    {
        Def_HrFailed0(
            MrmProfile::ChooseDefaultProfile(
                MrmProfile::ProfileType::EmptyInit, _platformVersion, nullptr, nullptr, nullptr, &_pMrmProfile),
            status);
        if (status->Failed() && (status->GetWhat() == E_MRM_UNSUPPORTED_PLATFORM))
        {
            status->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, targetOsVersion);
        }
    }

    result = ComputeHResult(result, status);
    if (SUCCEEDED(result))
    {
        _options.m_shouldUseOptimizedEncoding = (_platformVersion != MrmPlatformVersionInternal::WindowsClient8) &&
                                                (_platformVersion != MrmPlatformVersionInternal::WindowsClientBlue) &&
                                                (_platformVersion != MrmPlatformVersionInternal::WindowsPhoneBlue);
    }
    if (targetOsVersion != nullptr)
    {
        delete[] targetOsVersion;
    }
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_SetMajorVersionNumber(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper xml(root);
    wchar_t* majorVersion = nullptr;
    HRESULT result = xml.GetAttributeValue(L"majorVersion", status, &majorVersion);
    if (SUCCEEDED(result) && (majorVersion != nullptr))
    {
        _ulMajorVersion = static_cast<ULONG>(_wtoi(majorVersion));
    }
    else
    {
        result = S_OK;
        status->Reset();
    }
    delete[] majorVersion;

    result = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CHIndexerBase::_SetDeploymentMergeableFlag(IXMLDOMNode* const root, IDefStatusEx* const status)
{

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    bool hasDeploymentMergeableAttribute = false;
    CXmlHelper xml(root);
    wchar_t* deploymentMergeable = nullptr;
    if (xml.GetAttributeValue(L"isDeploymentMergeable", status, &deploymentMergeable) != S_OK)
    {
        status->Reset();
    }
    else
    {
        hasDeploymentMergeableAttribute = true;
        if (DefString_CompareWithOptions(deploymentMergeable, L"false", DefCompare_CaseInsensitive) == 0)
        {
            _options.m_shouldBuildDeploymentMergeablePri = false;
        }
    }
    delete deploymentMergeable;

    if (hasDeploymentMergeableAttribute)
    {
        if (_platformVersion == MrmPlatformVersionInternal::WindowsClient8)
        {
            status->SetError(E_DEF_PRICONFIG_INVALID_IDM_WIN8, L"");
        }
        if (_options.GetShouldBuildAutoMergePri() && _options.GetShouldBuildDeploymentMergeablePri())
        {
            status->SetError(E_DEF_PRICONFIG_INVALID_IDM_AM, L"");
        }
    }
    else if ((_platformVersion == MrmPlatformVersionInternal::WindowsClient8) || _options.GetShouldBuildAutoMergePri())
    {
        _options.m_shouldBuildDeploymentMergeablePri = false;
    }

    const HRESULT result = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

bool CHIndexerBase::_IsArpQualifier(const Atom qualifier)
{
    for (const Atom arpQualifier : _arpQualifierList)
    {
        if ((arpQualifier.GetPoolIndex() == qualifier.GetPoolIndex()) && (arpQualifier.GetIndex() == qualifier.GetIndex()))
        {
            return true;
        }
    }
    return false;
}

bool CHIndexerBase::_HasArpQualifier(QualifierSetResult qualifierSet, IDefStatusEx* const status)
{
    int qualifierIndex = 0;
    if (qualifierSet.GetNumQualifiers() <= 0)
    {
        return false;
    }

    while (true)
    {
        QualifierResult qualifier;
        if (!Def_HrFailed0(qualifierSet.GetQualifier(qualifierIndex, &qualifier, nullptr), status))
        {
            Atom qualifierAtom {};
            if (!Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierAtom), status) && _IsArpQualifier(qualifierAtom))
            {
                return true;
            }
        }
        ++qualifierIndex;
        if (qualifierIndex >= qualifierSet.GetNumQualifiers())
        {
            return false;
        }
    }
}

void CHIndexerBase::_TryRemoveLanguage(std::vector<std::wstring>& languages, const std::wstring& language)
{
    const std::wstring normalizedLanguage = CUtilities::NormalizeLanguageTag(language);
    languages.erase(std::remove(languages.begin(), languages.end(), normalizedLanguage), languages.end());
}

HRESULT CHIndexerBase::LogInfo(const wchar_t* const format, ...)
{
    va_list arguments;
    va_start(arguments, format);

    HRESULT result = S_OK;
    const std::size_t bufferCount = static_cast<std::size_t>(_vscwprintf_l(format, nullptr, arguments) + 1);
    wchar_t* const buffer = new (std::nothrow) wchar_t[bufferCount]();
    RETURN_LAST_ERROR_IF_NULL(buffer);
    vswprintf_s(buffer, bufferCount, format, arguments);
    _logItems.push_back(LogItem(MrmResourceIndexerMessageSeverityInfo, 0, std::wstring(buffer)));
    delete[] buffer;
    return result;
}

HRESULT CHIndexerBase::LogInfo(std::vector<LogItem>* const logItems, const wchar_t* const format, ...)
{
    va_list arguments;
    va_start(arguments, format);

    HRESULT result = S_OK;
    const std::size_t bufferCount = static_cast<std::size_t>(_vscwprintf_l(format, nullptr, arguments) + 1);
    wchar_t* const buffer = new (std::nothrow) wchar_t[bufferCount]();
    RETURN_LAST_ERROR_IF_NULL(buffer);
    vswprintf_s(buffer, bufferCount, format, arguments);
    logItems->push_back(LogItem(MrmResourceIndexerMessageSeverityInfo, 0, std::wstring(buffer)));
    delete[] buffer;
    return result;
}

IbcOptions::IbcOptions() :
    m_shouldProcessHiddenFiles(false),
    m_shouldProcessLinkedFiles(false),
    m_shouldBuildAutoMergePri(false),
    m_shouldBuildDeploymentMergeablePri(true),
    m_shouldValidateDefaultQualifiers(true),
    m_shouldSuppressConsoleOutput(false),
    m_shouldSuppressEmbeddedData(true),
    m_shouldGenerateReverseMap(false),
    m_shouldUseOptimizedEncoding(true),
    m_shouldOmitSchemaFromResourcePacks(true),
    m_indexerSchemaCollection(nullptr),
    m_shouldDisableDeduplication(false),
    m_shouldUseGranularResourceSplitting(false),
    m_shouldSplitLanguageVariants(false),
    m_shouldCreateContentChecksum(false)
{}

IbcOptions::~IbcOptions() = default;

bool IbcOptions::GetShouldProcessHiddenFiles() const { return m_shouldProcessHiddenFiles; }

bool IbcOptions::GetShouldProcessLinkedFiles() const { return m_shouldProcessLinkedFiles; }

bool IbcOptions::GetShouldSuppressEmbeddedData() const { return m_shouldSuppressEmbeddedData; }

bool IbcOptions::GetShouldUseOptimizedEncoding() const { return m_shouldUseOptimizedEncoding; }

bool IbcOptions::GetShouldSplitLanguageVariants() const { return m_shouldSplitLanguageVariants; }

const wchar_t* IbcOptions::GetIndexerSchemaPath() const { return m_indexerSchemaPath.GetRef(); }

const ISchemaCollection* IbcOptions::GetIndexerSchemaCollection(IDefStatusEx* const status) const
{
    static_cast<void>(status);
    return m_indexerSchemaCollection;
}

bool IbcOptions::GetShouldDisableDeduplication() const { return m_shouldDisableDeduplication; }

bool IbcOptions::GetShouldSuppressConsoleOutput() const { return m_shouldSuppressConsoleOutput; }

bool IbcOptions::GetShouldOmitSchemaFromResourcePacks() const { return m_shouldOmitSchemaFromResourcePacks; }

bool IbcOptions::GetShouldValidateDefaultQualifiers() const { return m_shouldValidateDefaultQualifiers; }

bool IbcOptions::GetShouldGenerateReverseMap() const { return m_shouldGenerateReverseMap; }

bool IbcOptions::GetShouldCreateContentChecksum() const { return m_shouldCreateContentChecksum; }

bool IbcOptions::GetShouldBuildDeploymentMergeablePri() const { return m_shouldBuildDeploymentMergeablePri; }

bool IbcOptions::GetShouldBuildAutoMergePri() const { return m_shouldBuildAutoMergePri; }

bool IbcOptions::GetShouldUseGranularResourceSplitting() const { return m_shouldUseGranularResourceSplitting; }

void IbcOptions::Set(const IIndexOptions* const options)
{
    DefStatusEx status;
    m_shouldProcessHiddenFiles = options->GetShouldProcessHiddenFiles();
    m_shouldProcessLinkedFiles = options->GetShouldProcessLinkedFiles();
    m_shouldBuildAutoMergePri = options->GetShouldBuildAutoMergePri();
    m_shouldBuildDeploymentMergeablePri = options->GetShouldBuildDeploymentMergeablePri();
    m_shouldValidateDefaultQualifiers = options->GetShouldValidateDefaultQualifiers();
    m_shouldSuppressConsoleOutput = options->GetShouldSuppressConsoleOutput();
    m_shouldSuppressEmbeddedData = options->GetShouldSuppressEmbeddedData();
    m_shouldGenerateReverseMap = options->GetShouldGenerateReverseMap();
    m_shouldUseOptimizedEncoding = options->GetShouldUseOptimizedEncoding();
    m_shouldOmitSchemaFromResourcePacks = options->GetShouldOmitSchemaFromResourcePacks();
    DefStringResult_SetCopy(m_indexerSchemaPath.GetStringResult(), options->GetIndexerSchemaPath());
    m_indexerSchemaCollection = options->GetIndexerSchemaCollection(&status);
    m_shouldDisableDeduplication = options->GetShouldDisableDeduplication();
    m_shouldUseGranularResourceSplitting = options->GetShouldUseGranularResourceSplitting();
    m_shouldSplitLanguageVariants = options->GetShouldSplitLanguageVariants();
    m_shouldCreateContentChecksum = options->GetShouldCreateContentChecksum();
}

} // namespace Microsoft::Resources::Indexers
