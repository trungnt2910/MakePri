#include "StdAfx.h"

#include <CResxXmlConfig.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* CResxXmlConfig::s_xmlConfigSchema =
    LR"xml(<xs:schema id="resw" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigResxType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((r|R)(e|E)(s|S)(w|W))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute  name="type"  type="IndexerConfigResxType" use="required"/>)xml"
                LR"xml(<xs:attribute  name="convertDotsToSlashes"  type="xs:boolean" use="required"/>)xml"
                LR"xml(<xs:attribute  name="initialPath"  type="xs:string" use="optional"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

CResxXmlConfig::CResxXmlConfig(IXMLDOMNode* const node) : m_domNode(node)
{
    m_resxConfigXmlHelper = new (std::nothrow) CXmlHelper(m_domNode);
    DefStringResult_SetCopy(m_initialPath.GetStringResult(), LOCALE_NAME_INVARIANT);
}

CResxXmlConfig::~CResxXmlConfig()
{
    if (m_resxConfigXmlHelper != nullptr)
    {
        delete m_resxConfigXmlHelper;
    }
    m_resxConfigXmlHelper = nullptr;
}

bool CResxXmlConfig::GetConvertDotsToSlashesFlag() { return m_convertDotsToSlashes; }

const wchar_t* CResxXmlConfig::GetInitialPath() { return m_initialPath.GetRef(); }

HRESULT CResxXmlConfig::Parse(IDefStatusEx* const status)
{
    HRESULT result = S_OK;
    std::uint32_t length = 0;
    IXMLDOMNode* child = nullptr;
    IXMLDOMNodeList* children = nullptr;
    _variant_t convertDotsToSlashes;
    _variant_t reservedVariant;
    wchar_t* type = nullptr;
    bool found = false;

    if (m_resxConfigXmlHelper != nullptr)
    {
        result = m_resxConfigXmlHelper->ValidateChildNodeAgainstChildSchema(
            L"indexer-config", s_xmlConfigSchema, L"type", L"RESW", false, status);
        if (SUCCEEDED(result))
        {
            m_resxConfigXmlHelper->TryGetChildren(L"indexer-config", status, &children);
            children->get_length(reinterpret_cast<long*>(&length));
            for (int index = 0; index < static_cast<int>(length) && !found && SUCCEEDED(result); ++index)
            {
                result = children->get_item(index, &child);
                if (SUCCEEDED(result) && child != nullptr)
                {
                    CXmlHelper* helper = new (std::nothrow) CXmlHelper(child);
                    if (helper != nullptr)
                    {
                        helper->GetAttributeValue(L"type", status, &type);
                        if (DefString_CompareWithOptions(L"RESW", type, DefCompare_CaseInsensitive) == 0)
                        {
                            helper->GetAttributeValueAsVariant(L"convertDotsToSlashes", &convertDotsToSlashes);
                            m_convertDotsToSlashes = static_cast<bool>(convertDotsToSlashes);

                            wchar_t* initialPath = nullptr;
                            result = helper->GetAttributeValue(L"initialPath", status, &initialPath);
                            if (status->GetWhat() == E_DEF_XML_ATTRIB_NOT_FOUND)
                            {
                                status->Reset();
                                result = S_OK;
                            }
                            else if (SUCCEEDED(result))
                            {
                                PathRemoveBackslashW(initialPath);
                                Def_HrFailed0(DefStringResult_SetCopy(m_initialPath.GetStringResult(), initialPath), status);
                            }
                            operator delete(initialPath);
                            found = true;
                        }
                        operator delete(type);
                        delete helper;
                        type = nullptr;
                    }
                    SAFE_RELEASE(child);
                }
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        }
    }
    return ComputeHResult(result, status);
}
} // namespace Microsoft::Resources::Indexers
