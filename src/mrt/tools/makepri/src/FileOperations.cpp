#include "StdAfx.h"

#include <ParameterManager.h>

namespace Microsoft::Resources::Tools::MakePri
{
namespace
{
HRESULT LastErrorToHResult() { return HRESULT_FROM_WIN32(GetLastError()); }
} // namespace

HRESULT FileOperations::MoveFiles(const bool overwrite, IDefStatusEx* const status)
{
    std::list<std::wstring> sourceFiles;
    HRESULT hr = _GetListOfSourceFiles(status, sourceFiles);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!overwrite)
    {
        std::list<std::wstring> filesToOverwrite;
        hr = _GetListOfFilesToOverwrite(sourceFiles, status, filesToOverwrite);
        if (SUCCEEDED(hr) && !filesToOverwrite.empty())
        {
            hr = _AskOverwritePermission(filesToOverwrite);
        }
        if (FAILED(hr))
        {
            return hr;
        }
    }
    return _MoveAllFiles(sourceFiles, status);
}

HRESULT FileOperations::_AskOverwritePermission(const std::list<std::wstring>& files)
{
    Log::NewLine();
    Log::WriteWorkOutput(MAKEPRI_STRING_FOLLOWING_FILE_S_ALREADY_EXIST_AT);
    for (const std::wstring& file : files)
    {
        Log::ItemWithFormat(0, MrmResourceIndexerMessageSeverityInfo, L"%s", file.c_str());
    }
    Log::WriteWorkOutput(MAKEPRI_STRING_OVERWRITE_THESE_FILE_S_Y_ES);
    Log::Flush();

    const wint_t input = _getwche();
    std::fwprintf(stdout, L"\n");
    wchar_t yes[MAX_PATH] {};
    LoadStringW(nullptr, MAKEPRI_STRING_YES, yes, MAX_PATH);
    return std::towupper(input) == yes[0] ? S_OK : E_ABORT;
}

HRESULT FileOperations::_GetListOfFilesToOverwrite(
    const std::list<std::wstring>& sourceFiles,
    IDefStatusEx* const status,
    std::list<std::wstring>& files)
{
    static_cast<void>(status);

    DEFSTRINGRESULT searchPath {};
    DefStringResult_InitBuf(&searchPath, nullptr);
    HRESULT hr = DefStringResult_InitRef(&searchPath, m_destinationFolder);
    if (FAILED(hr))
    {
        DefStringResult_Clear(&searchPath, true);
        return hr;
    }
    DefStringResult_Concat(&searchPath, L"\\*");

    const wchar_t* searchReference {};
    DefStringResult_GetRef(&searchPath, &searchReference);
    WIN32_FIND_DATAW findData {};
    const HANDLE find = FindFirstFileW(searchReference, &findData);
    if (find == INVALID_HANDLE_VALUE)
    {
        hr = LastErrorToHResult();
    }
    else
    {
        while (FindNextFileW(find, &findData))
        {
            if ((DefString_CompareWithOptions(findData.cFileName, L".", DefCompare_CaseInsensitive) == Def_Equal) ||
                (DefString_CompareWithOptions(findData.cFileName, L"..", DefCompare_CaseInsensitive) == Def_Equal))
            {
                continue;
            }

            for (const std::wstring& sourceFile : sourceFiles)
            {
                if (DefString_CompareWithOptions(sourceFile.c_str(), findData.cFileName, DefCompare_CaseInsensitive) == Def_Equal)
                {
                    files.emplace_back(findData.cFileName);
                }
            }
        }
        FindClose(find);
    }

    DefStringResult_Clear(&searchPath, true);
    return hr;
}

HRESULT FileOperations::_GetListOfSourceFiles(IDefStatusEx* const status, std::list<std::wstring>& files)
{
    static_cast<void>(status);

    DEFSTRINGRESULT searchPath {};
    DefStringResult_InitBuf(&searchPath, nullptr);
    HRESULT hr = DefStringResult_InitRef(&searchPath, m_sourceFolder);
    if (FAILED(hr))
    {
        DefStringResult_Clear(&searchPath, true);
        return hr;
    }
    DefStringResult_Concat(&searchPath, L"\\*");

    const wchar_t* searchReference {};
    DefStringResult_GetRef(&searchPath, &searchReference);
    WIN32_FIND_DATAW findData {};
    const HANDLE find = FindFirstFileW(searchReference, &findData);
    if (find == INVALID_HANDLE_VALUE)
    {
        hr = LastErrorToHResult();
    }
    else
    {
        while (FindNextFileW(find, &findData))
        {
            if ((DefString_CompareWithOptions(findData.cFileName, L".", DefCompare_CaseInsensitive) != Def_Equal) &&
                (DefString_CompareWithOptions(findData.cFileName, L"..", DefCompare_CaseInsensitive) != Def_Equal))
            {
                files.emplace_back(findData.cFileName);
            }
        }
        FindClose(find);
    }

    DefStringResult_Clear(&searchPath, true);
    return hr;
}

