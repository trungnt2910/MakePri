#include "StdAfx.h"

#include <CFolderIndexer.h>

namespace Microsoft::Resources::Indexers
{

CFolderIndexer::~CFolderIndexer()
{
    if (_pFIXmlConfig != nullptr)
    {
        _pFIXmlConfig->CFIXmlConfig::~CFIXmlConfig();
        ::operator delete(_pFIXmlConfig);
    }
    delete _pFolderInfo;
}

HRESULT CFolderIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pQualApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    if (_bDoneInit)
    {
        _bError = true;
        return NTE_EXISTS;
    }
    _bDoneInit = true;
    if (pEnvironment == nullptr || pProjectRootFolder == nullptr || pIndexPassNode == nullptr || pQualApplicator == nullptr)
    {
        _bError = true;
        return E_INVALIDARG;
    }
    if (_bError)
    {
        return E_FAIL;
    }

    _pQualApplicator = pQualApplicator;
    _options = options;
    return _Init(pEnvironment, pProjectRootFolder, pIndexPassNode, pStatus);
}

HRESULT CFolderIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pSink,
    IDefStatusEx* const pStatus,
    bool* const pRemoveContainer)
{
    if (pEntry == nullptr || pSink == nullptr || pRemoveContainer == nullptr)
    {
        _bError = true;
        return E_INVALIDARG;
    }
    if (_pFIXmlConfig == nullptr)
    {
        _bError = true;
    }
    if (_bError)
    {
        return E_FAIL;
    }
    if (!MrmEnvironment::IsPathResourceValueType(pEntry->resourceValueType) ||
        pEntry->resourceItemType != MrmEnvironment::ResourceItemType_Path)
    {
        *pRemoveContainer = false;
        return S_OK;
    }

    std::wstring path;
    path.append(_pFolderInfo->_wszFolder, 0, std::wstring::npos);
    const wchar_t* const entryPath = pEntry->value.GetRef();
    StringResult entryPathResult;
    Def_HrFailed0(DefStringResult_InitRef(entryPathResult.GetStringResult(), entryPath), pStatus);
    bool isAbsolute = false;
    entryPathResult.IsAbsolutePath(L'\\', &isAbsolute);
    if (isAbsolute)
    {
        path.assign(entryPath);
    }
    else
    {
        path.append(entryPath);
    }

    AutoDeletePtr<CFolderInfo> folder(new (std::nothrow) CFolderInfo());
    if (folder.Data() == nullptr)
    {
        RETURN_HR(E_OUTOFMEMORY);
    }

    HRESULT result = folder.Data()->Set(path.c_str(), pStatus);
    if (FAILED(result))
    {
        if (pStatus->GetWhat() == E_DEF_FSI_UNSUPPORTED_DIR_TYPE)
        {
            pStatus->Reset();
            result = S_OK;
            *pRemoveContainer = (pEntry->flags & 1) == 0;
            return result;
        }
        return result;
    }

    *pRemoveContainer = true;
    result = folder.Data()->TraverseChildren();
    RETURN_IF_FAILED(result);
    result = ProcessSubFolders(pEntry, folder.Data(), pSink, pStatus);
    RETURN_IF_FAILED(result);
    result = ProcessFiles(pEntry, folder.Data(), pSink, pStatus);
    RETURN_IF_FAILED(result);
    return result;
}

