#include "StdAfx.h"

#include <QualifierApplicator.h>

namespace Microsoft::Resources::Indexers
{
namespace
{

constexpr const wchar_t* LanguageQualifierName = L"Language";

} // namespace

CQualifierApplicator::CQualifierApplicator(
    const IXMLDOMNode* const configurationNode,
    const MrmProfile* const profile,
    const UnifiedEnvironment* const environment,
    Build::DecisionInfoBuilder* const decisionInfoBuilder,
    const std::map<std::wstring, std::set<std::wstring>>* const allowedQualifierMap) :
    _c_pProfile(profile),
    _c_pUnifiedEnvironment(environment),
    _pDecisionInfoBuilder(decisionInfoBuilder),
    _c_pIndexPassNode(configurationNode),
    _pAllowedQualifierValues(nullptr)
{
    if (environment == nullptr || decisionInfoBuilder == nullptr)
    {
        throw E_INVALIDARG;
    }

    DefStatus status;
    _neutralQualifierSetIndex = 0;
    if (allowedQualifierMap != nullptr && !allowedQualifierMap->empty())
    {
        _pAllowedQualifierValues = allowedQualifierMap;
    }
}

CQualifierApplicator::CQualifierApplicator(const MrmProfile* const profile, const UnifiedEnvironment* const environment) :
    _c_pProfile(profile), _c_pUnifiedEnvironment(environment), _pDecisionInfoBuilder(nullptr), _c_pIndexPassNode(nullptr)
{
    if (environment == nullptr)
    {
        throw E_INVALIDARG;
    }
}

CQualifierApplicator::~CQualifierApplicator()
{
    for (const auto& [qualifier, value] : _UltFallbackAttrValueMap)
    {
        static_cast<void>(value);
        operator delete(qualifier);
    }
    _UltFallbackAttrValueMap.clear();
    _UltFallbackAttrNameList.clear();
}

IbcQualifier::~IbcQualifier() = default;

HRESULT CQualifierApplicator::ApplyQualifier(
    const wchar_t* const token,
    const int qualifierSetIndex,
    const tagTOKEN_TYPE tokenType,
    int* const resultQualifierSetIndex,
    bool* const applied,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (token == nullptr || resultQualifierSetIndex == nullptr || applied == nullptr)
    {
        status->DiagnosticLogWithErrorCodeA(__FUNCTION__, E_INVALIDARG);
        return E_INVALIDARG;
    }

    std::multimap<std::wstring, std::wstring> AttributeValueMap;
    HRESULT result = _ParseToken(token, tokenType, &AttributeValueMap);
    if (SUCCEEDED(result) && !AttributeValueMap.empty())
    {
        result = _ValidateAttributeValueMap(&AttributeValueMap, token, tokenType, applied, status);
        if (SUCCEEDED(result))
        {
            if (*applied)
            {
                Build::DecisionInfoQualifierSetBuilder* builder = nullptr;
                Def_HrFailed0(Build::DecisionInfoQualifierSetBuilder::CreateInstance(_pDecisionInfoBuilder, &builder), status);
                if (builder != nullptr)
                {
                    bool actuallyApplied = false;
                    result = _AddQualifierToQSB(&AttributeValueMap, builder, nullptr, nullptr, &actuallyApplied, status);
                    if (SUCCEEDED(result))
                    {
                        result = _GetOrAddQualifierSet(builder, qualifierSetIndex, resultQualifierSetIndex, status);
                    }
                    *applied = actuallyApplied;
                    delete builder;
                }
            }
        }
    }
    else
    {
        *applied = false;
    }

    if (!*applied && SUCCEEDED(result))
    {
        *resultQualifierSetIndex = qualifierSetIndex <= -1 ? _neutralQualifierSetIndex : qualifierSetIndex;
    }

    const HRESULT computedResult = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return computedResult;
}

HRESULT CQualifierApplicator::ApplyQualifierSetFromBuilder(
    CQualifierSetBuilder* const builder,
    IDefStatusEx* const status,
    int* const qualifierSetIndex)
{
    if (builder != nullptr && qualifierSetIndex != nullptr)
    {
        return _GetOrAddQualifierSet(builder->_pQualifierSetBuilder, builder->_parentQualifierSetIndex, qualifierSetIndex, status);
    }
    return E_INVALIDARG;
}

HRESULT IbcQualifier::GetBaseQualifierIndex(int* const index) const
{
    *index = m_qualifier->name.GetIndex();
    return S_OK;
}

HRESULT IbcQualifier::GetCustomOperator(Atom* const result) const
{
    *result = Atom::NullAtom;
    return S_OK;
}

HRESULT IbcQualifier::GetFallbackScore(double* const score) const
{
    *score = 0.0;
    return S_OK;
}

int IbcQualifier::GetFallbackScoreAsScaledInt() const { return 0; }

HRESULT IbcQualifier::GetOperand1Attribute(Atom* const result) const
{
    *result = m_qualifier->name;
    return S_OK;
}

HRESULT IbcQualifier::GetOperand2Attribute(Atom* const result) const
{
    *result = Atom::NullAtom;
    return S_OK;
}

HRESULT IbcQualifier::GetOperand2Literal(StringResult* const result) const { return result->SetRef(m_value); }

HRESULT IbcQualifier::GetOperator(ConditionOperator* const result) const
{
    *result = static_cast<ConditionOperator>(10);
    return S_OK;
}

int IbcQualifier::GetPriority() const { return m_qualifier->defaultBuildPriority; }

HRESULT IbcQualifier::GetQualifierIndex(int* const index) const
{
    *index = -1;
    return S_OK;
}

bool IbcQualifier::OperatorIsCustom() const { return false; }

bool IbcQualifier::OperatorIsUnary() const { return false; }

HRESULT IbcQualifier::Operand2IsLiteral(bool* const result) const
{
    *result = m_value != nullptr;
    return S_OK;
}

HRESULT CQualifierApplicator::GetQualifierNameFromNameOrToken(
    const wchar_t* const nameOrToken,
    const MrmProfile* const profile,
    const UnifiedEnvironment* const environment,
    IDefStatusEx* const status,
    StringResult* const result)
{
    Atom qualifierNameAtom {};
    const IEnvironment* qualifierEnvironment = nullptr;
    if (!Def_HrFailed0(environment->GetQualifierNameAtom(nameOrToken, &qualifierNameAtom, &qualifierEnvironment), status))
    {
        Def_HrFailed0(DefStringResult_SetCopy(result->GetStringResult(), nameOrToken), status);
    }
    else
    {
        status->Reset();
        QualifierBuildInfo buildInfo {};
        if (!Def_HrFailed0(profile->GetQualifierBuildInfoByToken(nameOrToken, environment, &buildInfo), status))
        {
            Def_HrFailed0(
                environment->GetName(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, buildInfo.qualifier.name, result), status);
        }
    }
    return status->GetHResult();
}

HRESULT CQualifierApplicator::GetQualifierSetBuilder(
    const int qualifierSetIndex,
    IDefStatusEx* const status,
    CQualifierSetBuilder** const result)
{
    if (result == nullptr)
    {
        return E_INVALIDARG;
    }

    Build::DecisionInfoQualifierSetBuilder* builder = nullptr;
    Def_HrFailed0(Build::DecisionInfoQualifierSetBuilder::CreateInstance(_pDecisionInfoBuilder, &builder), status);
    if (builder != nullptr)
    {
        CQualifierSetBuilder* const wrapper = new CQualifierSetBuilder;
        wrapper->_parentQualifierSetIndex = qualifierSetIndex;
        wrapper->_pQualifierSetBuilder = builder;
        wrapper->_pQualifierApplicator = this;
        *result = wrapper;
    }
    return status->GetHResult();
}

HRESULT CQualifierApplicator::ValidateAllowedQualiferMapSet(
    const MrmProfile* const profile,
    const UnifiedEnvironment* const environment,
    const std::map<std::wstring, std::set<std::wstring>>* const allowedQualifierMap,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (profile == nullptr || environment == nullptr || allowedQualifierMap == nullptr)
    {
        return E_INVALIDARG;
    }

    bool stop = false;
    for (auto allowedQualifier = allowedQualifierMap->cbegin(); allowedQualifier != allowedQualifierMap->cend(); ++allowedQualifier)
    {
        std::wstring qualifierName;
        Atom qualifierAtom {};
        bool recognized = !Def_HrFailed0(
            environment->GetAtom(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, allowedQualifier->first.c_str(), &qualifierAtom),
            status);
        if (recognized)
        {
            qualifierName.assign(allowedQualifier->first);
        }
        else
        {
            const int tokenCount = profile->GetNumSupportedTokens();
            int tokenIndex = 0;
            for (; tokenIndex < tokenCount; ++tokenIndex)
            {
                StringResult token;
                if (!Def_HrFailed0(profile->GetToken(tokenIndex, &token), status) &&
                    token.CompareWithOptions(allowedQualifier->first.c_str(), DefCompare_CaseInsensitive) == Def_Equal)
                {
                    QualifierBuildInfo buildInfo {};
                    if (!Def_HrFailed0(profile->GetQualifierBuildInfoByToken(token.GetRef(), environment, &buildInfo), status))
                    {
                        StringResult resolvedName;
                        if (!Def_HrFailed0(
                                environment->GetName(
                                    UnifiedEnvironment::EnvironmentNamesType::QualifierNames, buildInfo.qualifier.name, &resolvedName),
                                status))
                        {
                            qualifierName.assign(resolvedName.GetRef());
                            recognized = true;
                        }
                    }
                    break;
                }
            }
            if (!recognized)
            {
                status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, qualifierName.c_str());
                break;
            }
        }

        for (auto allowedValue = allowedQualifier->second.cbegin(); allowedValue != allowedQualifier->second.cend(); ++allowedValue)
        {
            if (Def_HrFailed0(
                    environment->ValidateQualifierComparison(
                        qualifierName.c_str(), static_cast<ICondition::ConditionOperator>(10), allowedValue->c_str()),
                    status))
            {
                stop = true;
                if (status->GetWhat() == HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE))
                {
                    status->Reset();
                    const std::wstring description = qualifierName + L"-" + *allowedValue;
                    status->SetError(E_DEF_INVALID_ATTRIBUTE_VALUE, description.c_str());
                }
                else if (status->GetWhat() == HRESULT_FROM_WIN32(ERROR_MRM_UNKNOWN_QUALIFIER))
                {
                    status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, qualifierName.c_str());
                }
                break;
            }
        }
        if (stop)
        {
            break;
        }
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->GetHResult();
}

