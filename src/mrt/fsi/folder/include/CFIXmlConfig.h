#pragma once

#include <cstdint>

#include <IFsIndexer.h>
#include <mrm/BaseInternal.h>

#include <windows.h>
#include <msxml6.h>

#include <map>
#include <string>
#include <vector>

namespace Microsoft::Resources
{
class IDefStatusEx;
}

namespace Microsoft::Resources::Indexers
{
class CXmlHelper;

enum class EXCLUDE_TYPE_FLAG : std::uint32_t
{
    NONE = 0,
    PATH = 1,
    NAME = 2,
    EXTENSION = 4,
    TREE = 8,
};

struct CExclusionResult
{
    bool doNotTraverse {};
    bool doNotIndex {};
};

class CFIXmlConfig : public IFsiConfigHelper
{
public:
    explicit CFIXmlConfig(IXMLDOMNode* pDomNode);
    ~CFIXmlConfig();

    HRESULT Parse(IDefStatusEx* pStatus) override;

    HRESULT IsExcludedFolder(const wchar_t* pFolderPath, const wchar_t* pFolder, CExclusionResult* pResult);
    HRESULT IsExcludedFile(const wchar_t* pFolderPath, const wchar_t* pFile, CExclusionResult* pResult);
    bool IsFoldernameAsDimension();
    bool IsFilenameAsDimension();
    const wchar_t* GetQualifierDelimiter();

private:
    static const wchar_t* s_pFISchema;

    struct EXCLUDE_ATTRIBUTES
    {
        EXCLUDE_TYPE_FLAG type {};
        CExclusionResult result {};
        std::uint16_t padding {};
    };

    HRESULT _IsExcludedFolderOrFile(const std::wstring& wszFolderOrFile, EXCLUDE_TYPE_FLAG iSearchExcludeType, CExclusionResult* pResult);
    HRESULT _ProcessExcludeNode(IXMLDOMNode* pExcludeNode, IDefStatusEx* pStatus);
    HRESULT _ProcessIndexerConfigNode(IXMLDOMNode* pIndexerConfig, IDefStatusEx* pStatus);
    HRESULT _IsValidValue(
        const std::wstring& wszValue,
        EXCLUDE_TYPE_FLAG iSearchExcludeType,
        EXCLUDE_ATTRIBUTES attribute,
        IDefStatusEx* pStatus);

    std::map<std::wstring, std::vector<EXCLUDE_ATTRIBUTES>*> _excludeTypeMap;
    CXmlHelper* _pFolderConfigXmlHelper {};
    IXMLDOMNode* _pDomNode {};
    bool _bFoldernameAsQualifier {};
    bool _bFilenameAsQualifier {};
    StringResult _strQualifierDelimiter;
};
} // namespace Microsoft::Resources::Indexers
