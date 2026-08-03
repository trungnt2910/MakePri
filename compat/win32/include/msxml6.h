#if __has_include_next(<msxml6.h>)
#include_next <msxml6.h>
#else

#pragma once

#include <oleauto.h>

enum DOMNodeType
{
    NODE_INVALID = 0,
    NODE_ELEMENT = 1,
    NODE_ATTRIBUTE = 2,
    NODE_TEXT = 3,
    NODE_COMMENT = 8,
    NODE_DOCUMENT = 9,
};

struct IXMLDOMDocument;
struct IXMLDOMNode;
struct IXMLDOMNodeList;
struct IXMLDOMNamedNodeMap;
struct IXMLDOMParseError;
struct ISAXContentHandler;
struct ISAXErrorHandler;
struct ISAXDTDHandler;

struct IXMLDOMNode : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE get_nodeName(BSTR* name) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_nodeValue(VARIANT* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_nodeValue(VARIANT value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_text(BSTR* text) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_text(BSTR text) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_attributes(IXMLDOMNamedNodeMap** attributes) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ownerDocument(IXMLDOMDocument** document) = 0;
    virtual HRESULT STDMETHODCALLTYPE appendChild(IXMLDOMNode* child, IXMLDOMNode** result) = 0;
    virtual HRESULT STDMETHODCALLTYPE cloneNode(VARIANT_BOOL deep, IXMLDOMNode** clone) = 0;
    virtual HRESULT STDMETHODCALLTYPE selectNodes(BSTR query, IXMLDOMNodeList** result) = 0;
    virtual HRESULT STDMETHODCALLTYPE selectSingleNode(BSTR query, IXMLDOMNode** result) = 0;
};

struct IXMLDOMNodeList : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE get_item(LONG index, IXMLDOMNode** node) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_length(LONG* length) = 0;
    virtual HRESULT STDMETHODCALLTYPE nextNode(IXMLDOMNode** node) = 0;
    virtual HRESULT STDMETHODCALLTYPE reset() = 0;
};

struct IXMLDOMNamedNodeMap : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE setNamedItem(IXMLDOMNode* node, IXMLDOMNode** replaced) = 0;
};

struct IXMLDOMElement : IXMLDOMNode
{
    virtual HRESULT STDMETHODCALLTYPE getAttribute(BSTR name, VARIANT* value) = 0;
};

struct IXMLDOMComment : IXMLDOMNode
{};

struct IXMLDOMParseError : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE get_errorCode(LONG* error) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_reason(BSTR* reason) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_line(LONG* line) = 0;
};

struct IXMLDOMDocument : IXMLDOMNode
{
    virtual HRESULT STDMETHODCALLTYPE load(VARIANT source, VARIANT_BOOL* loaded) = 0;
    virtual HRESULT STDMETHODCALLTYPE loadXML(BSTR xml, VARIANT_BOOL* loaded) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_parseError(IXMLDOMParseError** error) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_async(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_validateOnParse(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_resolveExternals(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_preserveWhiteSpace(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE createComment(BSTR text, IXMLDOMComment** comment) = 0;
};

struct IXMLDOMDocument2 : IXMLDOMDocument
{
    virtual HRESULT STDMETHODCALLTYPE createNode(VARIANT type, BSTR name, BSTR namespaceUri, IXMLDOMNode** node) = 0;
    virtual HRESULT STDMETHODCALLTYPE setProperty(BSTR name, VARIANT value) = 0;
};

struct IXMLDOMDocument3 : IXMLDOMDocument2
{
    virtual HRESULT STDMETHODCALLTYPE putref_schemas(VARIANT schemas) = 0;
    virtual HRESULT STDMETHODCALLTYPE validateNode(IXMLDOMNode* node, IXMLDOMParseError** error) = 0;
};

struct IXMLDOMSchemaCollection : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE add(BSTR namespaceUri, VARIANT schema) = 0;
};

struct IMXWriter : IDispatch
{
    virtual HRESULT STDMETHODCALLTYPE put_output(VARIANT destination) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_encoding(BSTR encoding) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_standalone(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_omitXMLDeclaration(VARIANT_BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_indent(VARIANT_BOOL value) = 0;
};

struct ISAXContentHandler : IUnknown
{};
struct ISAXErrorHandler : IUnknown
{};
struct ISAXDTDHandler : IUnknown
{};

struct ISAXXMLReader : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE putContentHandler(ISAXContentHandler* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE putErrorHandler(ISAXErrorHandler* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE putDTDHandler(ISAXDTDHandler* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE putProperty(const wchar_t* name, VARIANT value) = 0;
    virtual HRESULT STDMETHODCALLTYPE parse(VARIANT input) = 0;
};

DEFINE_GUID(CLSID_DOMDocument60, 0x88d96a05, 0xf192, 0x11d4, 0xa6, 0x5f, 0x00, 0x40, 0x96, 0x32, 0x51, 0xe5);
DEFINE_GUID(CLSID_MXXMLWriter60, 0x88d96a0f, 0xf192, 0x11d4, 0xa6, 0x5f, 0x00, 0x40, 0x96, 0x32, 0x51, 0xe5);
DEFINE_GUID(CLSID_SAXXMLReader60, 0x88d96a0c, 0xf192, 0x11d4, 0xa6, 0x5f, 0x00, 0x40, 0x96, 0x32, 0x51, 0xe5);
DEFINE_GUID(CLSID_XMLSchemaCache60, 0x88d96a07, 0xf192, 0x11d4, 0xa6, 0x5f, 0x00, 0x40, 0x96, 0x32, 0x51, 0xe5);
DEFINE_GUID(IID_IXMLDOMElement, 0x2933bf86, 0x7b36, 0x11d2, 0xb2, 0x0e, 0x00, 0xc0, 0x4f, 0x98, 0x3e, 0x60);
DEFINE_GUID(IID_IXMLDOMDocument, 0x2933bf81, 0x7b36, 0x11d2, 0xb2, 0x0e, 0x00, 0xc0, 0x4f, 0x98, 0x3e, 0x60);
DEFINE_GUID(IID_IXMLDOMDocument2, 0x2933bf95, 0x7b36, 0x11d2, 0xb2, 0x0e, 0x00, 0xc0, 0x4f, 0x98, 0x3e, 0x60);
DEFINE_GUID(IID_IXMLDOMDocument3, 0x2933bf96, 0x7b36, 0x11d2, 0xb2, 0x0e, 0x00, 0xc0, 0x4f, 0x98, 0x3e, 0x60);
DEFINE_GUID(IID_IXMLDOMSchemaCollection, 0x373984c8, 0xb845, 0x449b, 0x91, 0xe7, 0x45, 0xac, 0x83, 0x03, 0x6a, 0xde);

#endif // __has_include_next(<msxml6.h>)
