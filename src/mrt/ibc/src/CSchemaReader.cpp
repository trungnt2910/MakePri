#include "StdAfx.h"

#include <SchemaReader.h>

namespace Microsoft::Resources::Indexers
{
const wchar_t* CPriSchemaReader::s_pszPriInfoNodeName = L"PriInfo";
const wchar_t* CPriSchemaReader::s_pszResourceMapNodeName = L"ResourceMap";
const wchar_t* CPriSchemaReader::s_pszResourceMapNameAttribute = L"name";
const wchar_t* CPriSchemaReader::s_pszResourceMapUniqueNameAttribute = L"uniqueName";
const wchar_t* CPriSchemaReader::s_pszResourceMapVersionAttribute = L"version";
const wchar_t* CPriSchemaReader::s_pszSubtreeNodeName = L"ResourceMapSubtree";
const wchar_t* CPriSchemaReader::s_pszSubtreeNameAttribute = L"name";
const wchar_t* CPriSchemaReader::s_pszSubtreeIndexAttribute = L"index";
const wchar_t* CPriSchemaReader::s_pszResourceNodeName = L"NamedResource";
const wchar_t* CPriSchemaReader::s_pszResourceNameAttribute = L"name";
const wchar_t* CPriSchemaReader::s_pszResourceIndexAttribute = L"index";

CPriSchemaReader::CPriSchemaReader() = default;

CPriSchemaReader::~CPriSchemaReader()
{
    if (_pSubtrees != nullptr)
    {
        wchar_t** const names = _pSubtrees->GetData();
        if (names != nullptr && _pSubtrees->GetCount() != 0)
        {
            for (std::uint32_t index = 0; index < _pSubtrees->GetCount(); ++index)
            {
                if (names[index] != nullptr)
                {
                    HeapFree(GetProcessHeap(), 0, names[index]);
                }
            }
        }
        delete _pSubtrees;
        _pSubtrees = nullptr;
    }

    if (_pResources != nullptr)
    {
        wchar_t** const names = _pResources->GetData();
        if (names != nullptr && _pResources->GetCount() != 0)
        {
            for (std::uint32_t index = 0; index < _pResources->GetCount(); ++index)
            {
                if (names[index] != nullptr)
                {
                    HeapFree(GetProcessHeap(), 0, names[index]);
                }
            }
        }
        delete _pResources;
        _pResources = nullptr;
    }

    if (_pSchema != nullptr)
    {
        delete _pSchema;
    }
    if (_pSchemaBlob != nullptr)
    {
        HeapFree(GetProcessHeap(), 0, _pSchemaBlob);
    }
}

HRESULT CPriSchemaReader::InitializeFromFile(const wchar_t* const path, IDefStatusEx* const status)
{
    if (_pSubtrees != nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_ALREADY_INITIALIZED, L"" __FILE__, 93, L"", 0);
        }
        return status->GetHResult();
    }

    Def_HrFailed0(DynamicArray<wchar_t*>::CreateInstance(16, &_pSubtrees), status);
    if (status->Succeeded())
    {
        Def_HrFailed0(DynamicArray<wchar_t*>::CreateInstance(128, &_pResources), status);
    }
    if (_pSubtrees == nullptr || _pResources == nullptr)
    {
        return status->GetHResult();
    }

    const wchar_t* accessiblePath = nullptr;
    RETURN_IF_FAILED(CUtilities::GetPathInAccessibleFormat(path, &accessiblePath));
    const std::unique_ptr<const wchar_t> accessiblePathOwner(accessiblePath);

    if (!PathFileExistsW(accessiblePath))
    {
        status->SetError(E_DEF_FILE_NOT_FOUND, accessiblePath);
        return status->GetHResult();
    }

    CXmlHelper helper;
    RETURN_IF_FAILED(helper.Init(accessiblePath, CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_FILE_PATH, s_pszPriInfoNodeName, status));
    RETURN_IF_FAILED(ParsePriSchemaFile(accessiblePath, CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_FILE_PATH, status));
    return BuildSchema(status);
}

HRESULT CPriSchemaReader::ParsePriSchemaFile(
    const wchar_t* const input,
    const CXmlHelper::INPUT_XML_STR_TYPE inputType,
    IDefStatusEx* const status)
{
    static_cast<void>(inputType);

    IXMLDOMNode* resourceMap = nullptr;
    CXmlHelper helper;
    HRESULT result = helper.Init(input, CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_FILE_PATH, s_pszPriInfoNodeName, status);
    if (SUCCEEDED(result))
    {
        result = helper.TryGetChildNode(s_pszResourceMapNodeName, status, &resourceMap);
        if (SUCCEEDED(result) && resourceMap != nullptr)
        {
            result = ParseResourceMapNode(resourceMap, status);
            SAFE_RELEASE(resourceMap);
        }
    }
    return result;
}

