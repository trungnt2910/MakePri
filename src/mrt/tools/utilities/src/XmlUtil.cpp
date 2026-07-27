#include "StdAfx.h"

#include <XmlUtil.h>

using Microsoft::WRL::ComPtr;

HRESULT CXMLUtil::LoadXMLDataFromFile(IXMLDOMDocument2* const document, const wchar_t* const path)
{
    if (document == nullptr || path == nullptr)
    {
        return E_INVALIDARG;
    }

    VARIANT source;
    VariantInit(&source);
    source.vt = VT_BSTR;
    source.bstrVal = SysAllocString(path);

    VARIANT_BOOL loaded = VARIANT_FALSE;
    HRESULT result = document->load(source, &loaded);
    if (loaded == VARIANT_FALSE)
    {
        result = E_FAIL;
    }

    VariantClear(&source);
    return result;
}

void SanitizeData(wchar_t* const value)
{
    int index = 0;
    wchar_t character = value[0];
    if (character == L'\0')
    {
        return;
    }

    wchar_t* current = value;
    while (true)
    {
        if (character < L' ')
        {
            if (character != L'\r' && character != L'\t' && character != L'\n')
            {
                *current = L' ';
            }
        }
        else if (character >= static_cast<wchar_t>(0xFFFE))
        {
            *current = L' ';
        }

        ++index;
        current = value + index;
        character = *current;
        if (character == L'\0')
        {
            return;
        }
    }
}

HRESULT CXMLUtil::CreateXMLDocument(IXMLDOMDocument2** const document)
{
    if (document == nullptr)
    {
        return E_INVALIDARG;
    }
    *document = nullptr;

    HRESULT result = CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(document));
    if (SUCCEEDED(result))
    {
        if (*document == nullptr)
        {
            result = E_FAIL;
        }
        else
        {
            result = (*document)->put_async(VARIANT_FALSE);
            if (SUCCEEDED(result))
            {
                result = (*document)->put_validateOnParse(VARIANT_FALSE);
            }
            if (SUCCEEDED(result))
            {
                result = (*document)->put_resolveExternals(VARIANT_FALSE);
            }
            if (SUCCEEDED(result))
            {
                result = (*document)->put_preserveWhiteSpace(VARIANT_TRUE);
            }
        }
    }

    if (FAILED(result) && *document != nullptr)
    {
        (*document)->Release();
        *document = nullptr;
    }
    return result;
}

HRESULT CXMLUtil::WriteXmlToFile(IXMLDOMDocument2* const document, const wchar_t* const path)
{
    if (document == nullptr || path == nullptr)
    {
        return E_INVALIDARG;
    }

    ComPtr<IStream> stream;
    HRESULT result = SHCreateStreamOnFileW(path, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, &stream);
    if (SUCCEEDED(result))
    {
        result = FormatAndWriteDOMDocument(document, stream.Get(), XmlEncoding::Utf8);
    }
    return result;
}