HRESULT CQualifierApplicator::ValidateUltFallbackQualifiers(IDefStatusEx* const status)
{
    std::map<std::wstring, std::wstring> environmentDefaults;
    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);

    const int tokenCount = _c_pProfile->GetNumSupportedTokens();
    for (int tokenIndex = 0; tokenIndex < tokenCount; ++tokenIndex)
    {
        StringResult token;
        if (!Def_HrFailed0(_c_pProfile->GetToken(tokenIndex, &token), status))
        {
            QualifierBuildInfo buildInfo {};
            if (!Def_HrFailed0(_c_pProfile->GetQualifierBuildInfoByToken(token.GetRef(), _c_pUnifiedEnvironment, &buildInfo), status))
            {
                StringResult qualifierName;
                if (!Def_HrFailed0(
                        _c_pUnifiedEnvironment->GetName(
                            UnifiedEnvironment::EnvironmentNamesType::QualifierNames, buildInfo.qualifier.name, &qualifierName),
                        status))
                {
                    DEFCOMPARISON comparison = Def_Equal;
                    DefStringResult_CompareWithOptions(
                        qualifierName.GetStringResult(), LanguageQualifierName, DefCompare_CaseInsensitive, &comparison);
                    if (comparison != Def_Equal)
                    {
                        environmentDefaults.insert(
                            std::make_pair(std::wstring(qualifierName.GetRef()), std::wstring(buildInfo.pDefaultValue)));
                    }
                }
            }
        }
    }

    if (status->Succeeded())
    {
        bool hasLanguage = false;
        for (const std::wstring& name : _UltFallbackAttrNameList)
        {
            if (DefString_CompareWithOptions(LanguageQualifierName, name.c_str(), DefCompare_CaseInsensitive) == Def_Equal)
            {
                hasLanguage = true;
                break;
            }
        }

        for (const auto& [name, value] : environmentDefaults)
        {
            bool found = false;
            for (const std::wstring& ultimateFallbackName : _UltFallbackAttrNameList)
            {
                if (DefString_CompareWithOptions(ultimateFallbackName.c_str(), name.c_str(), DefCompare_CaseInsensitive) == Def_Equal)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                result = _ValidateAndAddUltFallbackPair(name.c_str(), value.c_str(), status);
            }
        }

        if (!hasLanguage && status->Succeeded())
        {
            status->SetError(HRESULT_FROM_WIN32(ERROR_MRM_MISSING_DEFAULT_LANGUAGE), L"");
        }
    }

    const HRESULT computedResult = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return ComputeHResult(result, status);
}

