#pragma once

#include <cstdint>

#include <windows.h>

struct IXMLDOMNode;

namespace Microsoft::Resources
{
class IDefStatusEx;
class UnifiedEnvironment;
} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{
class CItemInstanceEntry;
class CItemInstanceSink;
class CQualifierApplicator;
class IIndexOptions;

class IFsiConfigHelper
{
public:
    virtual HRESULT Parse(IDefStatusEx* status) = 0;
};

static_assert(sizeof(IFsiConfigHelper) == sizeof(void*));

class IFormatSpecificIndexer
{
public:
    virtual ~IFormatSpecificIndexer() = default;

    virtual HRESULT Init(
        const UnifiedEnvironment* environment,
        const wchar_t* projectRoot,
        IXMLDOMNode* configuration,
        CQualifierApplicator* qualifierApplicator,
        const IIndexOptions* options,
        IDefStatusEx* status) = 0;
    virtual HRESULT Process(CItemInstanceEntry* entry, CItemInstanceSink* sink, IDefStatusEx* status, bool* handled) = 0;
};

enum FSIList_Group : std::int32_t
{
    FSI_MainGroup = 0,
    FSI_FinalGroup = 1,
};

class CFsiFactory
{
public:
    static IFormatSpecificIndexer* WINAPI s_GetIndexer(const wchar_t* type, IDefStatusEx* status, FSIList_Group* group);
};
} // namespace Microsoft::Resources::Indexers
