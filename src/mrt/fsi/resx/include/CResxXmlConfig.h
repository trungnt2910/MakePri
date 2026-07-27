#pragma once

#include <IFsIndexer.h>
#include <mrm/BaseInternal.h>
#include <mrm/Results.h>

#include <windows.h>
#include <msxml6.h>

#include <string>

namespace Microsoft::Resources
{
class IDefStatusEx;
}

namespace Microsoft::Resources::Indexers
{
class CXmlHelper;

class CResxXmlConfig : public IFsiConfigHelper
{
public:
    explicit CResxXmlConfig(IXMLDOMNode* node);
    ~CResxXmlConfig();

    HRESULT Parse(IDefStatusEx* status) override;
    bool GetConvertDotsToSlashesFlag();
    bool IsFilenameAsCollection() { return m_fileNameAsCollection; }
    const wchar_t* GetInitialPath();

private:
    static const wchar_t* s_xmlConfigSchema;

    IXMLDOMNode* m_domNode {};
    CXmlHelper* m_resxConfigXmlHelper {};
    bool m_convertDotsToSlashes {true};
    bool m_fileNameAsCollection {true};
    std::wstring m_collectionName;
    StringResult m_initialPath;
};
} // namespace Microsoft::Resources::Indexers