HRESULT CQualifierApplicator::_AddAttributeValuePairToMap(
    const wchar_t* const name,
    const wchar_t* const value,
    const wchar_t* const token,
    const tagTOKEN_TYPE tokenType,
    std::multimap<std::wstring, std::wstring>* const values) const
{
    const wchar_t* nameToAdd = nullptr;
    const wchar_t* valueToAdd = nullptr;
    _GetAttrNameValueToAdd(name, value, token, tokenType, &nameToAdd, &valueToAdd);
    if (nameToAdd != nullptr && valueToAdd != nullptr)
    {
        values->insert(std::make_pair(std::wstring(nameToAdd), std::wstring(valueToAdd)));
    }
    else if (nameToAdd != LanguageQualifierName)
    {
        return E_FAIL;
    }
    return S_OK;
}

CQualifierApplicator::CQualifierSetBuilder::~CQualifierSetBuilder()
{
    if (_pQualifierSetBuilder != nullptr)
    {
        delete _pQualifierSetBuilder;
    }
}

HRESULT CQualifierApplicator::CQualifierSetBuilder::_AddQualifier(
    const wchar_t* const name,
    const wchar_t* const value,
    double* const fallbackScore,
    int* const priority,
    const tagTOKEN_TYPE tokenType,
    bool* const applied,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (name == nullptr || value == nullptr || applied == nullptr)
    {
        return E_INVALIDARG;
    }

    *applied = false;
    bool valid = false;
    std::multimap<std::wstring, std::wstring> AttributeValueMap;
    AttributeValueMap.insert(std::make_pair(std::wstring(name), std::wstring(value)));
    HRESULT result = _pQualifierApplicator->_ValidateAttributeValueMap(&AttributeValueMap, nullptr, tokenType, &valid, status);
    if (SUCCEEDED(result) && valid)
    {
        result =
            _pQualifierApplicator->_AddQualifierToQSB(&AttributeValueMap, _pQualifierSetBuilder, fallbackScore, priority, applied, status);
    }
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CQualifierApplicator::_AddQualifierToQSB(
    std::multimap<std::wstring, std::wstring>* const values,
    Build::DecisionInfoQualifierSetBuilder* const builder,
    double* const fallbackScore,
    int* const priority,
    bool* const applied,
    IDefStatusEx* const status) const
{
    HRESULT result = S_OK;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    *applied = false;

    for (auto value = values->begin(); value != values->end(); ++value)
    {
        int qualifierIndex = 0;
        bool isUltimateFallback = false;
        double score = 0.0;
        if (fallbackScore == nullptr)
        {
            result = _IsUltFallbackQualifier(value, &isUltimateFallback, &score, status);
        }
        if (FAILED(result))
        {
            break;
        }

        if (fallbackScore != nullptr)
        {
            score = *fallbackScore;
        }

        HRESULT callResult;
        if (priority == nullptr)
        {
            callResult = builder->AddQualifier(value->first.c_str(), value->second.c_str(), score, &qualifierIndex);
        }
        else
        {
            callResult = builder->AddQualifier(
                value->first.c_str(),
                static_cast<ICondition::ConditionOperator>(10),
                value->second.c_str(),
                *priority,
                score,
                &qualifierIndex);
        }
        *applied = !Def_HrFailed0(callResult, status);
    }

    if (status->GetWhat() == HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE))
    {
        status->Reset();
    }

    const HRESULT computedResult = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return computedResult;
}

