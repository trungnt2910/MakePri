#include "StdAfx.h"

#include <IndexerBase.h>

namespace Microsoft::Resources::Indexers
{

HRESULT WINAPI CIndexPass::New(
    IXMLDOMNode* const pIndexPassNode,
    const MrmProfile* const pProfile,
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    Build::DecisionInfoBuilder* const pDecisionInfoBuilder,
    const IIndexOptions* const options,
    std::vector<LogItem>* const pLogItems,
    IDefStatusEx* const pStatus,
    CIndexPass** const ppCIndexPass)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT hr = E_OUTOFMEMORY;
    CIndexPass* pCIndexPass = new (std::nothrow) CIndexPass();
    if (pCIndexPass != nullptr)
    {
        hr = pCIndexPass->_Init(
            pIndexPassNode, pProfile, pEnvironment, pProjectRootFolder, pDecisionInfoBuilder, options, pLogItems, pStatus);
        if (FAILED(hr))
        {
            delete pCIndexPass;
        }
        else
        {
            *ppCIndexPass = pCIndexPass;
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(hr, pStatus));
    return ComputeHResult(hr, pStatus);
}

HRESULT CIndexPass::_Init(
    IXMLDOMNode* const pIndexPassNode,
    const MrmProfile* const pProfile,
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    Build::DecisionInfoBuilder* const pDecisionInfoBuilder,
    const IIndexOptions* const options,
    std::vector<LogItem>* const pLogItems,
    IDefStatusEx* const pStatus)
{
    _pIndexPassNode = pIndexPassNode;
    _c_pEnvironment = pEnvironment;
    _c_pProfile = pProfile;
    _pDecisionInfoBuilder = pDecisionInfoBuilder;
    _bSuppressConsoleOutput = options->GetShouldSuppressConsoleOutput();
    _bSuppressEmbeddedData = options->GetShouldSuppressEmbeddedData();
    _pLogItems = pLogItems;

    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = _ParseAllowedNodes(_pIndexPassNode, pStatus);
    if (FAILED(result))
    {
        pStatus->SetError(E_DEF_PRICONFIG_INVALID_ALLOWED_NODE, L"");
    }
    else
    {
        result = CQualifierApplicator::ValidateAllowedQualiferMapSet(pProfile, pEnvironment, &_allowedQualifierValues, pStatus);
        if (SUCCEEDED(result))
        {
            result = E_OUTOFMEMORY;
            _pQualifierApplicator = new (std::nothrow)
                CQualifierApplicator(pIndexPassNode, pProfile, pEnvironment, pDecisionInfoBuilder, &_allowedQualifierValues);
            if (_pQualifierApplicator != nullptr)
            {
                result = CBootStrapIndexer::New(
                    pEnvironment,
                    pDecisionInfoBuilder,
                    pProjectRootFolder,
                    pIndexPassNode,
                    _pQualifierApplicator,
                    pStatus,
                    &_pBootStrapIndexer);
                if (SUCCEEDED(result) && _pBootStrapIndexer != nullptr)
                {
                    _pProjectRootFolder = _pBootStrapIndexer->GetProjectRoot(pStatus);
                    result = _ParseNode(pIndexPassNode, pStatus);
                    if (SUCCEEDED(result))
                    {
                        result = _InitializeIndexers(pIndexPassNode, options, pStatus);
                    }
                }
            }
            else
            {
                pStatus->DiagnosticLogA("Failed - Allocating CQualifierApplicator");
            }
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

CIndexPass::~CIndexPass()
{
    delete _pQualifierApplicator;
    delete _pBootStrapIndexer;

    while (!_IndexPassNodeDisposalList.empty())
    {
        SAFE_RELEASE(_IndexPassNodeDisposalList.top());
        _IndexPassNodeDisposalList.pop();
    }

    for (auto& indexerList : _FSILists)
    {
        for (IFormatSpecificIndexer*& indexer : indexerList)
        {
            delete indexer;
            indexer = nullptr;
        }
    }
}

HRESULT CIndexPass::GetIndexablePath(IDefStatusEx* const pStatus, StringResult* const strIndexablePath)
{
    return _pBootStrapIndexer->GetIndexablePath(pStatus, strIndexablePath);
}

HRESULT CIndexPass::GetDefaultQualifierValues(
    const wchar_t* const pSource,
    IDefStatusEx* const pStatus,
    StringResult* const strQualifierValues)
{
    return _pQualifierApplicator->GetDefaultQualifierValues(pSource, pStatus, strQualifierValues);
}

HRESULT CIndexPass::Process(CItemInstanceSink* const pTraversalSink, CItemInstanceSink* const pIndexSink, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    StringResult source;

    CItemInstanceSink* pOutputSink;
    if (pIndexSink->empty())
    {
        pOutputSink = pIndexSink;
    }
    else
    {
        pOutputSink = new (std::nothrow) CItemInstanceSink();
    }

    if (pOutputSink != nullptr && _pBootStrapIndexer != nullptr)
    {
        bool handled = false;
        if (SUCCEEDED(_pBootStrapIndexer->Process(pTraversalSink, &handled)))
        {
            CItemInstanceEntry* entry = pTraversalSink->PopEntry();
            Def_HrFailed0(DefStringResult_SetCopy(source.GetStringResult(), entry->value.GetRef()), pStatus);
            while (entry != nullptr)
            {
                bool retainEntry = (entry->flags & 1) != 0;
                if ((entry->flags & 2) != 0)
                {
                    for (auto& indexerList : _FSILists)
                    {
                        for (IFormatSpecificIndexer* const indexer : indexerList)
                        {
                            result = indexer->Process(entry, pTraversalSink, pStatus, &handled);
                            if (FAILED(result))
                            {
                                if (result != E_DEF_IBC_CANDIDATE_NOT_EMBEDDED)
                                {
                                    break;
                                }
                                pStatus->AddWarning(E_DEF_IBC_CANDIDATE_NOT_EMBEDDED, entry->value.GetRef());
                                result = S_OK;
                            }
                            retainEntry = retainEntry && !handled;
                        }
                        if (FAILED(result) || !retainEntry)
                        {
                            break;
                        }
                    }
                }

                if (FAILED(result))
                {
                    delete entry;
                    while (!pTraversalSink->empty())
                    {
                        delete pTraversalSink->PopEntry();
                    }
                    break;
                }

                if (retainEntry)
                {
                    entry->projectRoot = _pProjectRootFolder;
                    result = pOutputSink->AddEntry(entry);
                    if (FAILED(result))
                    {
                        delete entry;
                        while (!pTraversalSink->empty())
                        {
                            delete pTraversalSink->PopEntry();
                        }
                        break;
                    }
                }
                else
                {
                    delete entry;
                }

                handled = false;
                entry = pTraversalSink->PopEntry();
            }
        }
    }

    if (SUCCEEDED(result))
    {
        result = _DisplayQualifierInfo(pOutputSink, source.GetRef(), pStatus);
    }

    if (pOutputSink != pIndexSink)
    {
        if (SUCCEEDED(result))
        {
            while (!pOutputSink->empty())
            {
                CItemInstanceEntry* const entry = pOutputSink->PopEntry();
                result = pIndexSink->AddEntry(entry);
                if (FAILED(result))
                {
                    delete entry;
                    break;
                }
            }
        }
        delete pOutputSink;
    }

    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CIndexPass::_ParseNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper helper(pIndexPassNode);
    IXMLDOMNodeList* pNodeList = nullptr;
    helper.TryGetChildren(L"indexer-config", pStatus, &pNodeList);

    long length;
    HRESULT result = pNodeList->get_length(&length);
    if (length > 0)
    {
        result = _ProcessIndexerNodes(pNodeList, pStatus);
    }
    SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(pNodeList));
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CIndexPass::_ProcessIndexerNodes(IXMLDOMNodeList* const pNodeList, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    long length;
    HRESULT result = pNodeList->get_length(&length);
    if (SUCCEEDED(result))
    {
        for (int index = 0; index < length; ++index)
        {
            IXMLDOMNode* node;
            result = pNodeList->get_item(index, &node);
            if (FAILED(result))
            {
                break;
            }

            CXmlHelper helper(node);
            wchar_t* type = nullptr;
            helper.GetAttributeValue(L"type", pStatus, &type);
            result = _InstantiateIndexer(type, pStatus);
            if (FAILED(result))
            {
                SAFE_RELEASE(node);
                operator delete(type);
                break;
            }
            operator delete(type);
            SAFE_RELEASE(node);
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CIndexPass::_InitializeIndexers(IXMLDOMNode* const pIPNode, const IIndexOptions* const options, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    for (auto& indexerList : _FSILists)
    {
        for (IFormatSpecificIndexer* const indexer : indexerList)
        {
            CXmlHelper helper(pIPNode);
            IXMLDOMNode* clone;
            result = pIPNode->cloneNode(VARIANT_TRUE, &clone);
            if (SUCCEEDED(result))
            {
                IXMLDOMDocument* document;
                result = CoCreateInstance(
                    CLSID_DOMDocument60, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, reinterpret_cast<void**>(&document));
                if (SUCCEEDED(result))
                {
                    IXMLDOMNode* child;
                    result = document->appendChild(clone, &child);
                    if (SUCCEEDED(result))
                    {
                        const HRESULT initResult =
                            indexer->Init(_c_pEnvironment, _pProjectRootFolder, child, _pQualifierApplicator, options, pStatus);
                        _IndexPassNodeDisposalList.push(clone);
                        _IndexPassNodeDisposalList.push(reinterpret_cast<IXMLDOMNode*>(document));
                        _IndexPassNodeDisposalList.push(child);
                        result = initResult;
                    }
                }
            }
            if (FAILED(result))
            {
                break;
            }
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CIndexPass::_DisplayQualifierInfo(
    CItemInstanceSink* const pIndexSink,
    const wchar_t* const pStartIndexAtValue,
    IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = S_OK;
    if (!_bSuppressConsoleOutput)
    {
        std::map<std::wstring, CUtilities::QualifierValues*> qualifierMap;
        result = CUtilities::GetQualifierStringMap(
            _pDecisionInfoBuilder, const_cast<AtomPoolGroup*>(_c_pEnvironment->GetAtoms()), pIndexSink, &qualifierMap, pStatus, false);
        if (SUCCEEDED(result))
        {
            if (pStartIndexAtValue != nullptr && wcsnlen(pStartIndexAtValue, MAX_PATH) != 0)
            {
                result = CHIndexerBase::LogInfo(_pLogItems, L"Index Pass Completed: %s", pStartIndexAtValue);
                if (FAILED(result))
                {
                    // Original line: 460
                    RETURN_HR(result);
                }
            }
            else
            {
                result = CHIndexerBase::LogInfo(_pLogItems, L"Index Pass Completed.");
                if (FAILED(result))
                {
                    // Original line: 464
                    RETURN_HR(result);
                }
            }

            for (const auto& [name, values] : qualifierMap)
            {
                result = CHIndexerBase::LogInfo(_pLogItems, L"%s Qualifiers: %s", name.c_str(), values->wstrValues.c_str());
                if (FAILED(result))
                {
                    // Original line: 469
                    RETURN_HR(result);
                }
            }

            result = CHIndexerBase::LogInfo(_pLogItems, L"");
            if (FAILED(result))
            {
                // Original line: 471
                RETURN_HR(result);
            }
        }

        for (auto& [name, values] : qualifierMap)
        {
            delete values;
        }
    }

    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CIndexPass::_InstantiateIndexer(wchar_t* const pIndexerType, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (!_bSuppressEmbeddedData || DefString_CompareWithOptions(pIndexerType, L"embedfiles", DefCompare_CaseInsensitive) != 0)
    {
        FSIList_Group group;
        IFormatSpecificIndexer* const indexer = CFsiFactory::s_GetIndexer(pIndexerType, pStatus, &group);
        if (indexer != nullptr)
        {
            auto& indexerList = _FSILists[static_cast<int>(group)];
            bool found = false;
            for (IFormatSpecificIndexer* const existing : indexerList)
            {
                if (existing == indexer)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                indexerList.push_front(indexer);
            }
        }
    }

    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, pStatus->GetHResult());
    return pStatus->GetHResult();
}

HRESULT CIndexPass::_ParseAllowedNodes(IXMLDOMNode* const pIndexNode, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper helper(pIndexNode);
    IXMLDOMNode* defaultNode = nullptr;
    HRESULT result;
    if (SUCCEEDED(helper.TryGetChildNode(L"default", pStatus, &defaultNode)) && defaultNode != nullptr)
    {
        CXmlHelper defaultHelper(defaultNode);
        IXMLDOMNodeList* qualifiers = nullptr;
        result = defaultHelper.TryGetChildren(L"qualifier", pStatus, &qualifiers);
        if (SUCCEEDED(result) && qualifiers != nullptr)
        {
            long length;
            result = qualifiers->get_length(&length);
            if (SUCCEEDED(result))
            {
                int index = 0;
                do
                {
                    if (index >= length)
                    {
                        break;
                    }
                    IXMLDOMNode* qualifier = nullptr;
                    result = qualifiers->get_item(index, &qualifier);
                    if (SUCCEEDED(result) && qualifier != nullptr)
                    {
                        result = _ParseAllowedNodeQualifierValues(qualifier, pStatus);
                        SAFE_RELEASE(qualifier);
                    }
                    ++index;
                } while (SUCCEEDED(result));
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(qualifiers));
        }
        SAFE_RELEASE(defaultNode);
    }
    else
    {
        result = S_OK;
        pStatus->Reset();
    }

    const HRESULT computedResult = ComputeHResult(result, pStatus);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return computedResult;
}

HRESULT CIndexPass::_ParseAllowedNodeQualifierValues(IXMLDOMNode* const pQualifierNode, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    CXmlHelper helper(pQualifierNode);
    wchar_t* name;
    HRESULT result = helper.GetAttributeValue(L"name", pStatus, &name);
    if (SUCCEEDED(result) && name != nullptr)
    {
        std::wstring qualifierName(name);
        std::wstring valueNodeName = qualifierName + L"Value";
        IXMLDOMNode* allowedNode = nullptr;
        if (SUCCEEDED(helper.TryGetChildNode(L"allowed", pStatus, &allowedNode)) && allowedNode != nullptr)
        {
            CXmlHelper allowedHelper(allowedNode);
            IXMLDOMNodeList* valuesNodes = nullptr;
            result = allowedHelper.TryGetChildren(valueNodeName.c_str(), pStatus, &valuesNodes);
            if (SUCCEEDED(result) && valuesNodes != nullptr)
            {
                long length;
                result = valuesNodes->get_length(&length);
                std::set<std::wstring> values;
                for (int index = 0; SUCCEEDED(result) && index < length; ++index)
                {
                    IXMLDOMNode* valueNode = nullptr;
                    result = valuesNodes->get_item(index, &valueNode);
                    if (SUCCEEDED(result) && valueNode != nullptr)
                    {
                        CXmlHelper valueHelper(valueNode);
                        wchar_t* value = nullptr;
                        result = valueHelper.GetNodeText(pStatus, &value);
                        if (SUCCEEDED(result) && value != nullptr)
                        {
                            values.insert(std::wstring(value));
                            operator delete(value);
                        }
                        SAFE_RELEASE(valueNode);
                    }
                }

                const wchar_t* const mapName = CompareStringOrdinal(name, -1, L"Platform", -1, TRUE) == CSTR_EQUAL ? L"DeviceFamily" : name;
                _allowedQualifierValues.insert(std::make_pair(std::wstring(mapName), values));
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(valuesNodes));
            }
            SAFE_RELEASE(allowedNode);
        }
        else
        {
            result = S_OK;
            pStatus->Reset();
        }
        operator delete(name);
    }

    const HRESULT computedResult = ComputeHResult(result, pStatus);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, computedResult);
    return computedResult;
}

} // namespace Microsoft::Resources::Indexers
