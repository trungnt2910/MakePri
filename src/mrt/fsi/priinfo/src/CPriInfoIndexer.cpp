#include "StdAfx.h"

#include <CPriInfoIndexer.h>

namespace Microsoft::Resources::Indexers
{
namespace
{

// clang-format off
constexpr const wchar_t* s_pszResFilesSchema =
    LR"xml(<xs:schema id="priinfo" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigPriInfoType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((p|P)(r|R)(i|I)(i|I)(n|N)(f|F)(o|O))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute name="type" type="IndexerConfigPriInfoType" use="required"/>)xml"
                LR"xml(<xs:attribute name="emitStrings" type="xs:boolean" use="optional"/>)xml"
                LR"xml(<xs:attribute name="emitPaths" type="xs:boolean" use="optional"/>)xml"
                LR"xml(<xs:attribute name="emitEmbeddedData" type="xs:boolean" use="optional"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

// clang-format off
constexpr const wchar_t* s_pszPriInfoSchema =
    LR"xml(<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" >)xml"
        LR"xml(<xs:simpleType name="candidateType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="Path|String|EmbeddedData"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:complexType name="scopeType">)xml"
            LR"xml(<xs:sequence>)xml"
                LR"xml(<xs:element name="ResourceMapSubtree" type="scopeType" minOccurs="0" maxOccurs="unbounded"/>)xml"
                LR"xml(<xs:element name="NamedResource" minOccurs="0" maxOccurs="unbounded">)xml"
                    LR"xml(<xs:complexType>)xml"
                        LR"xml(<xs:sequence>)xml"
                            LR"xml(<xs:element name="Decision" minOccurs="0" maxOccurs="unbounded">)xml"
                                LR"xml(<xs:complexType>)xml"
                                    LR"xml(<xs:sequence>)xml"
                                        LR"xml(<xs:any processContents="skip" minOccurs="0" maxOccurs="unbounded"/>)xml"
                                    LR"xml(</xs:sequence>)xml"
                                    LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                                LR"xml(</xs:complexType>)xml"
                            LR"xml(</xs:element>)xml"
                            LR"xml(<xs:element name="Link" type="xs:string" minOccurs="0" maxOccurs="1" />)xml"
                            LR"xml(<xs:element name="Candidate" minOccurs="0" maxOccurs="unbounded">)xml"
                                LR"xml(<xs:complexType>)xml"
                                    LR"xml(<xs:sequence>)xml"
                                        LR"xml(<xs:element name="QualifierSet" minOccurs="0" maxOccurs="unbounded">)xml"
                                            LR"xml(<xs:complexType>)xml"
                                                LR"xml(<xs:sequence>)xml"
                                                    LR"xml(<xs:element name="Qualifier" minOccurs="0" maxOccurs="unbounded">)xml"
                                                        LR"xml(<xs:complexType>)xml"
                                                            LR"xml(<xs:attribute name="name" type="xs:string" use="required" />)xml"
                                                            LR"xml(<xs:attribute name="value" type="xs:string" use="required" />)xml"
                                                            LR"xml(<xs:attribute name="scoreAsDefault" type="xs:decimal" use="optional" />)xml"
                                                            LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                                                        LR"xml(</xs:complexType>)xml"
                                                    LR"xml(</xs:element>)xml"
                                                LR"xml(</xs:sequence>)xml"
                                                LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                                            LR"xml(</xs:complexType>)xml"
                                        LR"xml(</xs:element>)xml"
                                        LR"xml(<xs:choice>)xml"
                                            LR"xml(<xs:element name="Value" minOccurs="0" maxOccurs="unbounded">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:simpleContent>)xml"
                                                        LR"xml(<xs:extension base="xs:string">)xml"
                                                            LR"xml(<xs:attribute name="rawLocator" type="xs:string" use="optional"/>)xml"
                                                        LR"xml(</xs:extension>)xml"
                                                    LR"xml(</xs:simpleContent>)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                            LR"xml(<xs:element name="Base64Value" minOccurs="0" maxOccurs="unbounded">)xml"
                                                LR"xml(<xs:complexType>)xml"
                                                    LR"xml(<xs:simpleContent>)xml"
                                                        LR"xml(<xs:extension base="xs:base64Binary">)xml"
                                                            LR"xml(<xs:attribute name="rawLocator" type="xs:string" use="optional"/>)xml"
                                                        LR"xml(</xs:extension>)xml"
                                                    LR"xml(</xs:simpleContent>)xml"
                                                LR"xml(</xs:complexType>)xml"
                                            LR"xml(</xs:element>)xml"
                                        LR"xml(</xs:choice>)xml"
                                    LR"xml(</xs:sequence>)xml"
                                    LR"xml(<xs:attribute name="type" type="candidateType" use="required" />)xml"
                                LR"xml(</xs:complexType>)xml"
                            LR"xml(</xs:element>)xml"
                        LR"xml(</xs:sequence>)xml"
                        LR"xml(<xs:attribute name="name" use="required" type="xs:string" />)xml"
                        LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                    LR"xml(</xs:complexType>)xml"
                LR"xml(</xs:element>)xml"
            LR"xml(</xs:sequence>)xml"
            LR"xml(<xs:attribute name="name" use="required" type="xs:string" />)xml"
            LR"xml(<xs:anyAttribute processContents="skip" />)xml"
        LR"xml(</xs:complexType>)xml"
        LR"xml(<xs:element name="PriInfo">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:sequence>)xml"
                    LR"xml(<xs:element name="PriHeader" minOccurs="0" maxOccurs="1">)xml"
                        LR"xml(<xs:complexType>)xml"
                            LR"xml(<xs:sequence>)xml"
                                LR"xml(<xs:any minOccurs ="0" maxOccurs="unbounded" processContents="skip" />)xml"
                            LR"xml(</xs:sequence>)xml"
                            LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                        LR"xml(</xs:complexType>)xml"
                    LR"xml(</xs:element>)xml"
                    LR"xml(<xs:element name="QualifierInfo" minOccurs="0" maxOccurs="1">)xml"
                        LR"xml(<xs:complexType>)xml"
                            LR"xml(<xs:sequence>)xml"
                                LR"xml(<xs:any minOccurs="0" maxOccurs="unbounded" processContents="skip" />)xml"
                            LR"xml(</xs:sequence>)xml"
                        LR"xml(</xs:complexType>)xml"
                    LR"xml(</xs:element>)xml"
                    LR"xml(<xs:element name="ResourceMap">)xml"
                        LR"xml(<xs:complexType>)xml"
                            LR"xml(<xs:sequence>)xml"
                                LR"xml(<xs:element name="VersionInfo" minOccurs="0" maxOccurs="1">)xml"
                                    LR"xml(<xs:complexType>)xml"
                                        LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                                    LR"xml(</xs:complexType>)xml"
                                LR"xml(</xs:element>)xml"
                                LR"xml(<xs:element minOccurs="0" maxOccurs="unbounded" name="ResourceMapSubtree" type="scopeType" />)xml"
                            LR"xml(</xs:sequence>)xml"
                            LR"xml(<xs:attribute name="name" type="xs:string" use="optional" />)xml"
                            LR"xml(<xs:anyAttribute processContents="skip" />)xml"
                        LR"xml(</xs:complexType>)xml"
                    LR"xml(</xs:element>)xml"
                LR"xml(</xs:sequence>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