HRESULT CFolderIndexer::ProcessSubFolders(
    CItemInstanceEntry* const pEntry,
    CFolderInfo* const pFolder,
    CItemInstanceSink* const pSink,
    IDefStatusEx* const pStatus)
{
    const std::uint32_t count = pFolder->_pFolderList != nullptr ? static_cast<std::uint32_t>(pFolder->_pFolderList->size()) : 0;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        CFolderInfo childFolder;
        std::wstring folderName;
        std::wstring itemName;
        std::wstring value;
        ITEM_INSTANCE_ENTRY instance;
        _InitInstanceEntry(pEntry, &instance);

        HRESULT result = pFolder->GetCFolderInfo(index, &childFolder, pStatus);
        RETURN_IF_FAILED(result);
        const std::size_t lastSlash = childFolder._wszFolder.find_last_of(L"\\", std::wstring::npos);
        const std::size_t previousSlash = childFolder._wszFolder.find_last_of(L"\\", childFolder._wszFolder.length() - 2);
        if (previousSlash == std::wstring::npos)
        {
            RETURN_HR(E_FAIL);
        }
        folderName.append(childFolder._wszFolder.substr(previousSlash + 1, lastSlash - previousSlash - 1), 0, std::wstring::npos);

        FOLDER_FILE_ATTRIBUTES ignoredAttributes;
        if (SUCCEEDED(
                CFolderInfo::IsSpecialFileOrFolderToIgnore(
                    childFolder._ulFolderAttribute, childFolder._ulReserved, _options, &ignoredAttributes)))
        {
            pStatus->AddWarning(
                E_DEF_FSI_SPECIAL_FILE_IGNORED,
                pEntry->valueTypeName.GetRef(),
                0,
                childFolder._wszFolderAbsolutePath.c_str(),
                static_cast<std::uint32_t>(ignoredAttributes));
            continue;
        }

        result = _CreateFolderEntry(pEntry, folderName, itemName, value, &instance, pStatus);
        RETURN_IF_FAILED(result);
        AutoDeletePtr<CItemInstanceEntry> newEntry(
            CItemInstanceEntry::NewForString(
                L"Files",
                instance.pItemName,
                instance.itemType,
                instance.resourceValueType,
                instance.pValue,
                instance.conditionSetIndex,
                instance.uiActionFlags,
                nullptr,
                nullptr,
                pStatus));
        if (newEntry.Data() == nullptr)
        {
            result = pStatus->GetHResult();
            RETURN_IF_FAILED(result);
            return result;
        }
        result = pSink->AddEntry(newEntry.Data());
        RETURN_IF_FAILED(result);
        newEntry.Detach();
    }
    return S_OK;
}

HRESULT CFolderIndexer::ProcessFiles(
    CItemInstanceEntry* const pEntry,
    CFolderInfo* const pFolder,
    CItemInstanceSink* const pSink,
    IDefStatusEx* const pStatus)
{
    const std::uint32_t count = pFolder->_pFileList != nullptr ? static_cast<std::uint32_t>(pFolder->_pFileList->size()) : 0;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        CFileInfo file;
        std::wstring itemName;
        std::wstring value;
        std::wstring fileName;
        ITEM_INSTANCE_ENTRY instance;
        _InitInstanceEntry(pEntry, &instance);

        HRESULT result = pFolder->GetCFileInfo(index, &file, pStatus);
        RETURN_IF_FAILED(result);
        FOLDER_FILE_ATTRIBUTES ignoredAttributes;
        if (SUCCEEDED(CFolderInfo::IsSpecialFileOrFolderToIgnore(file._ulFileAttribute, file._ulReserved, _options, &ignoredAttributes)))
        {
            pStatus->AddWarning(
                E_DEF_FSI_SPECIAL_FILE_IGNORED,
                pEntry->valueTypeName.GetRef(),
                0,
                file._wszFullFilePath.c_str(),
                static_cast<std::uint32_t>(ignoredAttributes));
            continue;
        }

        const std::size_t slash = file._wszFullFilePath.find_last_of(L"\\", std::wstring::npos);
        if (slash == std::wstring::npos)
        {
            RETURN_HR(E_FAIL);
        }
        fileName.append(file._wszFullFilePath.substr(slash + 1, std::wstring::npos), 0, std::wstring::npos);
        result = _CreateFileEntry(pEntry, fileName, itemName, value, &instance, pStatus);
        RETURN_IF_FAILED(result);
        AutoDeletePtr<CItemInstanceEntry> newEntry(
            CItemInstanceEntry::NewForString(
                L"Files",
                instance.pItemName,
                instance.itemType,
                instance.resourceValueType,
                instance.pValue,
                instance.conditionSetIndex,
                instance.uiActionFlags,
                nullptr,
                nullptr,
                pStatus));
        if (newEntry.Data() == nullptr)
        {
            result = pStatus->GetHResult();
            RETURN_IF_FAILED(result);
            return result;
        }
        result = pSink->AddEntry(newEntry.Data());
        RETURN_IF_FAILED(result);
        newEntry.Detach();
    }
    return S_OK;
}

