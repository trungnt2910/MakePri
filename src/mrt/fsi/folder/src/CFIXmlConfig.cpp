#include "StdAfx.h"

#include <CFIXmlConfig.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* CFIXmlConfig::s_pFISchema =
    LR"xml(<xs:schema attributeFormDefault="unqualified" elementFormDefault="qualified" xmlns:xs="http://www.w3.org/2001/XMLSchema">)xml"
        LR"xml(<xs:simpleType name="ExclusionTypeList">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:enumeration value="path"/>)xml"
                LR"xml(<xs:enumeration value="extension"/>)xml"
                LR"xml(<xs:enumeration value="name"/>)xml"
                LR"xml(<xs:enumeration value="tree"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:complexType name="FolderExclusionType">)xml"
            LR"xml(<xs:attribute  name="type"  type="ExclusionTypeList" use="required"/>)xml"
            LR"xml(<xs:attribute  name="value"  type="xs:string" use="required"/>)xml"
            LR"xml(<xs:attribute  name="doNotTraverse"  type="xs:boolean" use="required"/>)xml"
            LR"xml(<xs:attribute  name="doNotIndex"  type="xs:boolean" use="required"/>)xml"
        LR"xml(</xs:complexType>)xml"
        LR"xml(<xs:simpleType name="IndexerConfigFolderType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((f|F)(o|O)(l|L)(d|D)(e|E)(r|R))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:sequence>)xml"
                    LR"xml(<xs:element name="exclude" type="FolderExclusionType"  minOccurs="0" maxOccurs="unbounded"/>)xml"
                LR"xml(</xs:sequence>)xml"
                LR"xml(<xs:attribute  name="type"  type="IndexerConfigFolderType" use="required"/>)xml"
                LR"xml(<xs:attribute  name="foldernameAsQualifier"  type="xs:boolean" use="required"/>)xml"
                LR"xml(<xs:attribute  name="filenameAsQualifier"  type="xs:boolean" use="required"/>)xml"
                LR"xml(<xs:attribute  name="qualifierDelimiter"  type="xs:string" use="required"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

CFIXmlConfig::CFIXmlConfig(IXMLDOMNode* const pDomNode) : _pDomNode(pDomNode)
{
    _pFolderConfigXmlHelper = new (std::nothrow) CXmlHelper(_pDomNode);
}

CFIXmlConfig::~CFIXmlConfig()
{
    for (auto iterator = _excludeTypeMap.begin(); iterator != _excludeTypeMap.end(); ++iterator)
    {
        auto* const attributes = iterator->second;
        if (attributes != nullptr)
        {
            delete attributes;
        }
    }
    _excludeTypeMap.clear();
    if (_pFolderConfigXmlHelper != nullptr)
    {
        delete _pFolderConfigXmlHelper;
    }
}

HRESULT CFIXmlConfig::IsExcludedFolder(const wchar_t* const pFolderPath, const wchar_t* const pFolder, CExclusionResult* const pResult)
{
    HRESULT result = E_FAIL;
    const wchar_t* path = pFolderPath + 1;
    if (*pFolderPath != L'\\')
    {
        path = pFolderPath;
    }

    std::wstring fullPath(path);
    std::wstring folderName(pFolder);
    if (!fullPath.empty() && path[fullPath.length() - 1] != L'\\')
    {
        fullPath.append(L"\\");
    }
    fullPath.append(folderName, 0, std::wstring::npos);

    HRESULT operationResult = _IsExcludedFolderOrFile(fullPath, EXCLUDE_TYPE_FLAG::PATH, pResult);
    if (SUCCEEDED(operationResult))
    {
        result = operationResult;
    }
    operationResult = _IsExcludedFolderOrFile(fullPath, EXCLUDE_TYPE_FLAG::TREE, pResult);
    if (SUCCEEDED(operationResult))
    {
        result = operationResult;
    }
    operationResult = _IsExcludedFolderOrFile(folderName, EXCLUDE_TYPE_FLAG::NAME, pResult);
    if (SUCCEEDED(operationResult))
    {
        result = operationResult;
    }

    if (SUCCEEDED(result))
    {
        std::wostringstream output;
        output << L"Folder Excluded: " << L"ParentFolderPath - " << fullPath << std::endl;
        output << L"Folder - " << pFolder << std::endl;
        DebugOutput(output.str().c_str());
    }
    return result;
}