HRESULT CQualifierApplicator::_EvaluteQalifierType(
    ResourceQualifier* const qualifier,
    const wchar_t* const value,
    const wchar_t* const fallbackValue,
    double* const score) const
{
    DefStatus status;
    const IBuildQualifierType* qualifierType = nullptr;
    IbcQualifier ibcQualifier(value, qualifier);
    if (!Def_HrFailed0(_c_pUnifiedEnvironment->GetTypeOfQualifier(qualifier->name, &qualifierType), &status))
    {
        Def_HrFailed0(qualifierType->Evaluate(&ibcQualifier, fallbackValue, score), &status);
    }
    return status.GetHResult();
}

void CQualifierApplicator::_GetAttrNameValueToAdd(
    const wchar_t* const name,
    const wchar_t* const value,
    const wchar_t* const token,
    const tagTOKEN_TYPE tokenType,
    const wchar_t** const nameToAdd,
    const wchar_t** const valueToAdd) const
{
    *nameToAdd = nullptr;
    *valueToAdd = nullptr;
    if (name == nullptr || value == nullptr)
    {
        *nameToAdd = LanguageQualifierName;
        *valueToAdd = tokenType == tagTOKEN_TYPE::tokenFile ? L"#" : token;
        return;
    }

    DefStatus status;
    Atom atom {};
    bool recognized =
        !Def_HrFailed0(_c_pUnifiedEnvironment->GetAtom(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, name, &atom), &status);
    if (recognized)
    {
        *nameToAdd = name;
    }
    else
    {
        const int tokenCount = _c_pProfile->GetNumSupportedTokens();
        for (int tokenIndex = 0; tokenIndex < tokenCount; ++tokenIndex)
        {
            StringResult supportedToken;
            if (!Def_HrFailed0(_c_pProfile->GetToken(tokenIndex, &supportedToken), &status) &&
                supportedToken.CompareWithOptions(name, DefCompare_CaseInsensitive) == Def_Equal)
            {
                QualifierBuildInfo buildInfo {};
                if (!Def_HrFailed0(
                        _c_pProfile->GetQualifierBuildInfoByToken(supportedToken.GetRef(), _c_pUnifiedEnvironment, &buildInfo), &status))
                {
                    StringResult resolvedName;
                    if (!Def_HrFailed0(
                            _c_pUnifiedEnvironment->GetName(
                                UnifiedEnvironment::EnvironmentNamesType::QualifierNames, buildInfo.qualifier.name, &resolvedName),
                            &status))
                    {
                        *nameToAdd = resolvedName.GetRef();
                        recognized = true;
                    }
                }
                break;
            }
        }
    }

    *valueToAdd = recognized ? value : nullptr;
    if (!recognized)
    {
        *nameToAdd = LanguageQualifierName;
        *valueToAdd = tokenType == tagTOKEN_TYPE::tokenFile ? L"#" : token;
    }
}