HRESULT CFolderIndexer::_CreateFileEntry(
    CItemInstanceEntry* const pEntry,
    const std::wstring& wszFileName,
    std::wstring& pwszItemName,
    std::wstring& pwszValue,
    ITEM_INSTANCE_ENTRY* const pLocalEntry,
    IDefStatusEx* const pStatus)
{
    CExclusionResult exclusionResult {};
    bool qualifierApplied = false;
    wchar_t lowercaseBuffer[MAX_PATH] {};
    LCMapStringEx(
        LOCALE_NAME_INVARIANT,
        LCMAP_LOWERCASE,
        wszFileName.c_str(),
        static_cast<int>(wszFileName.length()),
        lowercaseBuffer,
        MAX_PATH,
        nullptr,
        nullptr,
        0);
    std::wstring lowercaseFileName;
    lowercaseFileName.assign(lowercaseBuffer, wcslen(lowercaseBuffer));

    const wchar_t* const parentItemName = pEntry->itemName.GetRef();
    pwszItemName.append(parentItemName);
    if (!pwszItemName.empty() && parentItemName[pwszItemName.length() - 1] != L'\\')
    {
        pwszItemName.append(L"\\");
    }

    if (_pFIXmlConfig->IsFilenameAsDimension())
    {
        const wchar_t* const delimiter = _pFIXmlConfig->GetQualifierDelimiter();
        const std::size_t delimiterLength = wcslen(delimiter);
        std::size_t delimiterPosition = lowercaseFileName.rfind(delimiter, lowercaseFileName.length(), delimiterLength);
        if (*delimiter == L'.')
        {
            delimiterPosition = lowercaseFileName.rfind(delimiter, delimiterPosition - 1, delimiterLength);
        }

        if (delimiterPosition != std::wstring::npos)
        {
            const std::size_t dotPosition = lowercaseFileName.find_first_of(L".", delimiterPosition + 1);
            if (dotPosition != std::wstring::npos)
            {
                const std::size_t tokenLength = dotPosition - delimiterPosition;
                std::wstring token(lowercaseFileName.substr(delimiterPosition + 1, tokenLength - 1).c_str());
                HRESULT result = _pQualApplicator->ApplyQualifier(
                    token.c_str(),
                    pEntry->qualifierSetIndex,
                    CQualifierApplicator::tagTOKEN_TYPE::tokenFile,
                    &pLocalEntry->conditionSetIndex,
                    &qualifierApplied,
                    pStatus);
                RETURN_IF_FAILED(result);
                if (qualifierApplied)
                {
                    if (delimiterPosition == 0)
                    {
                        qualifierApplied = false;
                        pLocalEntry->conditionSetIndex = pEntry->qualifierSetIndex;
                        pStatus->SetError(E_DEF_QUALAPPL_INVALID_QUAL_FILENAME, wszFileName.c_str());
                    }
                    else
                    {
                        std::wstring nameWithoutQualifier(wszFileName);
                        nameWithoutQualifier.replace(delimiterPosition, tokenLength, nullptr, 0);
                        pwszItemName.append(nameWithoutQualifier.c_str());
                    }
                }
            }
        }
    }

    if (!qualifierApplied)
    {
        pwszItemName.append(wszFileName.c_str());
    }
    pLocalEntry->pItemName = pwszItemName.c_str();

    pwszValue.append(pEntry->value.GetRef());
    if (!pwszValue.empty() && pwszValue[pwszValue.length() - 1] != L'\\')
    {
        pwszValue.append(L"\\");
    }
    pwszValue.append(wszFileName.c_str());
    pLocalEntry->pValue = pwszValue.c_str();

    if (SUCCEEDED(_pFIXmlConfig->IsExcludedFile(pEntry->value.GetRef(), lowercaseFileName.c_str(), &exclusionResult)))
    {
        if (exclusionResult.doNotIndex)
        {
            pLocalEntry->uiActionFlags &= ~1u;
        }
        if (exclusionResult.doNotTraverse)
        {
            pLocalEntry->uiActionFlags &= ~2u;
        }
    }
    return ComputeHResult(S_OK, pStatus);
}