HRESULT CPriSchemaReader::ParseVersionString(const wchar_t* const version, IDefStatusEx* const status)
{
    StringResult versionResult;
    if (!Def_HrFailed0(DefStringResult_SetCopy(versionResult.GetStringResult(), version), status))
    {
        bool invalid = false;
        wchar_t* value = nullptr;
        std::size_t capacity = 0;
        Def_HrFailed0(versionResult.GetWritableRef(&value, &capacity), status);
        if (value != nullptr)
        {
            wchar_t* separator = nullptr;
            wchar_t* current = value;
            if (*current != L'\0')
            {
                while (!invalid)
                {
                    if (*current == L'.')
                    {
                        invalid = separator != nullptr;
                        if (separator == nullptr)
                        {
                            separator = current;
                        }
                    }
                    else if (iswdigit(*current) == 0)
                    {
                        invalid = true;
                    }

                    ++current;
                    if (*current == L'\0')
                    {
                        break;
                    }
                }

                if (!invalid && separator != nullptr && separator != value && separator[1] != L'\0')
                {
                    if (!status->Succeeded())
                    {
                        return status->GetHResult();
                    }

                    *separator = L'\0';
                    const int majorVersion = _wtoi(value);
                    const int minorVersion = _wtoi(separator + 1);
                    if (majorVersion >= 0x10000)
                    {
                        _majorVersion = 0xFFFF;
                    }
                    else
                    {
                        _majorVersion = static_cast<std::uint16_t>(majorVersion);
                        if (minorVersion < 0x10000)
                        {
                            _minorVersion = static_cast<std::uint16_t>(minorVersion);
                        }
                        else
                        {
                            _minorVersion = 0xFFFF;
                        }
                    }
                    return status->GetHResult();
                }
            }

            if (status != nullptr)
            {
                status->SetError(E_DEF_BAD_VALUE, L"" __FILE__, 292, L"", 0);
            }
        }
    }
    return status->GetHResult();
}

HRESULT CPriSchemaReader::ParseResourceMapNode(IXMLDOMNode* const node, IDefStatusEx* const status)
{
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    wchar_t* simpleName = nullptr;
    wchar_t* uniqueName = nullptr;
    wchar_t* version = nullptr;
    CXmlHelper helper(node);

    HRESULT result = helper.GetAttributeValue(s_pszResourceMapNameAttribute, status, &simpleName);
    if (SUCCEEDED(result) && !Def_HrFailed0(DefStringResult_SetCopy(_simpleName.GetStringResult(), simpleName), status))
    {
        result = helper.GetAttributeValue(s_pszResourceMapUniqueNameAttribute, status, &uniqueName);
        if (SUCCEEDED(result) && !Def_HrFailed0(DefStringResult_SetCopy(_uniqueName.GetStringResult(), uniqueName), status))
        {
            result = helper.GetAttributeValue(s_pszResourceMapVersionAttribute, status, &version);
            if (SUCCEEDED(result) && SUCCEEDED(ParseVersionString(version, status)))
            {
                result = helper.TryGetChildren(s_pszSubtreeNodeName, status, &children);
                if (SUCCEEDED(result) && children != nullptr)
                {
                    while (children->nextNode(&child) == S_OK && child != nullptr)
                    {
                        result = ParseResourceMapSubtreeNode(nullptr, child, status);
                        SAFE_RELEASE(child);
                    }
                    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
                }
            }
        }
    }

    operator delete(simpleName);
    operator delete(uniqueName);
    operator delete(version);
    if (SUCCEEDED(result))
    {
        result = status->GetHResult();
    }
    return result;
}

