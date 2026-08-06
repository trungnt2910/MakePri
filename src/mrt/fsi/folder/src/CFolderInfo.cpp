#include "StdAfx.h"

#include <CFolderInfo.h>

namespace Microsoft::Resources::Indexers
{
CFolderInfo::CFolderInfo() : _pFolderList(nullptr), _pFileList(nullptr), _ulFolderAttribute(0) {}

CFolderInfo::~CFolderInfo()
{
    delete _pFolderList;
    delete _pFileList;
}

HRESULT CFolderInfo::GetCFileInfo(const std::uint32_t index, CFileInfo* const pCFileInfo, IDefStatusEx* const pStatus) const
{
    const std::uint32_t uiCount = _pFileList != nullptr ? static_cast<std::uint32_t>(_pFileList->size()) : 0;
    if (index >= uiCount)
    {
        return E_FAIL;
    }
    const FILE_ATTRIUBTE fileAttribute = _pFileList->at(index);
    std::wstring wszFolderName(_wszFolder);
    wszFolderName.append(fileAttribute.wszFileName);
    return pCFileInfo->Set(wszFolderName.c_str(), fileAttribute.ulAttribute, fileAttribute.ulReserved, pStatus);
}

HRESULT CFolderInfo::GetCFolderInfo(const std::uint32_t index, CFolderInfo* const pCFolderInfo, IDefStatusEx* const pStatus) const
{
    const std::uint32_t uiCount = _pFolderList != nullptr ? static_cast<std::uint32_t>(_pFolderList->size()) : 0;
    if (index >= uiCount)
    {
        return E_FAIL;
    }
    const FILE_ATTRIUBTE fileAttribute = _pFolderList->at(index);
    std::wstring wszFolderName(_wszFolder);
    wszFolderName.append(fileAttribute.wszFileName);
    return pCFolderInfo->Set(wszFolderName.c_str(), fileAttribute.ulAttribute, fileAttribute.ulReserved, pStatus);
}

HRESULT CFolderInfo::IsSpecialFileOrFolderToIgnore(
    const DWORD attributes,
    const DWORD reserved,
    const IIndexOptions* const options,
    FOLDER_FILE_ATTRIBUTES* const specialAttributes)
{
    HRESULT status = E_FAIL;
    auto value = 0u;
    *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
    if ((attributes & FILE_ATTRIBUTE_HIDDEN) != 0 && !options->GetShouldProcessHiddenFiles())
    {
        value |= static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::HIDDEN_FILE);
        *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
        status = S_OK;
    }
    if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 || options->GetShouldProcessLinkedFiles())
    {
        return status;
    }

    value |= static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::REPARSE_POINT);
    *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
    if ((attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) == (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
    {
        value |= static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::JUNCTION_POINT);
        *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
    }
    if ((reserved & IO_REPARSE_TAG_DFS) != 0)
    {
        value |= static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::DFS);
        *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
    }
    if ((reserved & IO_REPARSE_TAG_SYMLINK) != 0)
    {
        value |= static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::SYMBOLIC_LINK);
        *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value);
    }
    if ((reserved & IO_REPARSE_TAG_MOUNT_POINT) != 0)
    {
        *specialAttributes = static_cast<FOLDER_FILE_ATTRIBUTES>(value | static_cast<std::uint32_t>(FOLDER_FILE_ATTRIBUTES::VOLUME_MOUNT));
    }
    return S_OK;
}