HRESULT FileOperations::_MoveAllFiles(const std::list<std::wstring>& files, IDefStatusEx* const status)
{
    HRESULT hr = S_OK;
    DEFSTRINGRESULT source {};
    DEFSTRINGRESULT destination {};
    DefStringResult_InitBuf(&source, nullptr);
    DefStringResult_InitBuf(&destination, nullptr);

    for (const std::wstring& file : files)
    {
        DefStringResult_SetCopy(&source, m_sourceFolder);
        DefStringResult_ConcatPathElement(&source, file.c_str(), L'\\');
        DefStringResult_SetCopy(&destination, m_destinationFolder);
        DefStringResult_ConcatPathElement(&destination, file.c_str(), L'\\');

        const wchar_t* sourceReference {};
        const wchar_t* destinationReference {};
        DefStringResult_GetRef(&source, &sourceReference);
        DefStringResult_GetRef(&destination, &destinationReference);
        hr = _MoveFile(sourceReference, destinationReference, status);
        if (FAILED(hr))
        {
            break;
        }
    }

    DefStringResult_Clear(&destination, true);
    DefStringResult_Clear(&source, true);
    return hr;
}

HRESULT FileOperations::_MoveFile(const wchar_t* const source, const wchar_t* const destination, IDefStatusEx* const status)
{
    status->DiagnosticLogA("Attempting to copy [%S] to [%S]", source, destination);
    HRESULT hr = S_OK;
    if (!CopyFileExW(source, destination, nullptr, nullptr, nullptr, 0))
    {
        hr = LastErrorToHResult();
        status->DiagnosticLogWithErrorCodeA("CopyFile failed", hr);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        status->DiagnosticLogA("[%S] successfully copied to [%S]", source, destination);
    }

    if (DeleteFileW(source))
    {
        status->DiagnosticLogA("[%S] successfully deleted", source);
        return hr;
    }

    HRESULT deleteError = LastErrorToHResult();
    status->DiagnosticLogWithErrorCodeA("DeleteFile first attempt failed.", deleteError);
    Sleep(500);
    if (DeleteFileW(source))
    {
        status->DiagnosticLogA("[%S] successfully deleted", source);
    }
    else
    {
        deleteError = LastErrorToHResult();
        status->DiagnosticLogWithErrorCodeA("DeleteFile second attempt failed.", deleteError);
    }
    return hr;
}

HRESULT FileOperations::s_DeleteFolderAndContents(const wchar_t* const path, IDefStatusEx* const status)
{
    static_cast<void>(status);

    HRESULT hr = S_OK;
    std::wstring searchPath(path);
    searchPath.append(L"\\*");
    WIN32_FIND_DATAW findData {};
    const HANDLE find = FindFirstFileW(searchPath.c_str(), &findData);
    if (find != INVALID_HANDLE_VALUE)
    {
        BOOL next {};
        do
        {
            next = FindNextFileW(find, &findData);
            if (std::wcscmp(findData.cFileName, L".") != 0 && std::wcscmp(findData.cFileName, L"..") != 0)
            {
                std::wstring child(path);
                child.append(findData.cFileName);
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    hr = s_DeleteFolderAndContents(child.c_str(), status);
                    if (SUCCEEDED(hr))
                    {
                        RemoveDirectoryW(child.c_str());
                    }
                }
                else
                {
                    DeleteFileW(child.c_str());
                }
            }
        } while (next);
        FindClose(find);
    }
    return hr;
}

HRESULT FileOperations::s_CreateUniqueTempFolder(IDefStatusEx* const status, StringResult* const result)
{
    static_cast<void>(status);

    wchar_t* tempPath {};
    std::size_t capacity {};
    HRESULT hr = result->SetEmptyContents(MAX_PATH, &tempPath, &capacity);
    if (FAILED(hr))
    {
        return hr;
    }
    if (GetTempPathW(static_cast<DWORD>(capacity), tempPath) == 0)
    {
        return LastErrorToHResult();
    }

    UUID uuid {};
    RPC_STATUS rpcStatus = UuidCreate(&uuid);
    if (rpcStatus != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcStatus);
    }

    RPC_WSTR uuidText {};
    rpcStatus = UuidToStringW(&uuid, &uuidText);
    if (rpcStatus != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcStatus);
    }

    wchar_t formattedUuid[MAX_PATH] {};
    hr = StringCchPrintfW(formattedUuid, MAX_PATH, L"%s", reinterpret_cast<const wchar_t*>(uuidText));
    RpcStringFreeW(&uuidText);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = DefStringResult_ConcatPathElement(result->GetStringResult(), formattedUuid, L'\\');
    if (FAILED(hr))
    {
        return hr;
    }

    if (result->GetLength() > MAX_PATH)
    {
        const wchar_t* accessiblePath {};
        hr = Indexers::CUtilities::GetPathInAccessibleFormat(result->GetRef(), &accessiblePath);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = result->SetContents(const_cast<wchar_t*>(accessiblePath), static_cast<std::uint32_t>(std::wcslen(accessiblePath)));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (!CreateDirectoryW(result->GetRef(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return LastErrorToHResult();
    }
    return S_OK;
}
} // namespace Microsoft::Resources::Tools::MakePri

namespace Microsoft::Resources
{
HRESULT ComputeHResult(const HRESULT result, IDefStatusEx* const status)
{
    if (SUCCEEDED(result))
    {
        return status->GetHResult();
    }
    return result;
}
} // namespace Microsoft::Resources