HRESULT CQualifierApplicator::_GetOrAddQualifierSet(
    Build::DecisionInfoQualifierSetBuilder* const builder,
    const int qualifierSetIndex,
    int* const resultQualifierSetIndex,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);

    if (builder->GetNumQualifiers() > 0)
    {
        std::list<QualifierResult> qualifiers;
        for (int index = 0; index < builder->GetNumQualifiers(); ++index)
        {
            QualifierResult qualifier;
            int qualifierIndex = 0;
            if (!Def_HrFailed0(builder->GetQualifier(index, &qualifier, &qualifierIndex), status))
            {
                qualifiers.push_back(qualifier);
            }
        }

        if (status->Succeeded() && qualifierSetIndex != _neutralQualifierSetIndex && qualifierSetIndex > -1)
        {
            QualifierSetResult qualifierSet;
            if (!Def_HrFailed0(_pDecisionInfoBuilder->GetQualifierSet(qualifierSetIndex, &qualifierSet), status))
            {
                for (int index = 0; index < qualifierSet.GetNumQualifiers(); ++index)
                {
                    QualifierResult qualifier;
                    int qualifierIndex = 0;
                    Atom qualifierNameAtom {};
                    StringResult qualifierName;
                    StringResult qualifierValue;
                    double score = 0.0;
                    Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, &qualifierIndex), status);
                    Def_HrFailed0(qualifier.GetOperand1Attribute(&qualifierNameAtom), status);
                    if (!Def_HrFailed0(
                            _c_pUnifiedEnvironment->GetName(
                                UnifiedEnvironment::EnvironmentNamesType::QualifierNames, qualifierNameAtom, &qualifierName),
                            status) &&
                        !Def_HrFailed0(qualifier.GetOperand2Literal(&qualifierValue), status))
                    {
                        Def_HrFailed0(qualifier.GetFallbackScore(&score), status);
                        int addedQualifierIndex = 0;
                        Def_HrFailed0(
                            builder->AddQualifier(qualifierName.GetRef(), qualifierValue.GetRef(), score, &addedQualifierIndex), status);
                    }
                }
            }
        }

        Def_HrFailed0(_pDecisionInfoBuilder->GetOrAddQualifierSet(builder, nullptr, resultQualifierSetIndex), status);
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->GetHResult();
}