HRESULT CXMLUtil::FormatAndWriteDOMDocument(IXMLDOMDocument2* const document, IStream* const stream, XmlEncoding encoding)
{
    static_cast<void>(encoding);

    ComPtr<IMXWriter> writer;
    HRESULT result = CoCreateInstance(CLSID_MXXMLWriter60, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&writer));

    ComPtr<ISAXContentHandler> contentHandler;
    if (SUCCEEDED(result))
    {
        result = writer.As(&contentHandler);
    }

    ComPtr<ISAXErrorHandler> errorHandler;
    if (SUCCEEDED(result))
    {
        result = writer.As(&errorHandler);
    }

    ComPtr<ISAXDTDHandler> dtdHandler;
    if (SUCCEEDED(result))
    {
        result = writer.As(&dtdHandler);
    }

    if (SUCCEEDED(result))
    {
        result = writer->put_omitXMLDeclaration(VARIANT_FALSE);
    }
    if (SUCCEEDED(result))
    {
        result = writer->put_standalone(VARIANT_TRUE);
    }
    if (SUCCEEDED(result))
    {
        result = writer->put_indent(VARIANT_TRUE);
    }
    if (SUCCEEDED(result))
    {
        result = writer->put_encoding(const_cast<wchar_t*>(L"UTF-8"));
    }

    ComPtr<ISAXXMLReader> reader;
    if (SUCCEEDED(result))
    {
        result = CoCreateInstance(CLSID_SAXXMLReader60, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&reader));
    }
    if (SUCCEEDED(result))
    {
        result = reader->putContentHandler(contentHandler.Get());
    }
    if (SUCCEEDED(result))
    {
        result = reader->putErrorHandler(errorHandler.Get());
    }
    if (SUCCEEDED(result))
    {
        result = reader->putDTDHandler(dtdHandler.Get());
    }

    VARIANT writerVariant;
    VariantInit(&writerVariant);
    writerVariant.vt = VT_UNKNOWN;
    writerVariant.punkVal = writer.Get();
    if (SUCCEEDED(result))
    {
        result = reader->putProperty(L"http://xml.org/sax/properties/lexical-handler", writerVariant);
    }
    if (SUCCEEDED(result))
    {
        result = reader->putProperty(L"http://xml.org/sax/properties/declaration-handler", writerVariant);
    }

    VARIANT documentVariant;
    VariantInit(&documentVariant);
    documentVariant.vt = VT_UNKNOWN;
    documentVariant.punkVal = document;

    VARIANT streamVariant;
    VariantInit(&streamVariant);
    streamVariant.vt = VT_UNKNOWN;
    streamVariant.punkVal = stream;

    if (SUCCEEDED(result))
    {
        result = writer->put_output(streamVariant);
    }
    if (SUCCEEDED(result))
    {
        result = reader->parse(documentVariant);
    }
    return result;
}

HRESULT CXMLUtil::AddComment(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const wchar_t* const value,
    IXMLDOMNode** const comment)
{
    if (document == nullptr || value == nullptr || comment == nullptr)
    {
        return E_INVALIDARG;
    }

    BSTR text = SysAllocString(value);
    HRESULT result = S_OK;
    if (text != nullptr)
    {
        result = document->createComment(text, reinterpret_cast<IXMLDOMComment**>(comment));
        if (SUCCEEDED(result))
        {
            IXMLDOMNode* const destination = parent == nullptr ? document : parent;
            result = destination->appendChild(*comment, nullptr);
        }
        if (FAILED(result) && *comment != nullptr)
        {
            (*comment)->Release();
            *comment = nullptr;
        }
        SysFreeString(text);
    }
    return result;
}

HRESULT CXMLUtil::AddElement(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const parent,
    const wchar_t* const name,
    IXMLDOMNode** const element)
{
    if (document == nullptr || name == nullptr || element == nullptr)
    {
        return E_INVALIDARG;
    }

    HRESULT result = CreateNewNode(document, name, NODE_ELEMENT, element);
    if (SUCCEEDED(result))
    {
        IXMLDOMNode* const destination = parent == nullptr ? document : parent;
        result = destination->appendChild(*element, nullptr);
    }

    if (FAILED(result) && *element != nullptr)
    {
        (*element)->Release();
        *element = nullptr;
    }
    return result;
}

HRESULT CXMLUtil::CreateNewNode(
    IXMLDOMDocument2* const document,
    const wchar_t* const name,
    const DOMNodeType type,
    IXMLDOMNode** const node)
{
    if (document == nullptr || node == nullptr)
    {
        return E_INVALIDARG;
    }
    *node = nullptr;

    VARIANT nodeType;
    VariantInit(&nodeType);
    nodeType.vt = VT_I4;
    nodeType.lVal = type;

    BSTR nodeName = SysAllocString(name);
    HRESULT result;
    if (nodeName != nullptr)
    {
        result = document->createNode(nodeType, nodeName, nullptr, node);
        SysFreeString(nodeName);
    }
    else
    {
        result = E_OUTOFMEMORY;
    }

    VariantClear(&nodeType);
    return result;
}

