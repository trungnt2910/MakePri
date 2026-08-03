#include "StdAfx.h"

#include <XmlHelper.h>

namespace Microsoft::Resources::Indexers
{

CXmlHelper::CXmlHelper(IXMLDOMNode* const pDomNode) :
    _pDomNode(pDomNode),
    _pDomElement(nullptr),
    _pDomDocument(nullptr),
    _pDomDocument2(nullptr),
    _pInputXmlDomDocument3(nullptr),
    _pXmlFileName(nullptr),
    _bOwnInputXmlDomDocument3(false)
{}

CXmlHelper::CXmlHelper() : CXmlHelper(nullptr) {}

HRESULT CXmlHelper::GetCurrentNode(IXMLDOMNode** const ppNode)
{
    if (ppNode == nullptr)
    {
        return E_FAIL;
    }
    *ppNode = _pDomNode;
    return S_OK;
}

CXmlHelper::~CXmlHelper()
{
    SAFE_RELEASE(_pDomElement);
    SAFE_RELEASE(_pDomDocument);
    SAFE_RELEASE(_pDomDocument2);
    if (_bOwnInputXmlDomDocument3)
    {
        SAFE_RELEASE(_pDomNode);
        SAFE_RELEASE(_pInputXmlDomDocument3);
    }
}

HRESULT CXmlHelper::GetAttributeValue(const wchar_t* const pAttributeName, IDefStatusEx* const pStatus, wchar_t** const ppAttributeValue)
{
    HRESULT operationResult = _ElementFromNode();
    if (SUCCEEDED(operationResult))
    {
        _bstr_t attributeName(pAttributeName);
        _variant_t value;
        operationResult = _pDomElement->getAttribute(attributeName, &value);
        if (operationResult != S_OK || value.bstrVal == nullptr)
        {
            pStatus->SetError(E_DEF_XML_ATTRIB_NOT_FOUND, _pXmlFileName, 0, pAttributeName);
        }
        else
        {
            operationResult = _CreateString(value.bstrVal, ppAttributeValue);
        }
    }
    if (operationResult == S_OK)
    {
        return pStatus->GetHResult();
    }
    return operationResult;
}

HRESULT CXmlHelper::GetAttributeValueAsVariant(const wchar_t* const pAttributeName, _variant_t* const pAttributeValue)
{
    HRESULT operationResult = _ElementFromNode();
    if (SUCCEEDED(operationResult))
    {
        _bstr_t attributeName(pAttributeName);
        operationResult = _pDomElement->getAttribute(attributeName, pAttributeValue);
    }
    return operationResult;
}

HRESULT CXmlHelper::GetNodeText(IDefStatusEx* const pStatus, wchar_t** const ppNodeText)
{
    BSTR text;
    HRESULT operationResult = _pDomNode->get_text(&text);
    if (FAILED(operationResult))
    {
        _WriteParseError(nullptr, pStatus);
    }
    else
    {
        operationResult = _CreateString(text, ppNodeText);
        SysFreeString(text);
    }
    return operationResult;
}

HRESULT CXmlHelper::Init(
    const wchar_t* const pInputXmlStr,
    const CXmlHelper::INPUT_XML_STR_TYPE pInputXmlStrType,
    const wchar_t* const pInitNodeNameStr,
    IDefStatusEx* const pStatus)
{
    VARIANT inputVariant {};
    IXMLDOMNodeList* nodes = nullptr;
    VARIANT_BOOL loaded = VARIANT_FALSE;
    HRESULT result = _CreateAndInitDOM(&_pInputXmlDomDocument3);
    if (FAILED(result))
    {
        return ComputeHResult(result, pStatus);
    }
    _bOwnInputXmlDomDocument3 = true;

    if (pInputXmlStrType == INPUT_XML_STR_TYPE::XML_STR_FILE_PATH)
    {
        const std::size_t prefixLength = wcsnlen(L"\\\\?\\", MAX_PATH);
        if (wcsncmp(pInputXmlStr, L"\\\\?\\", prefixLength) == 0)
        {
            wchar_t* contents = nullptr;
            result = CUtilities::ReadUnicodeTextFile(pInputXmlStr, &contents, nullptr, 0);
            if (SUCCEEDED(result))
            {
                BSTR xml = SysAllocString(contents);
                if (xml != nullptr)
                {
                    result = _pInputXmlDomDocument3->loadXML(xml, &loaded);
                    SysFreeString(xml);
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
                operator delete(contents);
            }
        }
        else
        {
            result = _VariantFromString(pInputXmlStr, inputVariant);
            if (SUCCEEDED(result))
            {
                result = _pInputXmlDomDocument3->load(inputVariant, &loaded);
                VariantClear(&inputVariant);
            }
        }
        _pXmlFileName = pInputXmlStr;
        if (FAILED(result))
        {
            return ComputeHResult(result, pStatus);
        }
    }
    else if (pInputXmlStrType == INPUT_XML_STR_TYPE::XML_STR_BLOB)
    {
        BSTR xml = SysAllocString(pInputXmlStr);
        if (xml == nullptr)
        {
            return ComputeHResult(E_OUTOFMEMORY, pStatus);
        }
        result = _pInputXmlDomDocument3->loadXML(xml, &loaded);
        SysFreeString(xml);
        if (FAILED(result))
        {
            return ComputeHResult(result, pStatus);
        }
    }

    result = _GetNodeList(pInitNodeNameStr, pStatus, &nodes);
    if (SUCCEEDED(result))
    {
        LONG length = 0;
        nodes->get_length(&length);
        if (length <= 0)
        {
            pStatus->SetError(
                E_DEF_XML_NODE_NOT_FOUND,
                pInputXmlStrType == INPUT_XML_STR_TYPE::XML_STR_FILE_PATH ? pInputXmlStr : nullptr,
                0,
                pInitNodeNameStr);
        }
        else
        {
            nodes->get_item(0, &_pDomNode);
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(nodes));
    }
    return ComputeHResult(result, pStatus);
}

HRESULT CXmlHelper::_ElementFromNode()
{
    HRESULT result = S_OK;
    if (_pDomElement == nullptr)
    {
        result = _pDomNode->QueryInterface(IID_IXMLDOMElement, reinterpret_cast<void**>(&_pDomElement));
    }
    return result;
}

HRESULT CXmlHelper::TryGetChildNode(const wchar_t* const pNodeName, IDefStatusEx* const pStatus, IXMLDOMNode** const ppNode)
{
    HRESULT operationResult = _SetDocumentSettings(nullptr, nullptr);
    if (SUCCEEDED(operationResult))
    {
        std::wstring child(pNodeName);
        std::wstring prefix(L"child::", 7);
        const std::wstring query = prefix + child;
        BSTR xpath = SysAllocString(query.c_str());
        if (xpath != nullptr)
        {
            operationResult = _pDomNode->selectSingleNode(xpath, ppNode);
            if (operationResult != S_OK)
            {
                pStatus->SetError(E_DEF_XML_NODE_NOT_FOUND, _pXmlFileName, 0, pNodeName);
            }
            SysFreeString(xpath);
        }
        else
        {
            operationResult = E_OUTOFMEMORY;
        }
    }
    return operationResult;
}

HRESULT CXmlHelper::TryGetChildren(const wchar_t* const pNodeName, IDefStatusEx* const pStatus, IXMLDOMNodeList** const ppNodeList)
{
    HRESULT operationResult = _SetDocumentSettings(nullptr, nullptr);
    if (SUCCEEDED(operationResult))
    {
        std::wstring child(pNodeName);
        std::wstring prefix(L"child::", 7);
        const std::wstring query = prefix + child;
        BSTR xpath = SysAllocString(query.c_str());
        if (xpath != nullptr)
        {
            operationResult = _pDomNode->selectNodes(xpath, ppNodeList);
            if (FAILED(operationResult))
            {
                pStatus->SetError(E_DEF_XML_NODE_NOT_FOUND, _pXmlFileName, 0, pNodeName);
            }
            SysFreeString(xpath);
        }
        else
        {
            operationResult = E_OUTOFMEMORY;
        }
    }
    return operationResult;
}

HRESULT CXmlHelper::ValidateAgainstSchema(const wchar_t* const pSchema, IDefStatusEx* const pStatus)
{
    IXMLDOMDocument* ownerDocument = nullptr;
    IXMLDOMDocument3* schemaDocument = nullptr;
    IXMLDOMSchemaCollection* schemaCollection = nullptr;
    IXMLDOMParseError* parseError = nullptr;
    HRESULT result = _pDomNode->get_ownerDocument(&ownerDocument);
    if (SUCCEEDED(result) && ownerDocument != nullptr)
    {
        IXMLDOMDocument3* const ownerDocument3 = static_cast<IXMLDOMDocument3*>(ownerDocument);
        result = _CreateAndInitDOM(&schemaDocument);
        if (SUCCEEDED(result))
        {
            VARIANT_BOOL loaded = VARIANT_FALSE;
            _bstr_t schemaText(pSchema);
            result = schemaDocument->loadXML(schemaText, &loaded);
            if (loaded == VARIANT_TRUE)
            {
                if (SUCCEEDED(result))
                {
                    result = CoCreateInstance(
                        CLSID_XMLSchemaCache60,
                        nullptr,
                        CLSCTX_INPROC_SERVER,
                        IID_IXMLDOMSchemaCollection,
                        reinterpret_cast<void**>(&schemaCollection));
                    if (SUCCEEDED(result))
                    {
                        BSTR namespaceName = SysAllocString(L"");
                        if (namespaceName != nullptr)
                        {
                            VARIANT schemaVariant;
                            VariantInit(&schemaVariant);
                            result = _VariantFromObject(schemaDocument, schemaVariant);
                            if (SUCCEEDED(result))
                            {
                                result = schemaCollection->add(namespaceName, schemaVariant);
                                if (SUCCEEDED(result))
                                {
                                    VARIANT collectionVariant;
                                    VariantInit(&collectionVariant);
                                    result = _VariantFromObject(schemaCollection, collectionVariant);
                                    if (SUCCEEDED(result))
                                    {
                                        result = ownerDocument3->putref_schemas(collectionVariant);
                                        if (SUCCEEDED(result))
                                        {
                                            result = ownerDocument3->validateNode(_pDomNode, &parseError);
                                            if (SUCCEEDED(result))
                                            {
                                                LONG errorCode = 0;
                                                result = parseError->get_errorCode(&errorCode);
                                                if (SUCCEEDED(result))
                                                {
                                                    if (errorCode != 0)
                                                    {
                                                        _WriteSchemaErrorEvent(parseError, nullptr, pStatus);
                                                    }
                                                    else
                                                    {
                                                        result = S_OK;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    collectionVariant.pdispVal->Release();
                                }
                            }
                            schemaVariant.pdispVal->Release();
                            SysFreeString(namespaceName);
                        }
                        else
                        {
                            result = E_OUTOFMEMORY;
                        }
                    }
                }
            }
            else
            {
                result = E_FAIL;
            }
        }
    }
    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(parseError));
    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(schemaCollection));
    SAFE_RELEASE(schemaDocument);
    SAFE_RELEASE(ownerDocument);
    if (SUCCEEDED(result))
    {
        return pStatus->GetHResult();
    }
    return result;
}

HRESULT CXmlHelper::ValidateChildNodeAgainstChildSchema(
    const wchar_t* const childName,
    const wchar_t* const schema,
    const wchar_t* const schemaName,
    const wchar_t* const type,
    const bool required,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(childName);
    static_cast<void>(schemaName);
    static_cast<void>(required);
    IXMLDOMNodeList* children = nullptr;
    HRESULT result = TryGetChildren(L"indexer-config", pStatus, &children);
    if (result == S_OK)
    {
        LONG length = 0;
        result = children->get_length(&length);
        if (FAILED(result) || length <= 0)
        {
            pStatus->SetError(E_DEF_PRICONFIG_UKNOWN, _pXmlFileName, 0, nullptr);
        }
        else
        {
            for (LONG index = 0; index < length; ++index)
            {
                IXMLDOMNode* child = nullptr;
                result = children->get_item(index, &child);
                if (result != S_OK)
                {
                    pStatus->SetError(E_DEF_PRICONFIG_UKNOWN, _pXmlFileName, 0, nullptr);
                }
                else
                {
                    wchar_t* childType = nullptr;
                    CXmlHelper helper(child);
                    result = helper.GetAttributeValue(L"type", pStatus, &childType);
                    if (result != S_OK)
                    {
                        pStatus->SetError(E_DEF_PRICONFIG_MISSING_ATTRIB, _pXmlFileName, 0, L"type");
                    }
                    else if (DefString_CompareWithOptions(childType, type, DefCompare_CaseInsensitive) == Def_Equal)
                    {
                        if (pStatus->Succeeded())
                        {
                            result = helper.ValidateAgainstSchema(schema, pStatus);
                        }
                        operator delete(childType);
                        SAFE_RELEASE(child);
                        break;
                    }
                    operator delete(childType);
                }
                SAFE_RELEASE(child);
            }
        }
    }
    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    if (result == S_OK)
    {
        return pStatus->GetHResult();
    }
    return result;
}

HRESULT CXmlHelper::_CreateAndInitDOM(IXMLDOMDocument3** const ppDoc)
{
    HRESULT operationResult =
        CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument3, reinterpret_cast<void**>(ppDoc));
    if (SUCCEEDED(operationResult))
    {
        (*ppDoc)->put_async(VARIANT_FALSE);
        (*ppDoc)->put_validateOnParse(VARIANT_FALSE);
        (*ppDoc)->put_resolveExternals(VARIANT_FALSE);
        (*ppDoc)->put_preserveWhiteSpace(VARIANT_TRUE);
    }
    return operationResult;
}

HRESULT CXmlHelper::_CreateString(BSTR const pString, wchar_t** const pOutString)
{
    _bstr_t copy(pString);
    const UINT length = static_cast<BSTR>(copy) != nullptr ? SysStringLen(static_cast<BSTR>(copy)) : 0;
    *pOutString = new (std::nothrow) wchar_t[length + 1];
    if (*pOutString == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    const HRESULT operationResult = StringCchCopyW(*pOutString, length + 1, static_cast<const wchar_t*>(copy));
    if (FAILED(operationResult))
    {
        operator delete(*pOutString);
        *pOutString = nullptr;
    }
    return operationResult;
}

HRESULT CXmlHelper::_GetNodeList(const wchar_t* const pNodeName, IDefStatusEx* const pStatus, IXMLDOMNodeList** const ppNodeList)
{
    VARIANT selectionLanguage {};
    if (pNodeName == nullptr || ppNodeList == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppNodeList = nullptr;
    StringResult xpath;
    Def_HrFailed0(DefStringResult_InitRef(xpath.GetStringResult(), L"//"), pStatus);
    HRESULT operationResult = pStatus->GetHResult();
    if (pStatus->Succeeded())
    {
        Def_HrFailed0(DefStringResult_Concat(xpath.GetStringResult(), pNodeName), pStatus);
        operationResult = pStatus->GetHResult();
    }
    if (pStatus->Succeeded())
    {
        operationResult = _VariantFromString(L"XPath", selectionLanguage);
        if (SUCCEEDED(operationResult) && _pInputXmlDomDocument3 != nullptr)
        {
            operationResult = _pInputXmlDomDocument3->setProperty(const_cast<wchar_t*>(L"SelectionLanguage"), selectionLanguage);
            if (SUCCEEDED(operationResult))
            {
                BSTR query = SysAllocString(xpath.GetRef());
                if (query != nullptr)
                {
                    IXMLDOMNodeList* nodes = nullptr;
                    operationResult = _pInputXmlDomDocument3->selectNodes(query, &nodes);
                    if (operationResult != S_OK)
                    {
                        pStatus->SetError(E_DEF_XML_NODE_NOT_FOUND, _pXmlFileName, 0, pNodeName);
                    }
                    else
                    {
                        *ppNodeList = nodes;
                    }
                    SysFreeString(query);
                }
                else
                {
                    operationResult = E_OUTOFMEMORY;
                }
            }
        }
        VariantClear(&selectionLanguage);
    }
    if (operationResult == S_OK)
    {
        operationResult = pStatus->GetHResult();
    }
    return operationResult;
}

HRESULT CXmlHelper::_SetDocumentSettings(const wchar_t* const language, const wchar_t* const property)
{
    static_cast<void>(language);
    static_cast<void>(property);
    HRESULT result = S_OK;
    if (_pDomDocument2 == nullptr)
    {
        result = _pDomNode->get_ownerDocument(&_pDomDocument);
        if (SUCCEEDED(result))
        {
            result = _pDomDocument->QueryInterface(IID_IXMLDOMDocument2, reinterpret_cast<void**>(&_pDomDocument2));
        }
    }
    if (SUCCEEDED(result))
    {
        VARIANT value {};
        _VariantFromString(L"XPath", value);
        BSTR name = SysAllocString(L"SelectionLanguage");
        if (name != nullptr)
        {
            result = _pDomDocument2->setProperty(name, value);
            SysFreeString(name);
        }
        else
        {
            result = E_OUTOFMEMORY;
        }
        VariantClear(&value);
    }
    return result;
}

HRESULT CXmlHelper::_VariantFromObject(IUnknown* const value, VARIANT& result)
{
    IDispatch* dispatch = nullptr;
    VariantInit(&result);
    const HRESULT operationResult = value->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&dispatch));
    result.pdispVal = dispatch;
    result.vt = VT_DISPATCH;
    return operationResult;
}

HRESULT CXmlHelper::_VariantFromString(const wchar_t* const value, VARIANT& result)
{
    BSTR string = SysAllocString(value);
    if (string == nullptr)
    {
        return E_FAIL;
    }
    result.vt = VT_BSTR;
    result.bstrVal = string;
    return S_OK;
}

void CXmlHelper::_WriteParseError(const wchar_t* const pDesc, IDefStatusEx* const pStatus)
{
    IXMLDOMParseError* parseError = nullptr;
    _pDomDocument2->get_parseError(&parseError);
    _WriteSchemaErrorEvent(parseError, pDesc, pStatus);
    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(parseError));
}

void CXmlHelper::_WriteSchemaErrorEvent(IXMLDOMParseError* const pError, const wchar_t* const pDesc, IDefStatusEx* const pStatus)
{
    static_cast<void>(pDesc);
    if (pError != nullptr)
    {
        BSTR reason;
        LONG line;
        pError->get_reason(&reason);
        pError->get_line(&line);
        pStatus->SetError(E_DEF_XML_SCHEMA_VALIDATION_FAIL, _pXmlFileName, static_cast<std::uint32_t>(line), reason);
        SysFreeString(reason);
    }
}
void SAFE_RELEASE(IXMLDOMNode* const pNode)
{
    if (pNode != nullptr)
    {
        pNode->Release();
    }
}

} // namespace Microsoft::Resources::Indexers
