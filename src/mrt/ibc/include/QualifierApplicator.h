#pragma once

#include <mrm/BaseInternal.h>
#include <mrm/Collections.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/Checksums.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/platform/base.h>
#include <mrm/Results.h>
#include <DefStatus.h>
#include <mrm/readers/MrmManagers.h>
#include <MrtMap.h>

#include <list>
#include <map>
#include <set>
#include <string>

struct IXMLDOMNode;

namespace Microsoft::Resources
{

class MrmProfile;
class StringResult;
struct ResourceQualifier;

namespace Build
{

class DecisionInfoBuilder;
class DecisionInfoQualifierSetBuilder;

} // namespace Build
} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{

class CHIndexerBase;

class IbcQualifier : public IQualifier
{
public:
    IbcQualifier(const wchar_t* value, const ResourceQualifier* qualifier) : m_value(value), m_qualifier(qualifier) {}

    virtual ~IbcQualifier();

    [[nodiscard]] int GetPriority() const override;
    HRESULT GetFallbackScore(double* score) const override;
    [[nodiscard]] int GetFallbackScoreAsScaledInt() const override;
    HRESULT GetQualifierIndex(int* index) const override;
    HRESULT GetBaseQualifierIndex(int* index) const override;
    HRESULT GetOperand1Attribute(Atom* result) const override;
    [[nodiscard]] bool OperatorIsCustom() const override;
    HRESULT GetOperator(ConditionOperator* result) const override;
    HRESULT GetCustomOperator(Atom* result) const override;
    [[nodiscard]] bool OperatorIsUnary() const override;
    HRESULT Operand2IsLiteral(bool* result) const override;
    HRESULT GetOperand2Literal(StringResult* result) const override;
    HRESULT GetOperand2Attribute(Atom* result) const override;

private:
    const wchar_t* m_value;
    const ResourceQualifier* m_qualifier;
};

class CQualifierApplicator
{
public:
    enum tagTOKEN_TYPE
    {
        tokenDefault = 0,
        tokenFile = 1,
        tokenFolder = 2,
    };

    struct _CaseInsensitiveWStringCompare
    {
        bool operator()(const std::wstring& left, const std::wstring& right);
    };

    class CQualifierSetBuilder
    {
    public:
        ~CQualifierSetBuilder();

        HRESULT _AddQualifier(
            const wchar_t* name,
            const wchar_t* value,
            double* fallbackScore,
            int* priority,
            tagTOKEN_TYPE tokenType,
            bool* applied,
            IDefStatusEx* status);

    private:
        friend class CQualifierApplicator;

        std::int32_t _parentQualifierSetIndex;
        Build::DecisionInfoQualifierSetBuilder* _pQualifierSetBuilder;
        CQualifierApplicator* _pQualifierApplicator;
    };

    CQualifierApplicator(
        const IXMLDOMNode* configurationNode,
        const MrmProfile* profile,
        const UnifiedEnvironment* environment,
        Build::DecisionInfoBuilder* decisionInfoBuilder,
        const std::map<std::wstring, std::set<std::wstring>>* allowedQualifierMap);
    CQualifierApplicator(const MrmProfile* profile, const UnifiedEnvironment* environment);
    ~CQualifierApplicator();

    HRESULT GetQualifierSetBuilder(int qualifierSetIndex, IDefStatusEx* status, CQualifierSetBuilder** result);
    HRESULT ApplyQualifierSetFromBuilder(CQualifierSetBuilder* builder, IDefStatusEx* status, int* qualifierSetIndex);
    HRESULT ApplyQualifier(
        const wchar_t* token,
        int qualifierSetIndex,
        tagTOKEN_TYPE tokenType,
        int* resultQualifierSetIndex,
        bool* applied,
        IDefStatusEx* status);
    HRESULT AddUltFallbackAttrValuePair(const wchar_t* name, const wchar_t* value, const wchar_t* source, IDefStatusEx* status);
    static HRESULT ValidateAllowedQualiferMapSet(
        const MrmProfile* profile,
        const UnifiedEnvironment* environment,
        const std::map<std::wstring, std::set<std::wstring>>* allowedQualifierMap,
        IDefStatusEx* status);
    HRESULT ValidateUltFallbackQualifiers(IDefStatusEx* status);
    HRESULT
    GetDefaultQualifierValues(const wchar_t* source, IDefStatusEx* status, StringResult* result);
    HRESULT GetQualifierMapFromToken(const wchar_t* token, Runtime::MrtMap<const wchar_t*, const wchar_t*>* map, IDefStatusEx* status);
    static HRESULT GetQualifierNameFromNameOrToken(
        const wchar_t* nameOrToken,
        const MrmProfile* profile,
        const UnifiedEnvironment* environment,
        IDefStatusEx* status,
        StringResult* result);

private:
    HRESULT _ParseToken(const wchar_t* token, tagTOKEN_TYPE tokenType, std::multimap<std::wstring, std::wstring>* values) const;
    HRESULT _AddAttributeValuePairToMap(
        const wchar_t* name,
        const wchar_t* value,
        const wchar_t* token,
        tagTOKEN_TYPE tokenType,
        std::multimap<std::wstring, std::wstring>* values) const;
    HRESULT _ValidateAttributeValueMap(
        std::multimap<std::wstring, std::wstring>* values,
        const wchar_t* token,
        tagTOKEN_TYPE tokenType,
        bool* valid,
        IDefStatusEx* status) const;
    HRESULT
    _ValidateAndAddUltFallbackPair(const wchar_t* name, const wchar_t* value, IDefStatusEx* status);
    void _GetAttrNameValueToAdd(
        const wchar_t* name,
        const wchar_t* value,
        const wchar_t* token,
        tagTOKEN_TYPE tokenType,
        const wchar_t** nameToAdd,
        const wchar_t** valueToAdd) const;
    HRESULT _EvaluteQalifierType(ResourceQualifier* qualifier, const wchar_t* value, const wchar_t* fallbackValue, double* score) const;
    HRESULT _IsUltFallbackQualifier(
        std::multimap<std::wstring, std::wstring>::iterator& value,
        bool* isUltimateFallback,
        double* fallbackScore,
        IDefStatusEx* status) const;
    HRESULT _AddQualifierToQSB(
        std::multimap<std::wstring, std::wstring>* values,
        Build::DecisionInfoQualifierSetBuilder* builder,
        double* fallbackScore,
        int* priority,
        bool* applied,
        IDefStatusEx* status) const;
    HRESULT _GetOrAddQualifierSet(
        Build::DecisionInfoQualifierSetBuilder* builder,
        int qualifierSetIndex,
        int* resultQualifierSetIndex,
        IDefStatusEx* status);

    const MrmProfile* _c_pProfile;
    const UnifiedEnvironment* _c_pUnifiedEnvironment;
    Build::DecisionInfoBuilder* _pDecisionInfoBuilder;
    [[maybe_unused]] const IXMLDOMNode* _c_pIndexPassNode;
    const std::map<std::wstring, std::set<std::wstring>>* _pAllowedQualifierValues;
    std::int32_t _neutralQualifierSetIndex;
    std::map<std::wstring, std::wstring> _AttributeValueMapStore;
    std::multimap<ResourceQualifier*, std::wstring> _UltFallbackAttrValueMap;
    [[maybe_unused]] _CaseInsensitiveWStringCompare _wstrCompareFunctor;
    std::list<std::wstring> _UltFallbackAttrNameList;
};

} // namespace Microsoft::Resources::Indexers
