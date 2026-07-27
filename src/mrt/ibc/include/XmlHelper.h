#pragma once

#include <ParameterParser.h>

#include <comutil.h>
#include <msxml6.h>
#include <windows.h>

#include <cstdint>

namespace Microsoft::Resources
{

class IDefStatusEx;

}

namespace Microsoft::Resources::Indexers
{

class CBootStrapIndexer;

void SAFE_RELEASE(IXMLDOMNode* pNode);

class CXmlHelper
{
public:
    enum INPUT_XML_STR_TYPE
    {
        XML_STR_FILE_PATH = 0,
        XML_STR_BLOB = 1,
    };

    explicit CXmlHelper(IXMLDOMNode* pDomNode);
    CXmlHelper();
    ~CXmlHelper();

    HRESULT GetCurrentNode(IXMLDOMNode** ppNode);
    HRESULT Init(const wchar_t* pInputXmlStr, INPUT_XML_STR_TYPE pInputXmlStrType, const wchar_t* pInitNodeNameStr, IDefStatusEx* pStatus);
    HRESULT GetNodeText(IDefStatusEx* pStatus, wchar_t** ppNodeText);
    HRESULT TryGetChildNode(const wchar_t* pNodeName, IDefStatusEx* pStatus, IXMLDOMNode** ppNode);
    HRESULT TryGetChildren(const wchar_t* pNodeName, IDefStatusEx* pStatus, IXMLDOMNodeList** ppNodeList);
    HRESULT ValidateAgainstSchema(const wchar_t* pSchema, IDefStatusEx* pStatus);
    HRESULT GetAttributeValueAsVariant(const wchar_t* pAttributeName, _variant_t* pAttributeValue);
    HRESULT GetAttributeValue(const wchar_t* pAttributeName, IDefStatusEx* pStatus, wchar_t** ppAttributeValue);
    HRESULT ValidateChildNodeAgainstChildSchema(
        const wchar_t* childName,
        const wchar_t* schema,
        const wchar_t* schemaName,
        const wchar_t* type,
        bool required,
        IDefStatusEx* pStatus);

private:
    friend class CBootStrapIndexer;

    HRESULT _ElementFromNode();
    HRESULT _CreateAndInitDOM(IXMLDOMDocument3** ppDoc);
    void _WriteParseError(const wchar_t* pDesc, IDefStatusEx* pStatus);
    void _WriteSchemaErrorEvent(IXMLDOMParseError* pError, const wchar_t* pDesc, IDefStatusEx* pStatus);
    HRESULT _SetDocumentSettings(const wchar_t* language, const wchar_t* property);
    HRESULT _CreateString(BSTR pString, wchar_t** pOutString);
    HRESULT _VariantFromString(const wchar_t* value, VARIANT& result);
    HRESULT _VariantFromObject(IUnknown* value, VARIANT& result);
    HRESULT _GetNodeList(const wchar_t* pNodeName, IDefStatusEx* pStatus, IXMLDOMNodeList** ppNodeList);

    IXMLDOMNode* _pDomNode;
    IXMLDOMElement* _pDomElement;
    IXMLDOMDocument* _pDomDocument;
    IXMLDOMDocument2* _pDomDocument2;
    IXMLDOMDocument3* _pInputXmlDomDocument3;
    const wchar_t* _pXmlFileName;
    std::uint64_t _EventsHandle;
    bool _bOwnInputXmlDomDocument3;
};

} // namespace Microsoft::Resources::Indexers