HRESULT CQualifierApplicator::_IsUltFallbackQualifier(
    std::multimap<std::wstring, std::wstring>::iterator& value,
    bool* const isUltimateFallback,
    double* const fallbackScore,
    IDefStatusEx* const status) const
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    *isUltimateFallback = false;
    *fallbackScore = 0.0;

    for (auto ultimateFallback = _UltFallbackAttrValueMap.begin(); ultimateFallback != _UltFallbackAttrValueMap.end(); ++ultimateFallback)
    {
        ResourceQualifier valueQualifier {};
        HRESULT callResult = _c_pUnifiedEnvironment->GetResourceQualifier(value->first.c_str(), &valueQualifier);
        if (Def_HrFailed0(callResult, status))
        {
            result = status->Succeeded() ? E_FAIL : status->GetHResult();
        }

        ResourceQualifier ultimateFallbackQualifier = *ultimateFallback->first;
        if (SUCCEEDED(result) && ultimateFallbackQualifier.qualifierType.GetPoolIndex() == valueQualifier.qualifierType.GetPoolIndex() &&
            ultimateFallbackQualifier.qualifierType.GetIndex() == valueQualifier.qualifierType.GetIndex())
        {
            callResult = _c_pUnifiedEnvironment->ValidateQualifierValue(valueQualifier.name, value->second.c_str());
            if (Def_HrFailed0(callResult, status))
            {
                DefStatus nameStatus;
                StringResult qualifierName;
                if (!Def_HrFailed0(
                        _c_pUnifiedEnvironment->GetName(
                            UnifiedEnvironment::EnvironmentNamesType::QualifierNames, valueQualifier.name, &qualifierName),
                        &nameStatus))
                {
                    status->SetError(status->GetWhat(), qualifierName.GetRef());
                }
                result = status->Succeeded() ? E_FAIL : status->GetHResult();
            }
            else
            {
                StringResult ultimateFallbackValue;
                Def_HrFailed0(DefStringResult_InitRef(ultimateFallbackValue.GetStringResult(), ultimateFallback->second.c_str()), status);
                StringResult qualifierValue;
                Def_HrFailed0(DefStringResult_InitRef(qualifierValue.GetStringResult(), value->second.c_str()), status);
                result = status->GetHResult();
                if (SUCCEEDED(result))
                {
                    result = _EvaluteQalifierType(
                        &ultimateFallbackQualifier, qualifierValue.GetRef(), ultimateFallbackValue.GetRef(), fallbackScore);
                    if (FAILED(result))
                    {
                        *fallbackScore = 0.0;
                        *isUltimateFallback = false;
                    }
                    else
                    {
                        *isUltimateFallback = true;
                    }
                }
            }
            break;
        }
        if (FAILED(result))
        {
            break;
        }
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CQualifierApplicator::_ParseToken(
    const wchar_t* const token,
    const tagTOKEN_TYPE tokenType,
    std::multimap<std::wstring, std::wstring>* const values) const
{
    HRESULT result = S_OK;
    const std::wstring tokenString(token);
    std::vector<std::wstring> parts;
    std::size_t start = 0;
    for (std::size_t end = tokenString.find(L"_", 0, 1); end != std::wstring::npos; end = tokenString.find(L"_", end + 1, 1))
    {
        if (start != end)
        {
            parts.emplace_back(
                tokenString.begin() + static_cast<std::ptrdiff_t>(start), tokenString.begin() + static_cast<std::ptrdiff_t>(end));
        }
        start = end + 1;
    }
    if (start < tokenString.length())
    {
        parts.emplace_back(tokenString.begin() + static_cast<std::ptrdiff_t>(start), tokenString.end());
    }

    for (const std::wstring& part : parts)
    {
        const std::size_t separator = part.find(L"-", 0, 1);
        if (separator == std::wstring::npos)
        {
            result = _AddAttributeValuePairToMap(nullptr, nullptr, part.c_str(), tokenType, values);
        }
        else
        {
            const std::wstring name(part.begin(), part.begin() + static_cast<std::wstring::difference_type>(separator));
            const std::wstring value(part.begin() + static_cast<std::ptrdiff_t>(separator + 1), part.end());
            result = _AddAttributeValuePairToMap(name.c_str(), value.c_str(), part.c_str(), tokenType, values);
        }
    }
    return result;
}

HRESULT CQualifierApplicator::_ValidateAndAddUltFallbackPair(
    const wchar_t* const name,
    const wchar_t* const value,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    ResourceQualifier qualifier {};
    if (!Def_HrFailed0(_c_pUnifiedEnvironment->GetResourceQualifier(name, &qualifier), status))
    {
        StringResult qualifierName;
        Def_HrFailed0(
            _c_pUnifiedEnvironment->GetName(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, qualifier.name, &qualifierName),
            status);
        _UltFallbackAttrNameList.push_back(qualifierName.GetRef());
        if (*value != L'\0')
        {
            if (Def_HrFailed0(_c_pUnifiedEnvironment->ValidateQualifierValue(qualifier.name, value), status))
            {
                status->SetError(status->GetWhat(), qualifierName.GetRef());
            }
            else
            {
                ResourceQualifier* const qualifierCopy = new (std::nothrow) ResourceQualifier {};
                if (qualifierCopy != nullptr)
                {
                    *qualifierCopy = qualifier;
                    _UltFallbackAttrValueMap.insert(std::make_pair(qualifierCopy, std::wstring(value)));
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
            }
        }
    }

    const HRESULT computedResult = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return ComputeHResult(result, status);
}

HRESULT CQualifierApplicator::_ValidateAttributeValueMap(
    std::multimap<std::wstring, std::wstring>* const values,
    const wchar_t* const token,
    const tagTOKEN_TYPE tokenType,
    bool* const valid,
    IDefStatusEx* const status) const
{
    StringResult invalidQualifierName;
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    *valid = false;
    bool isLanguage = false;

    for (auto value = values->begin(); value != values->end(); ++value)
    {
        if (Def_HrFailed0(
                _c_pUnifiedEnvironment->ValidateQualifierComparison(
                    value->first.c_str(), static_cast<ICondition::ConditionOperator>(10), value->second.c_str()),
                status))
        {
            *valid = false;
            isLanguage = wcscmp(value->first.c_str(), LanguageQualifierName) == 0;
            DefStringResult_SetCopy(invalidQualifierName.GetStringResult(), value->first.c_str());
            break;
        }

        if (wcscmp(value->first.c_str(), LanguageQualifierName) == 0 && tokenType == tagTOKEN_TYPE::tokenFolder &&
            !Windows::Internal::CLanguage::IsValidTag(value->second.c_str()))
        {
            isLanguage = true;
            *valid = false;
            break;
        }

        if (_pAllowedQualifierValues != nullptr)
        {
            Atom valueAtom {};
            if (!Def_HrFailed0(
                    _c_pUnifiedEnvironment->GetAtom(
                        UnifiedEnvironment::EnvironmentNamesType::QualifierNames, value->first.c_str(), &valueAtom),
                    status))
            {
                auto allowedQualifier = _pAllowedQualifierValues->begin();
                for (; allowedQualifier != _pAllowedQualifierValues->end(); ++allowedQualifier)
                {
                    Atom allowedAtom {};
                    if (!Def_HrFailed0(
                            _c_pUnifiedEnvironment->GetAtom(
                                UnifiedEnvironment::EnvironmentNamesType::QualifierNames, allowedQualifier->first.c_str(), &allowedAtom),
                            status) &&
                        allowedAtom.GetPoolIndex() == valueAtom.GetPoolIndex() && allowedAtom.GetIndex() == valueAtom.GetIndex())
                    {
                        break;
                    }
                }

                if (allowedQualifier != _pAllowedQualifierValues->end())
                {
                    auto allowedValue = allowedQualifier->second.begin();
                    for (; allowedValue != allowedQualifier->second.end(); ++allowedValue)
                    {
                        if (CompareStringOrdinal(value->second.c_str(), -1, allowedValue->c_str(), -1, true) == CSTR_EQUAL)
                        {
                            break;
                        }
                    }
                    if (allowedValue == allowedQualifier->second.end())
                    {
                        const std::wstring description = value->first + L"-" + value->second;
                        status->SetError(E_DEF_QUALAPPL_VALUE_NOT_ALLOWED, description.c_str());
                        break;
                    }
                }
            }
        }
        *valid = true;
    }

    if (status->GetWhat() == HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE))
    {
        status->Reset();
        const wchar_t* const displayToken = token != nullptr ? token : L"blank";
        if (!isLanguage || (wcscspn(displayToken, L"-") < wcslen(displayToken) && tokenType == tagTOKEN_TYPE::tokenFile))
        {
            status->AddWarning(E_DEF_QUALAPPL_INVALID_QUALIFIER, displayToken);
        }
    }
    else if (status->GetWhat() == HRESULT_FROM_WIN32(ERROR_MRM_UNKNOWN_QUALIFIER))
    {
        status->SetError(E_DEF_PRICONFIG_INVALID_QUAL, invalidQualifierName.GetRef());
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->GetHResult();
}

HRESULT CQualifierApplicator::AddUltFallbackAttrValuePair(
    const wchar_t* const name,
    const wchar_t* const value,
    const wchar_t* const source,
    IDefStatusEx* const status)
{
    static_cast<void>(source);
    const wchar_t* nameToAdd = nullptr;
    const wchar_t* valueToAdd = nullptr;
    _GetAttrNameValueToAdd(name, value, nullptr, tagTOKEN_TYPE::tokenDefault, &nameToAdd, &valueToAdd);
    if (nameToAdd != nullptr && valueToAdd != nullptr)
    {
        return _ValidateAndAddUltFallbackPair(nameToAdd, valueToAdd, status);
    }
    return E_INVALIDARG;
}

HRESULT CQualifierApplicator::GetDefaultQualifierValues(const wchar_t* const source, IDefStatusEx* const status, StringResult* const result)
{
    static_cast<void>(source);
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    Def_HrFailed0(result->SetRef(nullptr), status);

    for (const auto& [qualifier, value] : _UltFallbackAttrValueMap)
    {
        StringResult qualifierName;
        if (!Def_HrFailed0(
                _c_pUnifiedEnvironment->GetName(UnifiedEnvironment::EnvironmentNamesType::QualifierNames, qualifier->name, &qualifierName),
                status))
        {
            DEFCOMPARISON comparison = Def_Equal;
            DefStringResult_CompareWithOptions(
                qualifierName.GetStringResult(), LanguageQualifierName, DefCompare_CaseInsensitive, &comparison);
            if (comparison == Def_Equal)
            {
                if (result->GetLength() != 0)
                {
                    Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), L";"), status);
                }
                Def_HrFailed0(DefStringResult_Concat(result->GetStringResult(), value.c_str()), status);
            }
        }
    }

    const HRESULT computedResult = ComputeHResult(S_OK, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return ComputeHResult(S_OK, status);
}