constexpr std::uint8_t Base64Table[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 255, 255, 63,  52,  53,  54,  55,
    56,  57,  58,  59,  60,  61,  255, 255, 255, 0,   255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
    13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,
    33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255,
};

HRESULT __fastcall base64decodeU(
    const wchar_t* const source,
    std::uint8_t* const output,
    const std::uint32_t outputCapacity,
    std::uint32_t* const outputSize)
{
    const wchar_t* current = source;
    while (*current++ != L'\0')
    {
    }
    const std::uint32_t sourceLength = static_cast<std::uint32_t>(current - source - 1);

    if (outputSize != nullptr)
    {
        *outputSize = 0;
    }
    if (sourceLength == 0 || (sourceLength & 3) != 0)
    {
        return E_INVALIDARG;
    }

    std::uint32_t decodedSize = 3 * ((sourceLength + 3) >> 2);
    if (source[sourceLength - 1] == L'=')
    {
        if (source[sourceLength - 2] == L'=')
        {
            decodedSize -= 2;
        }
        else
        {
            --decodedSize;
        }
    }
    if (outputSize != nullptr)
    {
        *outputSize = decodedSize;
    }
    if (outputCapacity == 0 && output == nullptr)
    {
        return S_OK;
    }
    if (decodedSize > outputCapacity)
    {
        return static_cast<HRESULT>(STATUS_BUFFER_TOO_SMALL);
    }

    std::uint32_t inputIndex = 0;
    std::uint32_t outputIndex = 0;
    while (true)
    {
        const wchar_t source0 = source[inputIndex];
        const wchar_t source1 = source[inputIndex + 1];
        const wchar_t source2 = source[inputIndex + 2];
        const wchar_t source3 = source[inputIndex + 3];
        const std::uint8_t value0 = static_cast<std::uint32_t>(source0) >= 128 ? 255 : Base64Table[source0];
        const std::uint8_t value1 = static_cast<std::uint32_t>(source1) >= 128 ? 255 : Base64Table[source1];
        const std::uint8_t value2 = static_cast<std::uint32_t>(source2) >= 128 ? 255 : Base64Table[source2];
        const std::uint8_t value3 = static_cast<std::uint32_t>(source3) >= 128 ? 255 : Base64Table[source3];
        inputIndex += 4;

        if (value0 == 255 || value1 == 255 || value2 == 255 || value3 == 255)
        {
            return static_cast<HRESULT>(STATUS_INVALID_PARAMETER);
        }

        output[outputIndex] = static_cast<std::uint8_t>((4 * value0) | (value1 >> 4));
        ++outputIndex;
        if (outputIndex < decodedSize)
        {
            output[outputIndex] = static_cast<std::uint8_t>((16 * value1) | (value2 >> 2));
            ++outputIndex;
            if (outputIndex < decodedSize)
            {
                output[outputIndex] = static_cast<std::uint8_t>(value3 | (value2 << 6));
                ++outputIndex;
            }
        }
        if (inputIndex >= sourceLength)
        {
            return S_OK;
        }
    }
}

} // namespace