HRESULT CFileInfo::Set(const wchar_t* const pFilePath, const DWORD ulAttribute, const DWORD ulReserved, IDefStatusEx* const pStatus)
{
    _ulReserved = ulReserved;
    if (ulAttribute == INVALID_FILE_ATTRIBUTES || (ulAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        pStatus->SetError(E_DEF_FSI_INVALID_FILE_TYPE, pFilePath);
    }
    else
    {
        _wszFullFilePath.clear();
        _wszFullFilePath.append(pFilePath);
        if (_wszFullFilePath.length() >= 0x7FA4)
        {
            pStatus->SetError(E_DEF_FSI_FILE_PATH_TOO_LONG, pFilePath);
            _wszFullFilePath.clear();
        }
        else
        {
            _ulFileAttribute = ulAttribute;
        }
    }
    return ComputeHResult(S_OK, pStatus);
}

HRESULT CFolderInfo::Set(const wchar_t* const pFolder, const DWORD ulAttribute, const DWORD ulReserved, IDefStatusEx* const pStatus)
{
    if (_wszFolderAbsolutePath.empty())
    {
        const wchar_t* convertedPath = nullptr;
        if (SUCCEEDED(CUtilities::GetPathInAccessibleFormat(pFolder, &convertedPath)))
        {
            _wszFolderAbsolutePath.append(convertedPath);
        }
        delete[] convertedPath;
    }
    if (_wszFolder.empty())
    {
        _wszFolder.append(pFolder);
    }

    if (ulAttribute == INVALID_FILE_ATTRIBUTES)
    {
        pStatus->SetError(E_DEF_FSI_INVALID_FILE_TYPE, pFolder);
    }
    else if ((ulAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        if (pFolder[wcslen(pFolder) - 1] != L'\\')
        {
            _wszFolder.append(L"\\");
        }
        if (_wszFolderAbsolutePath[_wszFolderAbsolutePath.length() - 1] != L'\\')
        {
            _wszFolderAbsolutePath.append(L"\\");
        }
        if (_wszFolderAbsolutePath.length() >= 0x7FA0)
        {
            pStatus->SetError(E_DEF_FSI_FILE_PATH_TOO_LONG, pFolder);
        }
        else
        {
            _ulFolderAttribute = ulAttribute;
            _ulReserved = ulReserved;
        }
    }
    else
    {
        pStatus->SetError(E_DEF_FSI_UNSUPPORTED_DIR_TYPE, pFolder);
    }

    const HRESULT result = ComputeHResult(S_OK, pStatus);
    if (FAILED(result))
    {
        _wszFolderAbsolutePath.clear();
        _wszFolder.clear();
    }
    return result;
}

HRESULT CFolderInfo::Set(const wchar_t* const pFolder, IDefStatusEx* const pStatus)
{
    const wchar_t* accessiblePath = nullptr;
    HRESULT result = CUtilities::GetPathInAccessibleFormat(pFolder, &accessiblePath);
    if (SUCCEEDED(result))
    {
        _wszFolder.clear();
        _wszFolder.append(pFolder);
        _wszFolderAbsolutePath.clear();
        _wszFolderAbsolutePath.append(accessiblePath);
        result = Set(pFolder, GetFileAttributesW(_wszFolderAbsolutePath.c_str()), 0, pStatus);
    }
    delete[] accessiblePath;
    return result;
}

HRESULT CFolderInfo::TraverseChildren()
{
    HRESULT result = S_OK;
    std::wstring wszSearchFolder(_wszFolderAbsolutePath);
    wszSearchFolder.append(L"*");
    WIN32_FIND_DATAW fileData;
    const HANDLE hFind = FindFirstFileW(wszSearchFolder.c_str(), &fileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        result = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        _pFolderList = new (std::nothrow) std::vector<FILE_ATTRIUBTE>();
        _pFileList = new (std::nothrow) std::vector<FILE_ATTRIUBTE>();
        FILE_ATTRIUBTE fileAttribute;
        while (FindNextFileW(hFind, &fileData))
        {
            if ((DefString_CompareWithOptions(fileData.cFileName, L".", DefCompare_CaseInsensitive) != Def_Equal) &&
                (DefString_CompareWithOptions(fileData.cFileName, L"..", DefCompare_CaseInsensitive) != Def_Equal))
            {
                fileAttribute.wszFileName.append(fileData.cFileName);
                fileAttribute.ulAttribute = fileData.dwFileAttributes;
                fileAttribute.ulReserved = fileData.dwReserved0;
                if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    _pFolderList->push_back(fileAttribute);
                }
                else
                {
                    _pFileList->push_back(fileAttribute);
                }
                fileAttribute.ulAttribute = 0;
                fileAttribute.wszFileName.clear();
            }
        }
        if (GetLastError() != ERROR_NO_MORE_FILES)
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }
        FindClose(hFind);
    }
    if (FAILED(result))
    {
        LogSink::WriteError(_wszFolderAbsolutePath.c_str(), L"Folder traversal", L"error", result, L"");
    }
    return result;
}
} // namespace Microsoft::Resources::Indexers
