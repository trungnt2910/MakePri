#pragma once

#include <cstdint>

#include <windows.h>
#include <msxml6.h>

void SanitizeData(wchar_t* value);

class CXMLUtil
{
public:
    enum class XmlEncoding
    {
        Utf8,
    };

    enum class XmlUtilFlags
    {
        XmlUtil_SanitizeStrings = 1,
        XmlUtil_PreserveStrings = 0,
        XmlUtil_SanitizationMask = 1,
        XmlUtil_DefaultFlags = 0,
    };

    static HRESULT CreateXMLDocument(IXMLDOMDocument2** document);
    static HRESULT LoadXMLDataFromFile(IXMLDOMDocument2* document, const wchar_t* path);
    static HRESULT GetSingleNodeValue(IXMLDOMDocument2* document, const wchar_t* xpath, wchar_t** value);
    static HRESULT WriteXmlToFile(IXMLDOMDocument2* document, const wchar_t* path);
    static HRESULT AddComment(IXMLDOMDocument2* document, IXMLDOMNode* parent, const wchar_t* value, IXMLDOMNode** comment);
    static HRESULT AddElement(IXMLDOMDocument2* document, IXMLDOMNode* parent, const wchar_t* name, IXMLDOMNode** element);
    static HRESULT AddAttribute(IXMLDOMDocument2* document, IXMLDOMNode* node, const wchar_t* name, VARIANT& value);
    static HRESULT AddAttribute(IXMLDOMDocument2* document, IXMLDOMNode* node, const wchar_t* name, std::uint32_t value);
    static HRESULT AddAttribute(IXMLDOMDocument2* document, IXMLDOMNode* node, const wchar_t* name, const wchar_t* value);
    static HRESULT SetElementValue(IXMLDOMNode* node, const wchar_t* value, XmlUtilFlags flags);
    static void CleanupNode(IXMLDOMNode** node);

private:
    static HRESULT FormatAndWriteDOMDocument(IXMLDOMDocument2* document, IStream* stream, XmlEncoding encoding);
    static HRESULT CreateNewNode(IXMLDOMDocument2* document, const wchar_t* name, DOMNodeType type, IXMLDOMNode** node);
};