HRESULT CPriSchemaReader::ParseResourceMapSubtreeNode(const wchar_t* const parentName, IXMLDOMNode* const node, IDefStatusEx* const status)
{
    StringResult fullName;
    HRESULT result = fullName.SetRef(parentName);
    if (Def_HrFailed0(result, status))
    {
        return status->GetHResult();
    }

    CXmlHelper helper(node);
    wchar_t* name = nullptr;
    result = helper.GetAttributeValue(s_pszSubtreeNameAttribute, status, &name);
    if (FAILED(result))
    {
        operator delete(name);
        return result;
    }

    if (Def_HrFailed0(DefStringResult_ConcatPathElement(fullName.GetStringResult(), name, L'/'), status))
    {
        operator delete(name);
        return status->GetHResult();
    }

    std::uint32_t scopeIndex = static_cast<std::uint32_t>(-1);
    _variant_t index;
    result = helper.GetAttributeValueAsVariant(s_pszSubtreeIndexAttribute, &index);
    if (FAILED(result))
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_XML_ATTRIB_NOT_FOUND, L"" __FILE__, 383, L"", 0);
        }
        result = status->GetHResult();
    }
    else
    {
        scopeIndex = static_cast<std::uint32_t>(static_cast<LONG>(index));
    }

    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    if (SUCCEEDED(result))
    {
        result = helper.TryGetChildren(s_pszSubtreeNodeName, status, &children);
        if (SUCCEEDED(result) && children != nullptr)
        {
            while (children->nextNode(&child) == S_OK && child != nullptr)
            {
                result = ParseResourceMapSubtreeNode(fullName.GetRef(), child, status);
                SAFE_RELEASE(child);
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        }
    }

    if (SUCCEEDED(result))
    {
        result = helper.TryGetChildren(s_pszResourceNodeName, status, &children);
        if (SUCCEEDED(result) && children != nullptr)
        {
            while (children->nextNode(&child) == S_OK && child != nullptr)
            {
                result = ParseNamedResourceNode(fullName.GetRef(), child, status);
                SAFE_RELEASE(child);
                if (FAILED(result))
                {
                    break;
                }
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        }
    }

    if (SUCCEEDED(result))
    {
        wchar_t* contents = nullptr;
        std::size_t capacity = 0;
        if (Def_HrFailed0(fullName.ReleaseContents(&contents, &capacity), status))
        {
            Def_HrFailed0(DefString_Dup(fullName.GetRef(), &contents), status);
        }

        if (contents != nullptr)
        {
            wchar_t* previous = nullptr;
            HRESULT setResult = S_OK;
            if (scopeIndex >= _pSubtrees->m_count)
            {
                setResult = _pSubtrees->SetExtent(scopeIndex + 1);
            }
            if (SUCCEEDED(setResult))
            {
                previous = _pSubtrees->m_data[scopeIndex];
                _pSubtrees->m_data[scopeIndex] = contents;
            }

            if (Def_HrFailed0(setResult, status))
            {
                HeapFree(GetProcessHeap(), 0, contents);
            }
            else if (previous != nullptr)
            {
                HeapFree(GetProcessHeap(), 0, previous);
                if (status != nullptr)
                {
                    status->SetError(E_DEF_DUPLICATE_INVALID_ENTRY, L"" __FILE__, 439, L"", 0);
                }
            }
        }
    }

    operator delete(name);
    if (SUCCEEDED(result))
    {
        result = status->GetHResult();
    }
    return result;
}

HRESULT CPriSchemaReader::ParseNamedResourceNode(const wchar_t* const parentName, IXMLDOMNode* const node, IDefStatusEx* const status)
{
    StringResult fullName;
    if (Def_HrFailed0(fullName.SetRef(parentName), status))
    {
        return status->GetHResult();
    }

    CXmlHelper helper(node);
    wchar_t* name = nullptr;
    const HRESULT nameResult = helper.GetAttributeValue(s_pszResourceNameAttribute, status, &name);
    if (SUCCEEDED(nameResult) && name != nullptr)
    {
        Def_HrFailed0(DefStringResult_ConcatPathElement(fullName.GetStringResult(), name, L'/'), status);
    }
    operator delete(name);

    std::uint32_t itemIndex = static_cast<std::uint32_t>(-1);
    _variant_t index;
    HRESULT result = helper.GetAttributeValueAsVariant(s_pszResourceIndexAttribute, &index);
    if (FAILED(result))
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_XML_ATTRIB_NOT_FOUND, L"" __FILE__, 492, L"", 0);
        }
        result = status->GetHResult();
    }
    else
    {
        itemIndex = static_cast<std::uint32_t>(static_cast<LONG>(index));
    }

    if (SUCCEEDED(result))
    {
        wchar_t* contents = nullptr;
        std::size_t capacity = 0;
        if (Def_HrFailed0(fullName.ReleaseContents(&contents, &capacity), status))
        {
            Def_HrFailed0(DefString_Dup(fullName.GetRef(), &contents), status);
        }

        if (contents != nullptr)
        {
            wchar_t* previous = nullptr;
            HRESULT setResult = S_OK;
            if (itemIndex >= _pResources->m_count)
            {
                setResult = _pResources->SetExtent(itemIndex + 1);
            }
            if (SUCCEEDED(setResult))
            {
                previous = _pResources->m_data[itemIndex];
                _pResources->m_data[itemIndex] = contents;
            }

            if (Def_HrFailed0(setResult, status))
            {
                HeapFree(GetProcessHeap(), 0, contents);
            }
            else if (previous != nullptr)
            {
                HeapFree(GetProcessHeap(), 0, previous);
                if (status != nullptr)
                {
                    status->SetError(E_DEF_DUPLICATE_INVALID_ENTRY, L"" __FILE__, 518, L"", 0);
                }
            }
        }
        result = status->GetHResult();
    }
    return result;
}

