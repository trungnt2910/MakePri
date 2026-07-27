#pragma once

#include <cstdint>

#include <mrm/common/BaseInternal.h>
#include <mrm/DefObject.h>
#include <mrm/Results.h>
#include <mrm/Atoms.h>
#include <mrm/readers/MrmReaders.h>
#include <XmlHelper.h>
#include <DefStatus.h>

#include <windows.h>

struct IXMLDOMNode;

namespace Microsoft::Resources
{

class IDefStatusEx;

namespace Indexers
{

template<typename T>
class DynamicArray;

class CPriSchemaReader
{
public:
    CPriSchemaReader();
    ~CPriSchemaReader();

    HRESULT InitializeFromFile(const wchar_t* path, IDefStatusEx* status);

    [[nodiscard]] const IHierarchicalSchema* GetSchema() const { return _pSchema; }

private:
    static const wchar_t* s_pszPriInfoNodeName;
    static const wchar_t* s_pszResourceMapNodeName;
    static const wchar_t* s_pszResourceMapNameAttribute;
    static const wchar_t* s_pszResourceMapUniqueNameAttribute;
    static const wchar_t* s_pszResourceMapVersionAttribute;
    static const wchar_t* s_pszSubtreeNodeName;
    static const wchar_t* s_pszSubtreeNameAttribute;
    static const wchar_t* s_pszSubtreeIndexAttribute;
    static const wchar_t* s_pszResourceNodeName;
    static const wchar_t* s_pszResourceNameAttribute;
    static const wchar_t* s_pszResourceIndexAttribute;

    HRESULT ParsePriSchemaFile(const wchar_t* input, CXmlHelper::INPUT_XML_STR_TYPE inputType, IDefStatusEx* status);
    HRESULT ParseVersionString(const wchar_t* version, IDefStatusEx* status);
    HRESULT ParseResourceMapNode(IXMLDOMNode* node, IDefStatusEx* status);
    HRESULT
    ParseResourceMapSubtreeNode(const wchar_t* parentName, IXMLDOMNode* node, IDefStatusEx* status);
    HRESULT
    ParseNamedResourceNode(const wchar_t* parentName, IXMLDOMNode* node, IDefStatusEx* status);
    HRESULT ValidateSchemaData(IDefStatusEx* status);
    HRESULT BuildSchema(IDefStatusEx* status);

    StringResult _simpleName;
    StringResult _uniqueName;
    std::uint16_t _majorVersion {};
    std::uint16_t _minorVersion {};
    std::uint8_t* _pSchemaBlob {};
    HierarchicalSchema* _pSchema {};
    DynamicArray<wchar_t*>* _pSubtrees {};
    DynamicArray<wchar_t*>* _pResources {};
};

class PathSchemaCollection final : public ISchemaCollection
{
public:
    PathSchemaCollection() = default;
    ~PathSchemaCollection() override;

    [[nodiscard]] int GetNumSchemas() const override;
    HRESULT GetSchema(int index, const IHierarchicalSchema** schema) const override;
    HRESULT GetPrimarySchema(const IHierarchicalSchema** schema) const override;
    HRESULT GetSchemaById(const wchar_t* id, const IHierarchicalSchema** schema) const override;
    HRESULT FindSchema(const HierarchicalSchemaReference* reference, const IHierarchicalSchema** schema) const override;

    [[nodiscard]] bool Initialize(const wchar_t* path, IDefStatusEx* status);

private:
    void* m_profile {};
    void* m_standalonePriFile {};
    void* m_priSchemaReader {};
    void* m_schemaWrapper {};
    void* m_schemaCollection {};
};

} // namespace Indexers
} // namespace Microsoft::Resources
