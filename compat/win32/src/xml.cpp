#include <algorithm>
#include <array>
#include <fstream>
#include <format>
#include <limits>
#include <new>
#include <string>
#include <vector>

#include <msxml6.h>
#include <objidlbase.h>

#include <libxml/parser.h>
#include <libxml/xmlsave.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <uni_algo/conv.h>

#include "internal/com.h"
#include "internal/errors.h"
#include "internal/strings.h"

namespace
{
struct ElementStack
{
    std::vector<std::string> names;
};

void StartElement(void* const context, const xmlChar* const name, const xmlChar**)
{
    if (context != nullptr && name != nullptr)
    {
        static_cast<ElementStack*>(context)->names.emplace_back(reinterpret_cast<const char*>(name));
    }
}

void EndElement(void* const context, const xmlChar* const name)
{
    if (context == nullptr || name == nullptr)
    {
        return;
    }
    auto& names = static_cast<ElementStack*>(context)->names;
    const std::string_view closing(reinterpret_cast<const char*>(name));
    const auto match = std::find(names.rbegin(), names.rend(), closing);
    if (match != names.rend())
    {
        names.erase(std::next(match).base(), names.end());
    }
}

std::vector<std::string> FindUnclosedTags(const std::string_view input)
{
    if (input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        return {};
    }

    ElementStack elements;
    xmlSAXHandler handler {};
    handler.startElement = StartElement;
    handler.endElement = EndElement;
    xmlParserCtxtPtr const parser = xmlCreatePushParserCtxt(&handler, &elements, nullptr, 0, nullptr);
    if (parser == nullptr)
    {
        return {};
    }
    xmlCtxtUseOptions(parser, XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    xmlParseChunk(parser, input.data(), static_cast<int>(input.size()), 1);
    xmlFreeParserCtxt(parser);
    return std::move(elements.names);
}

BSTR ToBstr(const xmlChar* const value)
{
    if (value == nullptr)
        return nullptr;
    const std::u16string converted = una::utf8to16<char, char16_t>(reinterpret_cast<const char*>(value));
    return SysAllocStringLen(reinterpret_cast<const wchar_t*>(converted.data()), static_cast<UINT>(converted.size()));
}

class XmlDocument;

class ParseError final : public IXMLDOMParseError, private win32_compat::ComReferenceCounted
{
public:
    ParseError(const LONG code, std::u16string reason, const LONG line) : code_(code), reason_(std::move(reason)), line_(line) {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override
    {
        if (object == nullptr)
            return E_POINTER;
        *object = static_cast<IXMLDOMParseError*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT get_errorCode(LONG* error) override
    {
        if (!error)
            return E_POINTER;
        *error = code_;
        return S_OK;
    }
    HRESULT get_reason(BSTR* reason) override
    {
        if (!reason)
            return E_POINTER;
        *reason = SysAllocStringLen(reinterpret_cast<const wchar_t*>(reason_.data()), static_cast<UINT>(reason_.size()));
        return *reason ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT get_line(LONG* line) override
    {
        if (!line)
            return E_POINTER;
        *line = line_;
        return S_OK;
    }

private:
    LONG code_;
    std::u16string reason_;
    LONG line_;
};

class XmlNode;

class NodeList final : public IXMLDOMNodeList, private win32_compat::ComReferenceCounted
{
public:
    NodeList(XmlDocument* document, std::vector<xmlNodePtr> nodes);
    ~NodeList() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override;
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT get_item(LONG index, IXMLDOMNode** node) override;
    HRESULT get_length(LONG* length) override;
    HRESULT nextNode(IXMLDOMNode** node) override { return get_item(static_cast<LONG>(cursor_++), node); }
    HRESULT reset() override
    {
        cursor_ = 0;
        return S_OK;
    }

private:
    XmlDocument* document_;
    std::vector<xmlNodePtr> nodes_;
    std::size_t cursor_ = 0;
};

class NamedNodeMap final : public IXMLDOMNamedNodeMap, private win32_compat::ComReferenceCounted
{
public:
    explicit NamedNodeMap(XmlNode* owner);
    ~NamedNodeMap() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override;
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT setNamedItem(IXMLDOMNode* node, IXMLDOMNode** replaced) override;

private:
    XmlNode* owner_;
};

class XmlNode final : public IXMLDOMElement, private win32_compat::ComReferenceCounted
{
public:
    XmlNode(XmlDocument* document, xmlNodePtr node, DOMNodeType pendingType = NODE_INVALID, std::u16string pendingName = {});
    ~XmlNode() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override;
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT get_nodeName(BSTR* name) override;
    HRESULT get_nodeValue(VARIANT* value) override;
    HRESULT put_nodeValue(VARIANT value) override;
    HRESULT get_text(BSTR* text) override;
    HRESULT put_text(BSTR text) override;
    HRESULT get_attributes(IXMLDOMNamedNodeMap** attributes) override;
    HRESULT get_ownerDocument(IXMLDOMDocument** document) override;
    HRESULT appendChild(IXMLDOMNode* child, IXMLDOMNode** result) override;
    HRESULT cloneNode(VARIANT_BOOL deep, IXMLDOMNode** clone) override;
    HRESULT selectNodes(BSTR query, IXMLDOMNodeList** result) override;
    HRESULT selectSingleNode(BSTR query, IXMLDOMNode** result) override;
    HRESULT getAttribute(BSTR name, VARIANT* value) override;
    xmlNodePtr Raw() const { return node_; }
    bool IsPendingAttribute() const { return node_ == nullptr && pendingType_ == NODE_ATTRIBUTE; }
    const std::u16string& PendingName() const { return pendingName_; }
    const std::u16string& PendingValue() const { return pendingValue_; }

private:
    XmlDocument* document_;
    xmlNodePtr node_;
    DOMNodeType pendingType_;
    std::u16string pendingName_;
    std::u16string pendingValue_;
};

class XmlDocument final : public IXMLDOMDocument3, private win32_compat::ComReferenceCounted
{
public:
    XmlDocument() : document_(xmlNewDoc(BAD_CAST "1.0")) {}
    ~XmlDocument() override
    {
        if (document_)
            xmlFreeDoc(document_);
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override
    {
        if (!object)
            return E_POINTER;
        *object = static_cast<IXMLDOMDocument3*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT get_nodeName(BSTR* name) override
    {
        if (!name)
            return E_POINTER;
        *name = SysAllocString(L"#document");
        return *name ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT get_nodeValue(VARIANT* value) override
    {
        if (!value)
            return E_POINTER;
        VariantInit(value);
        return S_FALSE;
    }
    HRESULT put_nodeValue(VARIANT) override { return E_FAIL; }
    HRESULT get_text(BSTR* text) override;
    HRESULT put_text(BSTR) override { return E_FAIL; }
    HRESULT get_attributes(IXMLDOMNamedNodeMap** attributes) override
    {
        if (!attributes)
            return E_POINTER;
        *attributes = nullptr;
        return S_FALSE;
    }
    HRESULT get_ownerDocument(IXMLDOMDocument** document) override;
    HRESULT appendChild(IXMLDOMNode* child, IXMLDOMNode** result) override;
    HRESULT cloneNode(VARIANT_BOOL, IXMLDOMNode**) override { return E_NOTIMPL; }
    HRESULT selectNodes(BSTR query, IXMLDOMNodeList** result) override { return Select(nullptr, query, result); }
    HRESULT selectSingleNode(BSTR query, IXMLDOMNode** result) override;
    HRESULT load(VARIANT source, VARIANT_BOOL* loaded) override;
    HRESULT loadXML(BSTR xml, VARIANT_BOOL* loaded) override;
    HRESULT get_parseError(IXMLDOMParseError** error) override;
    HRESULT put_async(VARIANT_BOOL) override { return S_OK; }
    HRESULT put_validateOnParse(VARIANT_BOOL) override { return S_OK; }
    HRESULT put_resolveExternals(VARIANT_BOOL) override { return S_OK; }
    HRESULT put_preserveWhiteSpace(VARIANT_BOOL) override { return S_OK; }
    HRESULT createComment(BSTR text, IXMLDOMComment** comment) override;
    HRESULT createNode(VARIANT type, BSTR name, BSTR, IXMLDOMNode** node) override;
    HRESULT setProperty(BSTR, VARIANT) override { return S_OK; }
    HRESULT putref_schemas(VARIANT) override { return S_OK; }
    HRESULT validateNode(IXMLDOMNode*, IXMLDOMParseError** error) override;
    HRESULT Select(xmlNodePtr context, BSTR query, IXMLDOMNodeList** result);
    xmlDocPtr Raw() const { return document_; }
    void Replace(xmlDocPtr document)
    {
        if (document_)
            xmlFreeDoc(document_);
        document_ = document;
    }
    void RecordError(std::string_view contents);

private:
    xmlDocPtr document_;
    LONG errorCode_ = 0;
    LONG errorLine_ = 0;
    std::u16string errorReason_;
};

class XmlWriter final : public IMXWriter, private win32_compat::ComReferenceCounted
{
public:
    ~XmlWriter() override
    {
        if (stream_)
            stream_->Release();
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override
    {
        if (!object)
            return E_POINTER;
        *object = static_cast<IMXWriter*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT put_output(VARIANT destination) override
    {
        if (destination.vt != VT_UNKNOWN || !destination.punkVal)
            return E_INVALIDARG;
        IStream* stream = nullptr;
        const HRESULT result = destination.punkVal->QueryInterface(IID_IStream, reinterpret_cast<void**>(&stream));
        if (FAILED(result))
            return result;
        if (stream_)
            stream_->Release();
        stream_ = stream;
        return S_OK;
    }
    HRESULT put_encoding(BSTR) override { return S_OK; }
    HRESULT put_standalone(VARIANT_BOOL) override { return S_OK; }
    HRESULT put_omitXMLDeclaration(VARIANT_BOOL) override { return S_OK; }
    HRESULT put_indent(VARIANT_BOOL) override { return S_OK; }
    HRESULT Write(XmlDocument* document)
    {
        if (!stream_ || !document)
            return E_FAIL;
        document->Raw()->standalone = 1;
        xmlBufferPtr buffer = xmlBufferCreate();
        if (!buffer)
            return E_OUTOFMEMORY;
        xmlSaveCtxtPtr save = xmlSaveToBuffer(buffer, "UTF-8", XML_SAVE_FORMAT | XML_SAVE_INDENT);
        if (!save)
        {
            xmlBufferFree(buffer);
            return E_OUTOFMEMORY;
        }
        xmlSaveSetIndentString(save, "\t");
        const int saveStatus = xmlSaveDoc(save, document->Raw());
        const int closeStatus = xmlSaveClose(save);
        std::size_t size = xmlBufferLength(buffer);
        const xmlChar* const memory = xmlBufferContent(buffer);
        if (size > 0 && memory[size - 1] == '\n')
            --size;
        ULONG written = 0;
        const HRESULT result = saveStatus >= 0 && closeStatus >= 0 && size <= static_cast<std::size_t>(std::numeric_limits<ULONG>::max()) ?
                                   stream_->Write(memory, static_cast<ULONG>(size), &written) :
                                   E_FAIL;
        xmlBufferFree(buffer);
        return SUCCEEDED(result) && written != size ? STG_E_WRITEFAULT : result;
    }

private:
    IStream* stream_ = nullptr;
};

class XmlReader final : public ISAXXMLReader, private win32_compat::ComReferenceCounted
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override
    {
        if (!object)
            return E_POINTER;
        *object = static_cast<ISAXXMLReader*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT putContentHandler(ISAXContentHandler* handler) override
    {
        writer_ = reinterpret_cast<XmlWriter*>(handler);
        return S_OK;
    }
    HRESULT putErrorHandler(ISAXErrorHandler*) override { return S_OK; }
    HRESULT putDTDHandler(ISAXDTDHandler*) override { return S_OK; }
    HRESULT putProperty(const wchar_t*, VARIANT) override { return S_OK; }
    HRESULT parse(VARIANT input) override
    {
        if (input.vt != VT_UNKNOWN || !input.punkVal || !writer_)
            return E_INVALIDARG;
        return writer_->Write(reinterpret_cast<XmlDocument*>(input.punkVal));
    }

private:
    XmlWriter* writer_ = nullptr;
};

class SchemaCollection final : public IXMLDOMSchemaCollection, private win32_compat::ComReferenceCounted
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** object) override
    {
        if (!object)
            return E_POINTER;
        *object = static_cast<IXMLDOMSchemaCollection*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT add(BSTR, VARIANT) override { return S_OK; }
};

NodeList::NodeList(XmlDocument* const document, std::vector<xmlNodePtr> nodes) : document_(document), nodes_(std::move(nodes))
{
    document_->AddRef();
}
NodeList::~NodeList() { document_->Release(); }
HRESULT NodeList::QueryInterface(REFIID, void** const object)
{
    if (!object)
        return E_POINTER;
    *object = static_cast<IXMLDOMNodeList*>(this);
    AddRef();
    return S_OK;
}
HRESULT NodeList::get_item(const LONG index, IXMLDOMNode** const node)
{
    if (!node)
        return E_POINTER;
    *node = nullptr;
    if (index < 0 || static_cast<std::size_t>(index) >= nodes_.size())
        return S_FALSE;
    *node = new (std::nothrow) XmlNode(document_, nodes_[static_cast<std::size_t>(index)]);
    return *node ? S_OK : E_OUTOFMEMORY;
}
HRESULT NodeList::get_length(LONG* const length)
{
    if (!length)
        return E_POINTER;
    *length = static_cast<LONG>(nodes_.size());
    return S_OK;
}

NamedNodeMap::NamedNodeMap(XmlNode* const owner) : owner_(owner) { owner_->AddRef(); }
NamedNodeMap::~NamedNodeMap() { owner_->Release(); }
HRESULT NamedNodeMap::QueryInterface(REFIID, void** const object)
{
    if (!object)
        return E_POINTER;
    *object = static_cast<IXMLDOMNamedNodeMap*>(this);
    AddRef();
    return S_OK;
}
HRESULT NamedNodeMap::setNamedItem(IXMLDOMNode* const node, IXMLDOMNode** const replaced)
{
    if (replaced)
        *replaced = nullptr;
    auto* const attribute = static_cast<XmlNode*>(node);
    if (!attribute || !attribute->IsPendingAttribute() || !owner_->Raw())
        return E_INVALIDARG;
    const std::string name = una::utf16to8<char16_t, char>(attribute->PendingName());
    const std::string value = una::utf16to8<char16_t, char>(attribute->PendingValue());
    return xmlSetProp(owner_->Raw(), BAD_CAST name.c_str(), BAD_CAST value.c_str()) ? S_OK : E_FAIL;
}

XmlNode::XmlNode(XmlDocument* const document, const xmlNodePtr node, const DOMNodeType pendingType, std::u16string pendingName) :
    document_(document), node_(node), pendingType_(pendingType), pendingName_(std::move(pendingName))
{
    document_->AddRef();
}
XmlNode::~XmlNode() { document_->Release(); }
HRESULT XmlNode::QueryInterface(REFIID, void** const object)
{
    if (!object)
        return E_POINTER;
    *object = static_cast<IXMLDOMElement*>(this);
    AddRef();
    return S_OK;
}
HRESULT XmlNode::get_nodeName(BSTR* const name)
{
    if (!name)
        return E_POINTER;
    if (IsPendingAttribute())
        *name = SysAllocStringLen(reinterpret_cast<const wchar_t*>(pendingName_.data()), static_cast<UINT>(pendingName_.size()));
    else
        *name = node_ ? ToBstr(node_->name) : nullptr;
    return *name ? S_OK : E_FAIL;
}
HRESULT XmlNode::get_nodeValue(VARIANT* const value)
{
    if (!value)
        return E_POINTER;
    VariantInit(value);
    if (IsPendingAttribute())
    {
        value->vt = VT_BSTR;
        value->bstrVal = SysAllocStringLen(reinterpret_cast<const wchar_t*>(pendingValue_.data()), static_cast<UINT>(pendingValue_.size()));
        return value->bstrVal ? S_OK : E_OUTOFMEMORY;
    }
    if (!node_ || node_->type != XML_ATTRIBUTE_NODE)
        return S_FALSE;
    xmlChar* content = xmlNodeGetContent(node_);
    value->vt = VT_BSTR;
    value->bstrVal = ToBstr(content);
    xmlFree(content);
    return value->bstrVal ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlNode::put_nodeValue(const VARIANT value)
{
    if (!IsPendingAttribute())
        return E_FAIL;
    if (value.vt == VT_BSTR)
        pendingValue_.assign(win32_compat::WideView(value.bstrVal));
    else if (value.vt == VT_I4)
    {
        pendingValue_ = una::utf8to16<char, char16_t>(std::format("{}", value.lVal));
    }
    else
        return E_INVALIDARG;
    return S_OK;
}
HRESULT XmlNode::get_text(BSTR* const text)
{
    if (!text || !node_)
        return E_POINTER;
    xmlChar* content = xmlNodeGetContent(node_);
    *text = ToBstr(content);
    xmlFree(content);
    return *text ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlNode::put_text(BSTR const text)
{
    if (!node_)
        return E_FAIL;
    const std::string content = una::utf16to8<char16_t, char>(win32_compat::WideView(text));
    xmlNodeSetContent(node_, BAD_CAST content.c_str());
    return S_OK;
}
HRESULT XmlNode::get_attributes(IXMLDOMNamedNodeMap** const attributes)
{
    if (!attributes)
        return E_POINTER;
    *attributes = new (std::nothrow) NamedNodeMap(this);
    return *attributes ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlNode::get_ownerDocument(IXMLDOMDocument** const document)
{
    if (!document)
        return E_POINTER;
    document_->AddRef();
    *document = document_;
    return S_OK;
}
HRESULT XmlNode::appendChild(IXMLDOMNode* const child, IXMLDOMNode** const result)
{
    if (!node_ || !child)
        return E_INVALIDARG;
    auto* const xmlChild = static_cast<XmlNode*>(child);
    if (!xmlChild->Raw() || !xmlAddChild(node_, xmlChild->Raw()))
        return E_FAIL;
    if (result)
    {
        child->AddRef();
        *result = child;
    }
    return S_OK;
}
HRESULT XmlNode::cloneNode(const VARIANT_BOOL deep, IXMLDOMNode** const clone)
{
    if (!clone || !node_)
        return E_POINTER;
    xmlNodePtr copy = xmlDocCopyNode(node_, document_->Raw(), deep == VARIANT_TRUE ? 1 : 0);
    *clone = copy ? new (std::nothrow) XmlNode(document_, copy) : nullptr;
    return *clone ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlNode::selectNodes(BSTR const query, IXMLDOMNodeList** const result) { return document_->Select(node_, query, result); }
HRESULT XmlNode::selectSingleNode(BSTR const query, IXMLDOMNode** const result)
{
    if (!result)
        return E_POINTER;
    *result = nullptr;
    IXMLDOMNodeList* list = nullptr;
    const HRESULT status = selectNodes(query, &list);
    if (FAILED(status))
        return status;
    const HRESULT itemStatus = list->get_item(0, result);
    list->Release();
    return itemStatus;
}
HRESULT XmlNode::getAttribute(BSTR const name, VARIANT* const value)
{
    if (!value || !node_)
        return E_POINTER;
    VariantInit(value);
    const std::string attributeName = una::utf16to8<char16_t, char>(win32_compat::WideView(name));
    xmlChar* content = xmlGetProp(node_, BAD_CAST attributeName.c_str());
    if (!content)
        return S_FALSE;
    value->vt = VT_BSTR;
    value->bstrVal = ToBstr(content);
    xmlFree(content);
    return value->bstrVal ? S_OK : E_OUTOFMEMORY;
}

HRESULT XmlDocument::get_text(BSTR* const text)
{
    if (!text)
        return E_POINTER;
    xmlNodePtr root = xmlDocGetRootElement(document_);
    if (!root)
    {
        *text = SysAllocString(L"");
        return S_OK;
    }
    xmlChar* content = xmlNodeGetContent(root);
    *text = ToBstr(content);
    xmlFree(content);
    return *text ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlDocument::get_ownerDocument(IXMLDOMDocument** const document)
{
    if (!document)
        return E_POINTER;
    AddRef();
    *document = this;
    return S_OK;
}
HRESULT XmlDocument::appendChild(IXMLDOMNode* const child, IXMLDOMNode** const result)
{
    if (!child)
        return E_INVALIDARG;
    auto* const xmlChild = static_cast<XmlNode*>(child);
    if (!xmlChild->Raw())
        return E_INVALIDARG;
    if (xmlChild->Raw()->type == XML_ELEMENT_NODE)
        xmlDocSetRootElement(document_, xmlChild->Raw());
    else
        xmlAddChild(reinterpret_cast<xmlNodePtr>(document_), xmlChild->Raw());
    if (result)
    {
        child->AddRef();
        *result = child;
    }
    return S_OK;
}
HRESULT XmlDocument::selectSingleNode(BSTR const query, IXMLDOMNode** const result)
{
    if (!result)
        return E_POINTER;
    *result = nullptr;
    IXMLDOMNodeList* list = nullptr;
    const HRESULT status = Select(nullptr, query, &list);
    if (FAILED(status))
        return status;
    const HRESULT itemStatus = list->get_item(0, result);
    list->Release();
    return itemStatus;
}
void XmlDocument::RecordError(const std::string_view contents)
{
    const xmlError* error = xmlGetLastError();
    errorCode_ = error ? error->code : 1;
    errorLine_ = error ? error->line : 0;
    switch (errorCode_)
    {
    case XML_ERR_DOCUMENT_EMPTY:
        errorReason_ = u"XML document must have a top level element.\r\n";
        break;
    case XML_ERR_INVALID_CHAR:
    case XML_ERR_INVALID_ENCODING:
    case XML_ERR_NAME_REQUIRED:
        errorReason_ = u"An invalid character was found in text content.\r\n";
        break;
    case XML_ERR_ATTRIBUTE_REDEFINED:
        errorReason_ = u"Duplicate attribute.\r\n";
        break;
    case XML_ERR_TAG_NOT_FINISHED:
    {
        const std::vector<std::string> tags = FindUnclosedTags(contents);
        if (tags.empty())
        {
            errorReason_ = u"A tag was not closed.\r\n";
            break;
        }
        errorReason_ = u"The following tags were not closed: ";
        for (std::size_t index = 0; index < tags.size(); ++index)
        {
            if (index != 0)
                errorReason_.append(u", ");
            errorReason_.append(una::utf8to16<char, char16_t>(tags[index]));
        }
        errorReason_.append(u".\r\n");
        break;
    }
    default:
        errorReason_ = error && error->message ? una::utf8to16<char, char16_t>(error->message) : u"XML parse error";
        while (!errorReason_.empty() && (errorReason_.back() == u'\n' || errorReason_.back() == u'\r'))
            errorReason_.pop_back();
        errorReason_.append(u"\r\n");
        break;
    }
    win32_compat::SetErrorDescription(errorReason_);
}
HRESULT XmlDocument::load(const VARIANT source, VARIANT_BOOL* const loaded)
{
    if (!loaded || source.vt != VT_BSTR || !source.bstrVal)
        return E_INVALIDARG;
    *loaded = VARIANT_FALSE;
    const std::filesystem::path path = win32_compat::ToPath(source.bstrVal);
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream)
    {
        errorCode_ = ERROR_FILE_NOT_FOUND;
        errorReason_ = u"File not found";
        return E_FAIL;
    }
    const auto end = stream.tellg();
    if (end < 0)
        return E_FAIL;
    std::string contents(static_cast<std::size_t>(end), '\0');
    stream.seekg(0);
    stream.read(contents.data(), end);
    xmlResetLastError();
    xmlDocPtr parsed = xmlReadMemory(
        contents.data(),
        static_cast<int>(contents.size()),
        path.string().c_str(),
        nullptr,
        XML_PARSE_NONET | XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!parsed)
    {
        RecordError(contents);
        return S_OK;
    }
    if (parsed->intSubset != nullptr || parsed->extSubset != nullptr)
    {
        xmlFreeDoc(parsed);
        errorCode_ = 1;
        errorLine_ = 0;
        errorReason_ = u"DTD is prohibited.\r\n";
        win32_compat::SetErrorDescription(errorReason_);
        return S_OK;
    }
    Replace(parsed);
    errorCode_ = 0;
    errorReason_.clear();
    *loaded = VARIANT_TRUE;
    return S_OK;
}
HRESULT XmlDocument::loadXML(BSTR const xml, VARIANT_BOOL* const loaded)
{
    if (!loaded || !xml)
        return E_INVALIDARG;
    *loaded = VARIANT_FALSE;
    const std::string contents = una::utf16to8<char16_t, char>(win32_compat::WideView(xml));
    if (contents.starts_with("\xef\xbb\xbf"))
    {
        errorCode_ = XML_ERR_DOCUMENT_EMPTY;
        errorLine_ = 0;
        errorReason_ = u"XML document must have a top level element.\r\n";
        win32_compat::SetErrorDescription(errorReason_);
        return S_OK;
    }
    xmlResetLastError();
    xmlDocPtr parsed = xmlReadMemory(
        contents.data(),
        static_cast<int>(contents.size()),
        nullptr,
        "UTF-8",
        XML_PARSE_NONET | XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!parsed)
    {
        RecordError(contents);
        return S_OK;
    }
    if (parsed->intSubset != nullptr || parsed->extSubset != nullptr)
    {
        xmlFreeDoc(parsed);
        errorCode_ = 1;
        errorLine_ = 0;
        errorReason_ = u"DTD is prohibited.\r\n";
        win32_compat::SetErrorDescription(errorReason_);
        return S_OK;
    }
    Replace(parsed);
    errorCode_ = 0;
    errorReason_.clear();
    *loaded = VARIANT_TRUE;
    return S_OK;
}
HRESULT XmlDocument::get_parseError(IXMLDOMParseError** const error)
{
    if (!error)
        return E_POINTER;
    *error = new (std::nothrow) ParseError(errorCode_, errorReason_, errorLine_);
    return *error ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlDocument::createComment(BSTR const text, IXMLDOMComment** const comment)
{
    if (!comment)
        return E_POINTER;
    const std::string content = una::utf16to8<char16_t, char>(win32_compat::WideView(text));
    xmlNodePtr node = xmlNewComment(BAD_CAST content.c_str());
    *comment = reinterpret_cast<IXMLDOMComment*>(node ? new (std::nothrow) XmlNode(this, node) : nullptr);
    return *comment ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlDocument::createNode(const VARIANT type, BSTR const name, BSTR, IXMLDOMNode** const node)
{
    if (!node || type.vt != VT_I4 || !name)
        return E_INVALIDARG;
    *node = nullptr;
    const auto nodeType = static_cast<DOMNodeType>(type.lVal);
    const std::u16string wideName(win32_compat::WideView(name));
    if (nodeType == NODE_ATTRIBUTE)
        *node = new (std::nothrow) XmlNode(this, nullptr, NODE_ATTRIBUTE, wideName);
    else if (nodeType == NODE_ELEMENT)
    {
        const std::string narrowName = una::utf16to8<char16_t, char>(wideName);
        xmlNodePtr raw = xmlNewNode(nullptr, BAD_CAST narrowName.c_str());
        *node = raw ? new (std::nothrow) XmlNode(this, raw) : nullptr;
    }
    return *node ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlDocument::validateNode(IXMLDOMNode*, IXMLDOMParseError** const error)
{
    if (!error)
        return E_POINTER;
    *error = new (std::nothrow) ParseError(0, {}, 0);
    return *error ? S_OK : E_OUTOFMEMORY;
}
HRESULT XmlDocument::Select(xmlNodePtr const context, BSTR const query, IXMLDOMNodeList** const result)
{
    if (!query || !result)
        return E_INVALIDARG;
    *result = nullptr;
    xmlXPathContextPtr xpathContext = xmlXPathNewContext(document_);
    if (!xpathContext)
        return E_OUTOFMEMORY;
    xpathContext->node = context ? context : reinterpret_cast<xmlNodePtr>(document_);
    const std::string expression = una::utf16to8<char16_t, char>(win32_compat::WideView(query));
    xmlXPathObjectPtr object = xmlXPathEvalExpression(BAD_CAST expression.c_str(), xpathContext);
    std::vector<xmlNodePtr> nodes;
    if (object && object->type == XPATH_NODESET && object->nodesetval)
        for (int index = 0; index < object->nodesetval->nodeNr; ++index)
            nodes.push_back(object->nodesetval->nodeTab[index]);
    if (object)
        xmlXPathFreeObject(object);
    xmlXPathFreeContext(xpathContext);
    *result = new (std::nothrow) NodeList(this, std::move(nodes));
    return *result ? S_OK : E_OUTOFMEMORY;
}

template<typename Implementation>
HRESULT CreateInstance(REFIID interfaceId, void** const object)
{
    if (object == nullptr)
    {
        return E_POINTER;
    }
    *object = nullptr;
    Implementation* const instance = new (std::nothrow) Implementation();
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    const HRESULT result = instance->QueryInterface(interfaceId, object);
    instance->Release();
    return result;
}

const std::array XmlClasses {
    win32_compat::ComClassRegistration {&CLSID_DOMDocument60, CreateInstance<XmlDocument>},
    win32_compat::ComClassRegistration {&CLSID_MXXMLWriter60, CreateInstance<XmlWriter>},
    win32_compat::ComClassRegistration {&CLSID_SAXXMLReader60, CreateInstance<XmlReader>},
    win32_compat::ComClassRegistration {&CLSID_XMLSchemaCache60, CreateInstance<SchemaCollection>},
};

[[maybe_unused]] const bool XmlClassesRegistered = [] {
    win32_compat::RegisterComClasses(XmlClasses);
    return true;
}();
} // namespace