HRESULT CFolderIndexer::_CreateFolderEntry(
    CItemInstanceEntry* const pEntry,
    const std::wstring& wszFolderName,
    std::wstring& pwszItemName,
    std::wstring& pwszValue,
    ITEM_INSTANCE_ENTRY* const pLocalEntry,
    IDefStatusEx* const pStatus)
{
    bool qualifierApplied = false;
    if (_pFIXmlConfig->IsFoldernameAsDimension())
    {
        const HRESULT result = _pQualApplicator->ApplyQualifier(
            wszFolderName.c_str(),
            pEntry->qualifierSetIndex,
            CQualifierApplicator::tagTOKEN_TYPE::tokenFolder,
            &pLocalEntry->conditionSetIndex,
            &qualifierApplied,
            pStatus);
        RETURN_IF_FAILED(result);
    }

    if (qualifierApplied)
    {
        pLocalEntry->pItemName = pEntry->itemName.GetRef();
    }
    else
    {
        pwszItemName.append(pEntry->itemName.GetRef());
        if (!pwszItemName.empty() && pwszItemName[pwszItemName.length() - 1] != L'\\')
        {
            pwszItemName.append(L"\\");
        }
        pwszItemName.append(wszFolderName.c_str());
        pLocalEntry->pItemName = pwszItemName.c_str();
    }

    pwszValue.append(pEntry->value.GetRef());
    if (!pwszValue.empty() && pwszValue[pwszValue.length() - 1] != L'\\')
    {
        pwszValue.append(L"\\");
    }
    pwszValue.append(wszFolderName.c_str());
    pLocalEntry->pValue = pwszValue.c_str();

    CExclusionResult exclusionResult {};
    if (SUCCEEDED(_pFIXmlConfig->IsExcludedFolder(pEntry->value.GetRef(), wszFolderName.c_str(), &exclusionResult)))
    {
        if (exclusionResult.doNotIndex)
        {
            pLocalEntry->uiActionFlags &= ~1u;
        }
        if (exclusionResult.doNotTraverse)
        {
            pLocalEntry->uiActionFlags &= ~2u;
        }
    }
    return S_OK;
}

HRESULT CFolderIndexer::_Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(pEnvironment);
    HRESULT result = S_OK;
    if (_pFIXmlConfig == nullptr)
    {
        _pFolderInfo = new (std::nothrow) CFolderInfo();
        if (_pFolderInfo != nullptr)
        {
            result = _pFolderInfo->Set(pProjectRootFolder, pStatus);
            if (SUCCEEDED(result))
            {
                _pFIXmlConfig = new (std::nothrow) CFIXmlConfig(pIndexPassNode);
                if (_pFIXmlConfig != nullptr)
                {
                    result = _pFIXmlConfig->Parse(pStatus);
                    if (SUCCEEDED(result))
                    {
                        return result;
                    }
                    _pFIXmlConfig->CFIXmlConfig::~CFIXmlConfig();
                    ::operator delete(_pFIXmlConfig);
                    _pFIXmlConfig = nullptr;
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
            }

            _bError = true;
            std::wostringstream output;
            output << L"Init failed:";
            if (pStatus->Failed())
            {
                output << L"DefStatus:" << pStatus->GetDefStatus();
            }
            else
            {
                output << result;
            }
            output << std::endl;
            DebugOutput(output.str().c_str());
        }
    }
    return result;
}

void CFolderIndexer::_InitInstanceEntry(CItemInstanceEntry* const pEntry, ITEM_INSTANCE_ENTRY* const pLocalEntry) const
{
    std::memset(pLocalEntry, 0, sizeof(*pLocalEntry));
    pLocalEntry->itemType = pEntry->resourceItemType;
    pLocalEntry->resourceValueType = pEntry->resourceValueType;
    pLocalEntry->pCollectionName = pEntry->source.GetRef();
    pLocalEntry->conditionSetIndex = pEntry->qualifierSetIndex;
    pLocalEntry->uiActionFlags = 3;
}
} // namespace Microsoft::Resources::Indexers