HRESULT CQualifierApplicator::GetQualifierMapFromToken(
    const wchar_t* const token,
    Runtime::MrtMap<const wchar_t*, const wchar_t*>* const map,
    IDefStatusEx* const status)
{
    if (token == nullptr || map == nullptr)
    {
        return E_INVALIDARG;
    }

    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    map->Clear();
    std::multimap<std::wstring, std::wstring> values;
    _AttributeValueMapStore.clear();
    HRESULT result = _ParseToken(token, tagTOKEN_TYPE::tokenDefault, &values);

    if (SUCCEEDED(result) && !values.empty())
    {
        for (auto current = values.begin(); current != values.end(); ++current)
        {
            auto duplicate = current;
            ++duplicate;
            while (duplicate != values.end())
            {
                if (DefString_CompareWithOptions(current->first.c_str(), duplicate->first.c_str(), DefCompare_CaseInsensitive) ==
                        Def_Equal &&
                    DefString_CompareWithOptions(current->second.c_str(), duplicate->second.c_str(), DefCompare_CaseInsensitive) ==
                        Def_Equal)
                {
                    values.erase(duplicate);
                    duplicate = current;
                    ++duplicate;
                }
                ++duplicate;
            }
        }

        for (auto value = values.begin(); value != values.end(); ++value)
        {
            if (Def_HrFailed0(
                    _c_pUnifiedEnvironment->ValidateQualifierComparison(
                        value->first.c_str(), static_cast<ICondition::ConditionOperator>(10), value->second.c_str()),
                    status))
            {
                const std::wstring description = L"(" + value->first + L": " + value->second + L")";
                status->SetError(E_DEF_QUALAPPL_INVALID_QUALIFIER, description.c_str());
                map->Clear();
                values.clear();
                _AttributeValueMapStore.clear();
                break;
            }

            auto existing = _AttributeValueMapStore.find(value->first);
            if (existing == _AttributeValueMapStore.end())
            {
                _AttributeValueMapStore.insert(std::make_pair(value->first, value->second));
            }
            else
            {
                existing->second.append(L";");
                existing->second.append(value->second, 0, std::wstring::npos);
            }
        }

        for (const auto& [name, value] : _AttributeValueMapStore)
        {
            map->Insert(name.c_str(), value.c_str());
        }
    }

    const HRESULT computedResult = ComputeHResult(result, status);
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return computedResult;
}
} // namespace Microsoft::Resources::Indexers