HRESULT CPriSchemaReader::ValidateSchemaData(IDefStatusEx* const status)
{
    if (_pSubtrees == nullptr || _pResources == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_NOT_READY, L"" __FILE__, 536, L"", 0);
        }
    }
    else if (_pSchema != nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_ALREADY_INITIALIZED, L"" __FILE__, 542, L"", 0);
        }
    }
    else
    {
        for (std::uint32_t index = 1; index < _pSubtrees->GetCount(); ++index)
        {
            wchar_t* name = nullptr;
            Def_HrFailed0(_pSubtrees->Get(index, &name), status);
            if (name == nullptr)
            {
                if (status != nullptr)
                {
                    status->SetError(E_DEF_INCOMPLETE_SCHEMA, L"" __FILE__, 553, L"", 0);
                }
                break;
            }
        }

        if (status->Succeeded())
        {
            for (std::uint32_t index = 0; index < _pResources->GetCount(); ++index)
            {
                wchar_t* name = nullptr;
                Def_HrFailed0(_pResources->Get(index, &name), status);
                if (name == nullptr)
                {
                    if (status != nullptr)
                    {
                        status->SetError(E_DEF_INCOMPLETE_SCHEMA, L"" __FILE__, 566, L"", 0);
                    }
                    break;
                }
            }
        }
    }
    return status->GetHResult();
}

HRESULT CPriSchemaReader::BuildSchema(IDefStatusEx* const status)
{
    HRESULT result = ValidateSchemaData(status);
    if (FAILED(result))
    {
        return result;
    }

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
    if (profile.Data() != nullptr)
    {
        Build::PriFileBuilder* fileBuilderValue = nullptr;
        Def_HrFailed0(Build::PriFileBuilder::CreateInstance(profile.Data(), &fileBuilderValue), status);
        AutoDeletePtr<Build::PriFileBuilder> fileBuilder(fileBuilderValue);
        if (fileBuilder.Data() != nullptr)
        {
            Build::PriSectionBuilder* priSectionBuilderValue = nullptr;
            Def_HrFailed0(Build::PriSectionBuilder::CreateInstance(fileBuilder.Data(), profile.Data(), &priSectionBuilderValue), status);
            AutoDeletePtr<Build::PriSectionBuilder> priSectionBuilder(priSectionBuilderValue);
            if (priSectionBuilder.Data() != nullptr)
            {
                StaticHierarchicalSchemaDescription* descriptionValue = nullptr;
                Def_HrFailed0(
                    StaticHierarchicalSchemaDescription::CreateInstance(
                        StaticHierarchicalSchemaDescription::CreateByReference,
                        _simpleName.GetRef(),
                        _uniqueName.GetRef(),
                        _majorVersion,
                        _minorVersion,
                        _pSubtrees->GetCount(),
                        _pSubtrees->GetData(),
                        _pResources->GetCount(),
                        _pResources->GetData(),
                        &descriptionValue),
                    status);
                AutoDeletePtr<StaticHierarchicalSchemaDescription> description(descriptionValue);
                if (description.Data() != nullptr)
                {
                    Build::HierarchicalSchemaSectionBuilder* schemaBuilderValue = nullptr;
                    Def_HrFailed0(
                        Build::HierarchicalSchemaSectionBuilder::CreateInstanceFromDescription(
                            priSectionBuilder.Data(), description.Data(), Build::PriBuildFromScratch, &schemaBuilderValue),
                        status);
                    AutoDeletePtr<Build::HierarchicalSchemaSectionBuilder> schemaBuilder(schemaBuilderValue);
                    if (schemaBuilder.Data() != nullptr && !Def_HrFailed0(schemaBuilder.Data()->Finalize(), status))
                    {
                        const std::uint32_t schemaSize = schemaBuilder.Data()->GetMaxSizeInBytes();
                        if (status->Succeeded())
                        {
                            const std::size_t allocationSize = _DefArray_Size(1, schemaSize);
                            _pSchemaBlob = allocationSize != 0 ?
                                               static_cast<std::uint8_t*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, allocationSize)) :
                                               nullptr;
                            if (_pSchemaBlob != nullptr)
                            {
                                if (!Def_HrFailed0(schemaBuilder.Data()->Build(_pSchemaBlob, schemaSize, nullptr), status))
                                {
                                    const DEFFILE_SECTION_TYPEID sectionType = schemaBuilder.Data()->GetSectionType();
                                    Def_HrFailed0(
                                        HierarchicalSchema::CreateInstance(
                                            sectionType, _pSchemaBlob, static_cast<int>(schemaSize), &_pSchema),
                                        status);
                                }
                            }
                            else
                            {
                                status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 638, L"", 0);
                            }
                        }
                    }
                }
            }
        }
    }
    return status->GetHResult();
}

} // namespace Microsoft::Resources::Indexers