HRESULT CXMLUtil::AddAttribute(IXMLDOMDocument2* const document, IXMLDOMNode* const node, const wchar_t* const name, VARIANT& value)
{
    if (document == nullptr || node == nullptr)
    {
        return E_INVALIDARG;
    }

    IXMLDOMNode* attribute = nullptr;
    ComPtr<IXMLDOMNamedNodeMap> attributes;
    HRESULT result = CreateNewNode(document, name, NODE_ATTRIBUTE, &attribute);
    if (SUCCEEDED(result))
    {
        result = attribute->put_nodeValue(value);
    }
    if (SUCCEEDED(result))
    {
        result = node->get_attributes(&attributes);
    }
    if (SUCCEEDED(result))
    {
        if (attributes != nullptr)
        {
            result = attributes->setNamedItem(attribute, nullptr);
        }
        else
        {
            result = E_FAIL;
        }
    }

    CleanupNode(&attribute);
    return result;
}

HRESULT CXMLUtil::AddAttribute(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const node,
    const wchar_t* const name,
    const std::uint32_t value)
{
    VARIANT variant;
    VariantInit(&variant);
    variant.vt = VT_I4;
    variant.lVal = static_cast<LONG>(value);
    const HRESULT result = AddAttribute(document, node, name, variant);
    VariantClear(&variant);
    return result;
}

HRESULT CXMLUtil::AddAttribute(
    IXMLDOMDocument2* const document,
    IXMLDOMNode* const node,
    const wchar_t* const name,
    const wchar_t* const value)
{
    if (value == nullptr)
    {
        return E_INVALIDARG;
    }

    VARIANT variant;
    VariantInit(&variant);
    variant.vt = VT_BSTR;
    variant.bstrVal = SysAllocString(value);

    HRESULT result;
    if (variant.bstrVal != nullptr)
    {
        result = AddAttribute(document, node, name, variant);
    }
    else
    {
        result = E_OUTOFMEMORY;
    }
    VariantClear(&variant);
    return result;
}

HRESULT CXMLUtil::SetElementValue(IXMLDOMNode* const node, const wchar_t* const value, const XmlUtilFlags flags)
{
    if (node == nullptr || value == nullptr)
    {
        return E_INVALIDARG;
    }

    BSTR text = SysAllocString(value);
    if (text == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    if ((static_cast<int>(flags) & static_cast<int>(XmlUtilFlags::XmlUtil_SanitizeStrings)) != 0)
    {
        SanitizeData(text);
    }

    const HRESULT result = node->put_text(text);
    SysFreeString(text);
    return result;
}

void CXMLUtil::CleanupNode(IXMLDOMNode** const node)
{
    if (node != nullptr && *node != nullptr)
    {
        (*node)->Release();
        *node = nullptr;
    }
}

HRESULT CXMLUtil::GetSingleNodeValue(IXMLDOMDocument2* const document, const wchar_t* xpath, wchar_t** const value)
{
    static_cast<void>(xpath);
    if (document == nullptr || value == nullptr)
    {
        return E_INVALIDARG;
    }

    VARIANT selectionLanguage;
    VariantInit(&selectionLanguage);
    selectionLanguage.vt = VT_BSTR;
    selectionLanguage.bstrVal = SysAllocString(L"XPath");
    if (selectionLanguage.bstrVal == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT result = document->setProperty(const_cast<wchar_t*>(L"SelectionLanguage"), selectionLanguage);
    VariantClear(&selectionLanguage);

    IXMLDOMNode* node = nullptr;
    if (SUCCEEDED(result))
    {
        BSTR query = SysAllocString(L"*[local-name()='Package']/*[local-name()='Identity']/@Name");
        result = document->selectSingleNode(query, &node);
        SysFreeString(query);
    }

    VARIANT nodeValue;
    VariantInit(&nodeValue);
    if (SUCCEEDED(result))
    {
        if (result == S_FALSE)
        {
            result = E_INVALIDARG;
        }
        else
        {
            result = node->get_nodeValue(&nodeValue);
            if (SUCCEEDED(result))
            {
                const std::uint32_t length = SysStringLen(nodeValue.bstrVal) + 1;
                *value = static_cast<wchar_t*>(operator new(length * sizeof(wchar_t)));
                if (*value != nullptr)
                {
                    result = StringCchCopyW(*value, length, nodeValue.bstrVal);
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
            }
        }
    }

    if (FAILED(result) && *value != nullptr)
    {
        operator delete(*value);
        *value = nullptr;
    }
    VariantClear(&nodeValue);
    CleanupNode(&node);
    return result;
}