HRESULT CFIXmlConfig::IsExcludedFile(const wchar_t* const pFolderPath, const wchar_t* const pFile, CExclusionResult* const pResult)
{
    HRESULT result = E_FAIL;
    HRESULT operationResult = IsExcludedFolder(pFolderPath, pFile, pResult);
    if (SUCCEEDED(operationResult))
    {
        result = operationResult;
    }

    const std::wstring fileName(pFile);
    const std::size_t extensionPosition = fileName.rfind(L".", std::wstring::npos, 1);
    if (extensionPosition != std::wstring::npos)
    {
        std::wstring extension = fileName.substr(extensionPosition, std::wstring::npos);
        operationResult = _IsExcludedFolderOrFile(extension, EXCLUDE_TYPE_FLAG::EXTENSION, pResult);
        if (SUCCEEDED(operationResult))
        {
            result = operationResult;
        }
        extension.clear();
        extension.assign(fileName.substr(extensionPosition + 1, std::wstring::npos), 0, std::wstring::npos);
        operationResult = _IsExcludedFolderOrFile(extension, EXCLUDE_TYPE_FLAG::EXTENSION, pResult);
        if (SUCCEEDED(operationResult))
        {
            result = operationResult;
        }
    }
    return result;
}

HRESULT CFIXmlConfig::_IsExcludedFolderOrFile(
    const std::wstring& wszFolderOrFile,
    const EXCLUDE_TYPE_FLAG iSearchExcludeType,
    CExclusionResult* const pResult)
{
    HRESULT result = E_FAIL;
    for (auto iterator = _excludeTypeMap.begin(); iterator != _excludeTypeMap.end(); ++iterator)
    {
        std::wstring exclusion(iterator->first);
        bool matched = false;
        if (exclusion.compare(L"\\") == 0)
        {
            exclusion.assign(LOCALE_NAME_INVARIANT, 0);
            matched = (static_cast<std::uint32_t>(iSearchExcludeType) & static_cast<std::uint32_t>(EXCLUDE_TYPE_FLAG::TREE)) != 0;
            if (matched)
            {
                goto process_match;
            }
        }

        {
            wchar_t lowercaseBuffer[266];
            const int mappedLength = LCMapStringEx(
                LOCALE_NAME_INVARIANT,
                LCMAP_LOWERCASE,
                wszFolderOrFile.c_str(),
                static_cast<int>(wszFolderOrFile.length()),
                lowercaseBuffer,
                MAX_PATH,
                nullptr,
                nullptr,
                0);
            if (mappedLength >= MAX_PATH)
            {
                continue;
            }
            lowercaseBuffer[mappedLength] = L'\0';
            std::wstring lowercaseValue;
            lowercaseValue.assign(lowercaseBuffer, wcslen(lowercaseBuffer));
            if (lowercaseValue.compare(exclusion) == 0)
            {
                matched = true;
            }
            else if (
                (static_cast<std::uint32_t>(iSearchExcludeType) & static_cast<std::uint32_t>(EXCLUDE_TYPE_FLAG::TREE)) != 0 &&
                lowercaseValue.find(exclusion.c_str(), 0, exclusion.length()) == 0)
            {
                if (lowercaseValue[exclusion.length()] == L'\0' || lowercaseValue[exclusion.length()] == L'\\')
                {
                    matched = true;
                }
            }
        }

    process_match:
        if (matched)
        {
            auto* const attributes = iterator->second;
            for (auto attribute = attributes->begin(); attribute < attributes->end(); ++attribute)
            {
                if ((static_cast<std::uint32_t>(iSearchExcludeType) & static_cast<std::uint32_t>(attribute->type)) != 0)
                {
                    result = S_OK;
                    pResult->doNotTraverse |= attribute->result.doNotTraverse;
                    pResult->doNotIndex |= attribute->result.doNotIndex;
                }
            }
        }
    }
    return result;
}