namespace Microsoft::Resources::Indexers
{

PathSchemaCollection::~PathSchemaCollection()
{
    delete static_cast<StandalonePriFile*>(m_standalonePriFile);
    delete static_cast<CoreProfile*>(m_profile);
    delete static_cast<SchemaCollectionSchemaWrapper*>(m_schemaWrapper);
    delete static_cast<CPriSchemaReader*>(m_priSchemaReader);
}

int PathSchemaCollection::GetNumSchemas() const
{
    const auto* const collection = static_cast<const ISchemaCollection*>(m_schemaCollection);
    return collection != nullptr ? collection->GetNumSchemas() : 0;
}

HRESULT PathSchemaCollection::GetSchema(const int index, const IHierarchicalSchema** const schema) const
{
    return static_cast<const ISchemaCollection*>(m_schemaCollection)->GetSchema(index, schema);
}

HRESULT PathSchemaCollection::GetPrimarySchema(const IHierarchicalSchema** const schema) const
{
    return static_cast<const ISchemaCollection*>(m_schemaCollection)->GetPrimarySchema(schema);
}

HRESULT PathSchemaCollection::GetSchemaById(const wchar_t* const id, const IHierarchicalSchema** const schema) const
{
    return static_cast<const ISchemaCollection*>(m_schemaCollection)->GetSchemaById(id, schema);
}

HRESULT PathSchemaCollection::FindSchema(const HierarchicalSchemaReference* const reference, const IHierarchicalSchema** const schema) const
{
    return static_cast<const ISchemaCollection*>(m_schemaCollection)->FindSchema(reference, schema);
}

bool PathSchemaCollection::Initialize(const wchar_t* const path, IDefStatusEx* const status)
{
    if (m_schemaCollection != nullptr)
    {
        status->SetError(E_DEF_ALREADY_INITIALIZED, L"" __FILE__, 700, L"", 0);
        return status->Succeeded() && m_schemaCollection != nullptr;
    }

    if (DefString_IsSuffixWithOptions(L".pri", path, DefCompare_CaseInsensitive))
    {
        MrmProfile* profile {};
        Def_HrFailed0(
            MrmProfile::ChooseDefaultProfile(
                MrmProfile::ProfileType::EmptyInit,
                MrmPlatformVersionInternal::DefaultPlatformVersion,
                nullptr,
                nullptr,
                nullptr,
                &profile),
            status);
        m_profile = profile;

        if (profile != nullptr)
        {
            StandalonePriFile* file {};
            Def_HrFailed0(StandalonePriFile::CreateInstance(0, path, profile, &file), status);
            m_standalonePriFile = file;
            if (file != nullptr)
            {
                m_schemaCollection = static_cast<ISchemaCollection*>(file);
            }
        }
    }
    else if (DefString_IsSuffixWithOptions(L".xml", path, DefCompare_CaseInsensitive))
    {
        auto* const reader = new CPriSchemaReader();
        m_priSchemaReader = reader;
        if (SUCCEEDED(reader->InitializeFromFile(path, status)))
        {
            const IHierarchicalSchema* const readerSchema = reader->GetSchema();
            if (readerSchema != nullptr)
            {
                SchemaCollectionSchemaWrapper* wrapper {};
                Def_HrFailed0(SchemaCollectionSchemaWrapper::CreateInstance(readerSchema, &wrapper), status);
                m_schemaWrapper = wrapper;
                if (wrapper != nullptr)
                {
                    m_schemaCollection = wrapper;
                }
            }
        }
    }
    else
    {
        status->SetError(E_DEF_UNSUPPORTED_FILE_TYPE, L"" __FILE__, 695, path, 0);
    }

    return status->Succeeded() && m_schemaCollection != nullptr;
}

} // namespace Microsoft::Resources::Indexers