CPriInfoIndexer::~CPriInfoIndexer()
{
    _pEnvironment = nullptr;
    _pProjectRoot = nullptr;
    _pQualApplicator = nullptr;
}

HRESULT CPriInfoIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pQualApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(options);
    _pEnvironment = pEnvironment;
    _pProjectRoot = pProjectRootFolder;
    _pQualApplicator = pQualApplicator;
    return ParseIndexPassNode(pIndexPassNode, pStatus);
}

HRESULT CPriInfoIndexer::ParseCandidateNode(
    const wchar_t* const pszCollectionName,
    const wchar_t* const pszItemName,
    IXMLDOMNode* const pXmlNode,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    if (pEntry == nullptr || _pEnvironment == nullptr || _pQualApplicator == nullptr)
    {
        return E_FAIL;
    }

    MrmEnvironment::ResourceItemType itemType = MrmEnvironment::ResourceItemType_String;
    MrmEnvironment::ResourceValueType valueType = MrmEnvironment::ResourceValueType_Utf16String;
    int qualifierSetIndex = 0;
    bool emitCandidate = false;
    bool isPath = false;
    bool isEmbeddedData = false;
    wchar_t* type = nullptr;
    IXMLDOMNode* childNode = nullptr;

    CXmlHelper helper(pXmlNode);
    HRESULT result = helper.GetAttributeValue(L"type", pStatus, &type);
    if (FAILED(result))
    {
        return result;
    }
    if (type != nullptr)
    {
        if (DefString_CompareWithOptions(type, L"String", DefCompare_CaseInsensitive) == 0 && _fEmitStringResources)
        {
            itemType = MrmEnvironment::ResourceItemType_String;
            valueType = MrmEnvironment::ResourceValueType_Utf16String;
            emitCandidate = true;
        }
        else if (DefString_CompareWithOptions(type, L"Path", DefCompare_CaseInsensitive) == 0 && _fEmitPathResources)
        {
            itemType = MrmEnvironment::ResourceItemType_Path;
            valueType = MrmEnvironment::ResourceValueType_Utf16Path;
            emitCandidate = true;
            isPath = true;
        }
        else if (DefString_CompareWithOptions(type, L"EmbeddedData", DefCompare_CaseInsensitive) == 0 && _fEmitEmbeddedDataResources)
        {
            itemType = MrmEnvironment::ResourceItemType_Path;
            valueType = MrmEnvironment::ResourceValueType_EmbeddedData;
            emitCandidate = true;
            isEmbeddedData = true;
        }
        else
        {
            if (pStatus != nullptr)
            {
                pStatus->SetError(E_MRM_BAD_INSTANCE_TYPE, L"" __FILE__, 756, L"", 0);
            }
            result = pStatus->GetHResult();
        }
        delete[] type;
        if (FAILED(result))
        {
            return result;
        }
    }

    result = helper.TryGetChildNode(L"QualifierSet", pStatus, &childNode);
    if (FAILED(result))
    {
        return result;
    }
    if (childNode != nullptr)
    {
        CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
        result = _pQualApplicator->GetQualifierSetBuilder(0, pStatus, &builder);
        if (SUCCEEDED(result) && builder != nullptr)
        {
            result = ParseQualifierSetNode(childNode, isPath || isEmbeddedData, builder, pStatus);
            if (SUCCEEDED(result))
            {
                result = _pQualApplicator->ApplyQualifierSetFromBuilder(builder, pStatus, &qualifierSetIndex);
            }
            delete builder;
        }
        SAFE_RELEASE(childNode);
        if (FAILED(result))
        {
            return result;
        }
    }
    if (!emitCandidate)
    {
        return result;
    }

    result = helper.TryGetChildNode(isEmbeddedData ? L"Base64Value" : L"Value", pStatus, &childNode);
    if (FAILED(result) || childNode == nullptr)
    {
        return result;
    }

    BSTR text = nullptr;
    result = childNode->get_text(&text);
    if (FAILED(result))
    {
        SAFE_RELEASE(childNode);
        return result;
    }

    {
        DefStatusEx localStatus;
        CItemInstanceEntry* candidateEntry = nullptr;
        if (isEmbeddedData)
        {
            std::uint32_t decodedBufferSize;
            result = base64decodeU(text, nullptr, 0, &decodedBufferSize);
            if (SUCCEEDED(result))
            {
                BlobResult blob;
                void* buffer;
                Def_HrFailed0(DefBlobResult_SetEmptyContents(blob.GetBlobResult(), decodedBufferSize, &buffer, nullptr), &localStatus);
                if (buffer != nullptr && localStatus.GetWhat() >= 0)
                {
                    std::uint32_t decodedSize;
                    result = base64decodeU(text, static_cast<std::uint8_t*>(buffer), decodedBufferSize, &decodedSize);
                    if (SUCCEEDED(result))
                    {
                        const wchar_t* const valueTypeName = nullptr;
                        candidateEntry = CItemInstanceEntry::NewForEmbeddedData(
                            pszCollectionName,
                            pszItemName,
                            itemType,
                            valueType,
                            &blob,
                            qualifierSetIndex,
                            1,
                            valueTypeName,
                            nullptr,
                            &localStatus);
                    }
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
            const wchar_t* valueTypeName = nullptr;
            if (!_strQualifierDelimiter.empty())
            {
                valueTypeName = _strQualifierDelimiter.c_str();
            }
            candidateEntry = CItemInstanceEntry::NewForString(
                pszCollectionName, pszItemName, itemType, valueType, text, qualifierSetIndex, 1, valueTypeName, nullptr, &localStatus);
        }

        AutoDeletePtr<CItemInstanceEntry> entry(candidateEntry);
        if (entry.Data() != nullptr)
        {
            result = pTraversalSink->AddEntry(entry.Data());
            if (SUCCEEDED(result))
            {
                entry.Detach();
            }
        }
        else if (SUCCEEDED(result))
        {
            result = localStatus.GetHResult();
        }
        SysFreeString(text);
    }

    SAFE_RELEASE(childNode);
    return result;
}

HRESULT CPriInfoIndexer::ParseIndexConfigNode(IXMLDOMNode* const pIndexConfigNode, IDefStatusEx* const pStatus)
{
    if (pIndexConfigNode == nullptr)
    {
        return E_FAIL;
    }

    HRESULT result = S_OK;
    wchar_t* type = nullptr;
    CXmlHelper helper(pIndexConfigNode);
    helper.GetAttributeValue(L"type", pStatus, &type);
    if (DefString_CompareWithOptions(type, L"priinfo", DefCompare_CaseInsensitive) == 0)
    {
        _variant_t emitStrings;
        _variant_t emitPaths;
        _variant_t emitEmbeddedData;

        result = helper.GetAttributeValueAsVariant(L"emitStrings", &emitStrings);
        if (result == S_OK)
        {
            _fEmitStringResources = static_cast<bool>(emitStrings);
        }
        if (SUCCEEDED(result))
        {
            result = helper.GetAttributeValueAsVariant(L"emitPaths", &emitPaths);
            if (result == S_OK)
            {
                _fEmitPathResources = static_cast<bool>(emitPaths);
            }
            if (SUCCEEDED(result))
            {
                result = helper.GetAttributeValueAsVariant(L"emitEmbeddedData", &emitEmbeddedData);
                if (result == S_OK)
                {
                    _fEmitEmbeddedDataResources = static_cast<bool>(emitEmbeddedData);
                }
            }
        }

        if (result == S_FALSE)
        {
            result = S_OK;
        }
    }
    delete[] type;
    return result;
}

HRESULT CPriInfoIndexer::ParseItemNode(
    const wchar_t* const pszCollectionName,
    const wchar_t* const pszScopeString,
    IXMLDOMNode* const pXmlNode,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    wchar_t* name = nullptr;
    std::wstring itemName;
    if (pszScopeString != nullptr)
    {
        itemName.append(pszScopeString);
        itemName.append(L"\\");
    }

    CXmlHelper helper(pXmlNode);
    HRESULT result = helper.GetAttributeValue(L"name", pStatus, &name);
    if (SUCCEEDED(result) && name != nullptr)
    {
        itemName.append(name);
        result = helper.TryGetChildren(L"Candidate", pStatus, &children);
        if (SUCCEEDED(result) && children != nullptr)
        {
            while (children->nextNode(&child) == S_OK)
            {
                if (child == nullptr)
                {
                    break;
                }
                result = ParseCandidateNode(pszCollectionName, itemName.c_str(), child, pEntry, pTraversalSink, pStatus);
                SAFE_RELEASE(child);
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        }

        IXMLDOMNode* linkNode = nullptr;
        DefStatusEx linkStatus;
        if (SUCCEEDED(helper.TryGetChildNode(L"Link", &linkStatus, &linkNode)) && linkNode != nullptr)
        {
            result = ParseLinkNode(pszCollectionName, itemName.c_str(), linkNode, pTraversalSink, &linkStatus);
            SAFE_RELEASE(linkNode);
        }
    }
    delete[] name;
    return result;
}

HRESULT CPriInfoIndexer::ParseLinkNode(
    const wchar_t* const collectionName,
    const wchar_t* const itemName,
    IXMLDOMNode* const linkXmlNode,
    CItemInstanceSink* const traversalSink,
    IDefStatusEx* const status)
{
    BSTR text;
    HRESULT result = linkXmlNode->get_text(&text);
    if (SUCCEEDED(result) && text != nullptr)
    {
        const wchar_t* valueTypeName = nullptr;
        if (!_strQualifierDelimiter.empty())
        {
            valueTypeName = _strQualifierDelimiter.c_str();
        }

        AutoDeletePtr<CItemInstanceEntry> entry(
            CItemInstanceEntry::NewForLink(collectionName, itemName, text, 1, valueTypeName, nullptr, status));
        SysFreeString(text);
        if (entry.Data() != nullptr)
        {
            result = traversalSink->AddEntry(entry.Data());
            if (SUCCEEDED(result))
            {
                entry.Detach();
            }
        }
        else
        {
            result = status->GetHResult();
        }
    }
    return result;
}

HRESULT CPriInfoIndexer::ParseIndexPassNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    if (pIndexPassNode == nullptr)
    {
        return E_FAIL;
    }

    CXmlHelper helper(pIndexPassNode);
    HRESULT result =
        helper.ValidateChildNodeAgainstChildSchema(L"indexer-config", s_pszResFilesSchema, L"type", L"priinfo", false, pStatus);
    if (SUCCEEDED(result))
    {
        IXMLDOMNodeList* children = nullptr;
        helper.TryGetChildren(L"indexer-config", pStatus, &children);

        IXMLDOMNode* child = nullptr;
        do
        {
            if (children->nextNode(&child) != S_OK || child == nullptr)
            {
                break;
            }
            result = ParseIndexConfigNode(child, pStatus);
            SAFE_RELEASE(child);
        } while (SUCCEEDED(result));
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    return result;
}

HRESULT CPriInfoIndexer::ParsePriInfo(
    std::wstring& contents,
    CItemInstanceEntry* const parentEntry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    CXmlHelper helper;
    HRESULT result = helper.Init(contents.c_str(), CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_BLOB, L"PriInfo", status);
    if (SUCCEEDED(result))
    {
        result = helper.ValidateAgainstSchema(s_pszPriInfoSchema, status);
        if (SUCCEEDED(result))
        {
            result = helper.TryGetChildren(L"ResourceMap", status, &children);
            if (SUCCEEDED(result) && children != nullptr)
            {
                while (children->nextNode(&child) == S_OK)
                {
                    if (child == nullptr)
                    {
                        break;
                    }
                    result = ParseResourceMapNode(child, parentEntry, sink, status);
                    SAFE_RELEASE(child);
                    if (FAILED(result))
                    {
                        break;
                    }
                }
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
                if (result == S_FALSE)
                {
                    result = S_OK;
                }
            }
        }
    }
    return result;
}

HRESULT CPriInfoIndexer::ParseQualifierNode(
    IXMLDOMNode* const pXmlNode,
    const bool fIsFile,
    CQualifierApplicator::CQualifierSetBuilder* const pQualifierSetBuilder,
    IDefStatusEx* const pStatus)
{
    if (pQualifierSetBuilder == nullptr)
    {
        return E_FAIL;
    }

    double* score = nullptr;
    int* priority = nullptr;
    wchar_t* name = nullptr;
    wchar_t* value = nullptr;
    double scoreValue = 0.0;
    int priorityValue = 0;
    bool applied = false;

    CXmlHelper helper(pXmlNode);
    HRESULT result = helper.GetAttributeValue(L"name", pStatus, &name);
    if (name != nullptr && (FAILED(result) || ((result = helper.GetAttributeValue(L"value", pStatus, &value)), value != nullptr)))
    {
        if (SUCCEEDED(result))
        {
            _variant_t priorityVariant;
            result = helper.GetAttributeValueAsVariant(L"priority", &priorityVariant);
            if (result == S_OK)
            {
                priorityValue = static_cast<LONG>(priorityVariant);
                priority = &priorityValue;
            }
            else if (result == S_FALSE)
            {
                result = S_OK;
            }

            if (SUCCEEDED(result))
            {
                _variant_t scoreVariant;
                result = helper.GetAttributeValueAsVariant(L"scoreAsDefault", &scoreVariant);
                if (result == S_OK)
                {
                    result =
                        VariantChangeTypeEx(static_cast<VARIANT*>(&scoreVariant), static_cast<VARIANT*>(&scoreVariant), 0x7F, 0, VT_R8);
                    if (SUCCEEDED(result))
                    {
                        scoreValue = scoreVariant.dblVal;
                        score = &scoreValue;
                    }
                }
                else if (result == S_FALSE)
                {
                    result = S_OK;
                }

                if (SUCCEEDED(result))
                {
                    pQualifierSetBuilder->_AddQualifier(
                        name,
                        value,
                        score,
                        priority,
                        static_cast<CQualifierApplicator::tagTOKEN_TYPE>(static_cast<int>(!fIsFile) + 1),
                        &applied,
                        pStatus);
                    if (!applied)
                    {
                        result = E_FAIL;
                    }
                }
            }
        }
    }
    else
    {
        result = E_FAIL;
    }

    delete[] name;
    delete[] value;
    return result;
}

HRESULT CPriInfoIndexer::ParseQualifierSetNode(
    IXMLDOMNode* const pXmlNode,
    const bool fIsFile,
    CQualifierApplicator::CQualifierSetBuilder* const pQualifierSetBuilder,
    IDefStatusEx* const pStatus)
{
    if (pQualifierSetBuilder == nullptr)
    {
        return E_FAIL;
    }

    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    CXmlHelper helper(pXmlNode);
    HRESULT result = helper.TryGetChildren(L"Qualifier", pStatus, &children);
    if (SUCCEEDED(result) && children != nullptr)
    {
        while (children->nextNode(&child) == S_OK)
        {
            if (child == nullptr)
            {
                break;
            }
            result = ParseQualifierNode(child, fIsFile, pQualifierSetBuilder, pStatus);
            SAFE_RELEASE(child);
            if (FAILED(result))
            {
                break;
            }
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    return result;
}

HRESULT CPriInfoIndexer::ParseResourceMapNode(
    IXMLDOMNode* const node,
    CItemInstanceEntry* const parentEntry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    CXmlHelper helper(node);
    HRESULT result = helper.TryGetChildren(L"ResourceMapSubtree", status, &children);
    if (SUCCEEDED(result) && children != nullptr)
    {
        while (children->nextNode(&child) == S_OK && child != nullptr)
        {
            result = ParseScopeNode(nullptr, nullptr, child, parentEntry, sink, status);
            SAFE_RELEASE(child);
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    return result;
}

HRESULT CPriInfoIndexer::ParseScopeNode(
    const wchar_t* pszCollectionName,
    const wchar_t* const pszPriorScopeString,
    IXMLDOMNode* const pXmlNode,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    wchar_t* name = nullptr;
    std::wstring newPath;
    if (pszPriorScopeString != nullptr)
    {
        newPath.append(pszPriorScopeString);
        newPath.append(L"\\");
    }

    CXmlHelper helper(pXmlNode);
    HRESULT result = helper.GetAttributeValue(L"name", pStatus, &name);
    if (SUCCEEDED(result))
    {
        const wchar_t* childPath = nullptr;
        newPath.append(name);
        if (pszCollectionName != nullptr)
        {
            childPath = newPath.c_str();
        }
        else
        {
            pszCollectionName = newPath.c_str();
        }

        result = helper.TryGetChildren(L"ResourceMapSubtree", pStatus, &children);
        if (SUCCEEDED(result))
        {
            if (children != nullptr)
            {
                while (children->nextNode(&child) == S_OK && child != nullptr)
                {
                    result = ParseScopeNode(pszCollectionName, childPath, child, pEntry, pTraversalSink, pStatus);
                    SAFE_RELEASE(child);
                }
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
                if (FAILED(result))
                {
                    delete[] name;
                    return result;
                }
            }

            result = helper.TryGetChildren(L"NamedResource", pStatus, &children);
            if (SUCCEEDED(result) && children != nullptr)
            {
                while (children->nextNode(&child) == S_OK)
                {
                    if (child == nullptr)
                    {
                        break;
                    }
                    result = ParseItemNode(pszCollectionName, childPath, child, pEntry, pTraversalSink, pStatus);
                    SAFE_RELEASE(child);
                    if (FAILED(result))
                    {
                        break;
                    }
                }
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
            }
        }
    }
    delete[] name;
    return result;
}

HRESULT CPriInfoIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus,
    bool* const pbRemoveContainerFromIndex)
{
    HRESULT result = S_OK;
    if (pEntry->resourceItemType == MrmEnvironment::ResourceItemType_PriInfo)
    {
        *pbRemoveContainerFromIndex = true;
        std::wstring contents(pEntry->value.GetRef());
        if (pEntry->valueTypeName.GetRef() != nullptr)
        {
            _strQualifierDelimiter.assign(pEntry->valueTypeName.GetRef());
        }
        return ComputeHResult(ParsePriInfo(contents, pEntry, pTraversalSink, pStatus), pStatus);
    }
    if (pbRemoveContainerFromIndex == nullptr)
    {
        return E_FAIL;
    }
    if (DefString_CompareWithOptions(pEntry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == 0)
    {
        const wchar_t* const path = pEntry->value.GetRef();
        *pbRemoveContainerFromIndex = false;
        const std::uint32_t length = static_cast<std::uint32_t>(wcslen(path));
        if (length <= 8 || DefString_CompareWithOptions(path + length - 8, L".pri.xml", DefCompare_CaseInsensitive) != 0)
        {
            return S_OK;
        }

        const wchar_t* accessiblePath = nullptr;
        result = CUtilities::GetPathInAccessibleFormat(_pProjectRoot, path, pStatus, &accessiblePath);
        if (SUCCEEDED(result))
        {
            _strQualifierDelimiter.assign(accessiblePath);
            if (PathFileExistsW(accessiblePath))
            {
                std::wstring contents;
                result = CUtilities::LoadFile(accessiblePath, contents, pStatus);
                if (SUCCEEDED(result))
                {
                    result = Redirect(accessiblePath, contents, pEntry, pTraversalSink, pStatus);
                    if (SUCCEEDED(result))
                    {
                        *pbRemoveContainerFromIndex = true;
                    }
                }
            }
            else
            {
                pStatus->SetError(E_DEF_FILE_NOT_FOUND, accessiblePath);
            }
        }
        delete[] accessiblePath;
    }
    return result;
}

HRESULT CPriInfoIndexer::Redirect(
    const wchar_t* const accessiblePath,
    std::wstring& contents,
    CItemInstanceEntry* const entry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    AutoDeletePtr<CItemInstanceEntry> redirected(
        CItemInstanceEntry::NewForString(
            entry->source.GetRef(),
            entry->itemName.GetRef(),
            MrmEnvironment::ResourceItemType_PriInfo,
            MrmEnvironment::ResourceValueType_Utf8String,
            contents.c_str(),
            entry->qualifierSetIndex,
            2,
            accessiblePath,
            nullptr,
            status));
    HRESULT result;
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
} // namespace Microsoft::Resources::Indexers