HRESULT CFIXmlConfig::_ProcessExcludeNode(IXMLDOMNode* const pExcludeNode, IDefStatusEx* const pStatus)
{
    HRESULT result = S_OK;
    CXmlHelper helper(pExcludeNode);
    wchar_t* typeValue = nullptr;
    EXCLUDE_ATTRIBUTES attributes {};
    helper.GetAttributeValue(L"type", pStatus, &typeValue);
    if (wcscmp(typeValue, L"path") == 0)
    {
        attributes.type = EXCLUDE_TYPE_FLAG::PATH;
    }
    else if (wcscmp(typeValue, L"name") == 0)
    {
        attributes.type = EXCLUDE_TYPE_FLAG::NAME;
    }
    else if (wcscmp(typeValue, L"extension") == 0)
    {
        attributes.type = EXCLUDE_TYPE_FLAG::EXTENSION;
    }
    else if (wcscmp(typeValue, L"tree") == 0)
    {
        attributes.type = EXCLUDE_TYPE_FLAG::TREE;
    }

    wchar_t* value = nullptr;
    helper.GetAttributeValue(L"value", pStatus, &value);
    StringResult normalizedValue;
    if (Def_HrFailed0(DefStringResult_SetCopy(normalizedValue.GetStringResult(), value), pStatus))
    {
        goto cleanup;
    }

    {
        wchar_t* writableValue = nullptr;
        std::size_t capacity = 0;
        Def_HrFailed0(normalizedValue.GetWritableRef(&writableValue, &capacity), pStatus);
        if (writableValue == nullptr)
        {
            goto cleanup;
        }

        const wchar_t* const reference = normalizedValue.GetRef();
        DEFCOMPARISON comparison = Def_Equal;
        if (reference != nullptr && *reference != L'\0' &&
            (DefStringResult_CompareWithOptions(normalizedValue.GetStringResult(), L"\\", DefCompare_Default, &comparison),
             comparison != Def_Equal))
        {
            if (writableValue[capacity - 2] == L'\\')
            {
                writableValue[capacity - 2] = L'\0';
            }
            if (*writableValue == L'\\')
            {
                ++writableValue;
                --capacity;
            }
        }
        else
        {
            if (!Def_HrFailed0(DefStringResult_SetCopy(normalizedValue.GetStringResult(), L"\\"), pStatus))
            {
                Def_HrFailed0(normalizedValue.GetWritableRef(&writableValue, &capacity), pStatus);
            }
        }

        if (writableValue != nullptr && pStatus->Succeeded())
        {
            wchar_t lowercaseBuffer[266];
            const int mappedLength = LCMapStringEx(
                LOCALE_NAME_INVARIANT,
                LCMAP_LOWERCASE,
                writableValue,
                static_cast<int>(capacity),
                lowercaseBuffer,
                MAX_PATH,
                nullptr,
                nullptr,
                0);
            if (mappedLength < MAX_PATH)
            {
                lowercaseBuffer[mappedLength] = L'\0';
                std::wstring lowercaseValue;
                lowercaseValue.assign(lowercaseBuffer, wcslen(lowercaseBuffer));

                std::vector<EXCLUDE_ATTRIBUTES>* attributeList = nullptr;
                const auto existing = _excludeTypeMap.find(lowercaseValue);
                if (existing == _excludeTypeMap.end())
                {
                    attributeList = new (std::nothrow) std::vector<EXCLUDE_ATTRIBUTES>();
                    _excludeTypeMap.insert(std::pair<const std::wstring, std::vector<EXCLUDE_ATTRIBUTES>*>(lowercaseValue, attributeList));
                }
                else
                {
                    attributeList = existing->second;
                }

                result = _IsValidValue(lowercaseValue, attributes.type, attributes, pStatus);
                if (SUCCEEDED(result))
                {
                    _variant_t doNotTraverse;
                    helper.GetAttributeValueAsVariant(L"doNotTraverse", &doNotTraverse);
                    attributes.result.doNotTraverse = static_cast<bool>(doNotTraverse);
                    _variant_t doNotIndex;
                    result = helper.GetAttributeValueAsVariant(L"doNotIndex", &doNotIndex);
                    attributes.result.doNotIndex = static_cast<bool>(doNotIndex);
                    attributeList->push_back(attributes);
                }
            }
        }
    }

cleanup:
    operator delete(value);
    operator delete(typeValue);
    return ComputeHResult(result, pStatus);
}

HRESULT CFIXmlConfig::_ProcessIndexerConfigNode(IXMLDOMNode* const pIndexerConfig, IDefStatusEx* const pStatus)
{
    CXmlHelper helper(pIndexerConfig);
    std::uint32_t index = 0;
    _variant_t folderNameAsQualifier;
    helper.GetAttributeValueAsVariant(L"foldernameAsQualifier", &folderNameAsQualifier);
    _bFoldernameAsQualifier = static_cast<bool>(folderNameAsQualifier);
    _variant_t fileNameAsQualifier;
    helper.GetAttributeValueAsVariant(L"filenameAsQualifier", &fileNameAsQualifier);
    _bFilenameAsQualifier = static_cast<bool>(fileNameAsQualifier);

    wchar_t* qualifierDelimiter = nullptr;
    HRESULT result = helper.GetAttributeValue(L"qualifierDelimiter", pStatus, &qualifierDelimiter);
    if (wcslen(qualifierDelimiter) != 1 || *qualifierDelimiter == L'-' || *qualifierDelimiter == L'_')
    {
        pStatus->SetError(E_DEF_FSI_INVALID_DELIMITER, qualifierDelimiter);
        result = ComputeHResult(result, pStatus);
    }
    else
    {
        Def_HrFailed0(DefStringResult_SetCopy(_strQualifierDelimiter.GetStringResult(), qualifierDelimiter), pStatus);
        operator delete(qualifierDelimiter);

        IXMLDOMNodeList* children = nullptr;
        result = helper.TryGetChildren(L"exclude", pStatus, &children);
        if (SUCCEEDED(result) && children != nullptr)
        {
            LONG length = 0;
            result = children->get_length(&length);
            if (length > 0)
            {
                do
                {
                    IXMLDOMNode* child = nullptr;
                    result = children->get_item(static_cast<LONG>(index), &child);
                    if (SUCCEEDED(result) && child != nullptr)
                    {
                        result = _ProcessExcludeNode(child, pStatus);
                    }
                    SAFE_RELEASE(child);
                    ++index;
                } while (static_cast<LONG>(index) < length);
            }
        }
        else
        {
            pStatus->Reset();
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    return result;
}

bool CFIXmlConfig::IsFoldernameAsDimension() { return _bFoldernameAsQualifier; }

bool CFIXmlConfig::IsFilenameAsDimension() { return _bFilenameAsQualifier; }

const wchar_t* CFIXmlConfig::GetQualifierDelimiter() { return _strQualifierDelimiter.GetRef(); }

HRESULT CFIXmlConfig::Parse(IDefStatusEx* const pStatus)
{
    HRESULT result = S_OK;
    IXMLDOMNodeList* children = nullptr;
    bool found = false;
    if (_pFolderConfigXmlHelper != nullptr)
    {
        result = _pFolderConfigXmlHelper->ValidateChildNodeAgainstChildSchema(
            L"indexer-config", s_pFISchema, L"type", L"folder", false, pStatus);
        if (SUCCEEDED(result))
        {
            _pFolderConfigXmlHelper->TryGetChildren(L"indexer-config", pStatus, &children);
            LONG length = 0;
            children->get_length(&length);
            for (LONG index = 0; index < length; ++index)
            {
                if (found)
                {
                    break;
                }

                IXMLDOMNode* child;
                result = children->get_item(index, &child);
                if (SUCCEEDED(result))
                {
                    CXmlHelper helper(child);
                    wchar_t* type = nullptr;
                    result = helper.GetAttributeValue(L"type", pStatus, &type);
                    if (DefString_CompareWithOptions(type, L"folder", DefCompare_CaseInsensitive) == 0)
                    {
                        result = _ProcessIndexerConfigNode(child, pStatus);
                        found = true;
                    }
                    operator delete(type);
                    SAFE_RELEASE(child);
                }
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        }
    }
    return result;
}

HRESULT CFIXmlConfig::_IsValidValue(
    const std::wstring& wszValue,
    const EXCLUDE_TYPE_FLAG iSearchExcludeType,
    const EXCLUDE_ATTRIBUTES attribute,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(attribute);
    const std::size_t length = wszValue.length();
    if (iSearchExcludeType == EXCLUDE_TYPE_FLAG::TREE)
    {
        if (!wszValue.empty() && wszValue[length - 1] == L'.')
        {
            goto invalid;
        }
    }
    else
    {
        if (wszValue.empty() || wszValue[length - 1] == L'.')
        {
            goto invalid;
        }
        if (iSearchExcludeType == EXCLUDE_TYPE_FLAG::EXTENSION)
        {
            if (wszValue.find(L".", 1, 1) == std::wstring::npos && wszValue.find_first_of(L"/:*?\"<>|\\\n\t\r ", 0) == std::wstring::npos)
            {
                return S_OK;
            }
            goto invalid;
        }
    }

    if (wszValue.find_first_of(L"/:*?\"<>|\n\t\r", 0) != std::wstring::npos || wszValue.find(L"\\\\", 0, 2) != std::wstring::npos ||
        (iSearchExcludeType == EXCLUDE_TYPE_FLAG::NAME && wszValue.find_first_of(L"\\", 0) != std::wstring::npos))
    {
        goto invalid;
    }

    {
        std::size_t position = wszValue.find(L".", 0, 1);
        if (position != std::wstring::npos)
        {
            const std::size_t last = length - 1;
            if (position < last)
            {
                while (true)
                {
                    if (wszValue[position + 1] == L'.')
                    {
                        ++position;
                    }
                    else
                    {
                        if (wszValue[position + 1] == L'\\')
                        {
                            goto invalid;
                        }
                        position = wszValue.find(L".", position + 1, 1);
                        if (position == std::wstring::npos)
                        {
                            return S_OK;
                        }
                    }
                    if (position >= last)
                    {
                        return S_OK;
                    }
                }
            }
        }
    }
    return S_OK;

invalid:
    pStatus->SetError(E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE, wszValue.c_str());
    return pStatus->GetHResult();
}
} // namespace Microsoft::Resources::Indexers
