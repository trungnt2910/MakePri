#include "StdAfx.h"

#include <ItemInstanceSink.h>

namespace Microsoft::Resources::Indexers
{
namespace
{

constexpr wchar_t ExtendedPathPrefix[] = L"\\\\?\\";
constexpr wchar_t ExtendedUncPathPrefix[] = L"\\\\?\\UNC\\";

template<typename Iterator>
std::wstring DelimitedStringFromItems(Iterator begin, const Iterator end)
{
    std::wstring result;
    bool first = true;
    while (begin != end)
    {
        if (!begin->empty())
        {
            if (!first)
            {
                result.append(1, L',');
            }
            result.append(*begin);
            first = false;
        }
        ++begin;
    }
    return result;
}

} // namespace

extern "C" HRESULT WINAPI FormatLanguageTag(const wchar_t* languageTag, int maximumLength, const wchar_t* source, wchar_t* result);

bool AtomComparator::operator()(const Atom& left, const Atom& right) const
{
    if (left.GetPoolIndex() == right.GetPoolIndex())
    {
        return left.GetIndex() < right.GetIndex();
    }
    return left.GetPoolIndex() < right.GetPoolIndex();
}

bool QualifierResultComparator::operator()(const QualifierResult& left, const QualifierResult& right) const
{
    int qualifierIndex;
    left.GetQualifierIndex(&qualifierIndex);
    right.GetQualifierIndex(&qualifierIndex);

    StringResult leftValue;
    StringResult rightValue;
    bool result = false;
    if (SUCCEEDED(left.GetOperand2Literal(&leftValue)) && SUCCEEDED(right.GetOperand2Literal(&rightValue)))
    {
        DEFCOMPARISON comparison;
        leftValue.CompareWithOptions(&rightValue, DefCompare_CaseInsensitive, &comparison);
        if (comparison != Def_Equal)
        {
            if (left.GetFallbackScoreAsScaledInt() == right.GetFallbackScoreAsScaledInt())
            {
                result = comparison == Def_Less;
            }
            else
            {
                result = left.GetFallbackScoreAsScaledInt() > right.GetFallbackScoreAsScaledInt();
            }
        }
    }
    return result;
}

bool CUtilities::GetVersionFromString(const wchar_t* const version, const wchar_t* const platform, MrmPlatformVersionInternal* const result)
{
    if (std::wcscmp(version, L"6.3.1") == 0)
    {
        if (platform == nullptr || DefString_CompareWithOptions(platform, L"WindowsPhone", DefCompare_CaseInsensitive) == Def_Equal)
        {
            *result = MrmPlatformVersionInternal::WindowsPhoneBlue;
            return true;
        }
        return false;
    }

    if (std::wcscmp(version, L"6.2.1") == 0)
    {
        *result = MrmPlatformVersionInternal::WindowsClient8;
    }
    else if (std::wcscmp(version, L"6.3") == 0 || std::wcscmp(version, L"6.3.0") == 0)
    {
        *result = MrmPlatformVersionInternal::WindowsClientBlue;
    }
    else if (std::wcscmp(version, L"10.0") == 0 || std::wcscmp(version, L"10.0.0") == 0)
    {
        *result = MrmPlatformVersionInternal::WindowsCore;
    }
    else if (std::wcscmp(version, L"10.0.0.5") == 0)
    {
        *result = MrmPlatformVersionInternal::WindowsCoreRS4;
    }
    else if (std::wcscmp(version, L"99.0.1") == 0)
    {
        *result = MrmPlatformVersionInternal::WindowsCoreVNext;
    }
    else
    {
        return false;
    }
    return true;
}

HRESULT CUtilities::CheckIfFileOrFolder(const wchar_t* const path, IDefStatusEx* const status, bool* const isFile)
{
    const DWORD attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        status->SetError(E_DEF_FSI_INVALID_FILE_TYPE, path);
    }
    else
    {
        *isFile = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
    return status->GetHResult();
}

HRESULT CUtilities::GetAbsolutePath(const wchar_t* const path, IDefStatusEx* const status, StringResult& result)
{
    const DWORD requiredLength = GetFullPathNameW(path, 0, nullptr, nullptr);
    RETURN_LAST_ERROR_IF(requiredLength == 0);

    wchar_t* buffer = nullptr;
    std::size_t capacity = 0;
    static_cast<void>(Def_HrFailed0(result.SetEmptyContents(requiredLength, &buffer, &capacity), status));
    RETURN_IF_FAILED(status->GetHResult());

    RETURN_HR_IF(E_UNEXPECTED, GetFullPathNameW(path, static_cast<DWORD>(capacity), buffer, nullptr) > requiredLength - 1);
    return S_OK;
}

HRESULT CUtilities::AdjustForProjectRoot(const wchar_t* const projectRoot, IDefStatusEx* const status, std::wstring& path)
{
    const wchar_t* normalizedProjectRoot = projectRoot;
    if (std::wcscmp(normalizedProjectRoot, L"") == 0)
    {
        normalizedProjectRoot = L".\\";
    }

    StringResult absoluteProjectRoot;
    const wchar_t* currentProjectRoot = normalizedProjectRoot;
    if (PathIsRelativeW(normalizedProjectRoot))
    {
        RETURN_IF_FAILED(GetAbsolutePath(normalizedProjectRoot, status, absoluteProjectRoot));
        currentProjectRoot = absoluteProjectRoot.GetRef();
    }

    HRESULT result = wil::ResultFromException([&] {
        std::wstring relativeFrom;
        if (!PathIsRelativeW(path.c_str()))
        {
            relativeFrom.assign(currentProjectRoot);
            constexpr std::size_t prefixLength = std::size(ExtendedPathPrefix) - 1;
            if (path.substr(0, prefixLength).compare(ExtendedPathPrefix) == 0 &&
                relativeFrom.substr(0, prefixLength).compare(ExtendedPathPrefix) != 0)
            {
                relativeFrom.insert(0, ExtendedPathPrefix, prefixLength);
            }
        }
        else if (PathIsRelativeW(normalizedProjectRoot))
        {
            relativeFrom.assign(normalizedProjectRoot);
        }

        StringResult relativePath;
        wchar_t* relativePathBuffer = nullptr;
        std::size_t relativePathCapacity = 0;
        static_cast<void>(Def_HrFailed0(
            relativePath.SetEmptyContents(relativeFrom.length() + path.length(), &relativePathBuffer, &relativePathCapacity), status));

        if (status->Succeeded() &&
            PathRelativePathToW(relativePathBuffer, relativeFrom.c_str(), FILE_ATTRIBUTE_DIRECTORY, path.c_str(), FILE_ATTRIBUTE_NORMAL))
        {
            const wchar_t* adjustedPath = relativePathBuffer;
            if (adjustedPath[0] == L'.' && adjustedPath[1] == L'\\')
            {
                adjustedPath += 2;
            }
            path.clear();
            path.append(adjustedPath);
        }
    });

    result = ComputeHResult(result, status);
    RETURN_IF_FAILED(result);
    return S_OK;
}

HRESULT CUtilities::GetPathInAccessibleFormat(
    const wchar_t* const projectRoot,
    const wchar_t* const path,
    IDefStatusEx* const status,
    const wchar_t** const accessiblePath)
{
    StringResult combinedPath;
    static_cast<void>(Def_HrFailed0(combinedPath.SetRef(path), status));

    bool isAbsolute = false;
    combinedPath.IsAbsolutePath(L'\\', &isAbsolute);
    const wchar_t* pathToConvert = path;
    if (!isAbsolute)
    {
        static_cast<void>(Def_HrFailed0(DefStringResult_SetCopy(combinedPath.GetStringResult(), projectRoot), status));
        static_cast<void>(Def_HrFailed0(DefStringResult_ConcatPathElement(combinedPath.GetStringResult(), path, L'\\'), status));
        pathToConvert = combinedPath.GetRef();
    }

    return GetPathInAccessibleFormat(pathToConvert, accessiblePath);
}

HRESULT CUtilities::GetPathInAccessibleFormat(const wchar_t* const path, const wchar_t** const accessiblePath)
{
    *accessiblePath = nullptr;

    const std::size_t pathLength = std::wcslen(path);
    const wchar_t* prefix = nullptr;
    bool isNetworkPath = false;
    if (pathLength > MAX_PATH)
    {
        prefix = ExtendedPathPrefix;
        const std::size_t prefixLength = wcsnlen(ExtendedPathPrefix, MAX_PATH);
        if (std::wcsncmp(path, ExtendedPathPrefix, prefixLength) == 0)
        {
            const std::size_t bufferLength = pathLength + 1;
            AutoDeletePtr<wchar_t> buffer(new (std::nothrow) wchar_t[bufferLength]());
            RETURN_IF_FAILED(StringCchCopyW(buffer.Data(), bufferLength, path));
            *accessiblePath = buffer.Detach();
            return S_OK;
        }

        isNetworkPath = PathIsNetworkPathW(path) != FALSE;
        if (isNetworkPath)
        {
            prefix = ExtendedUncPathPrefix;
        }
    }

    DWORD fullPathLength = GetFullPathNameW(path, 0, nullptr, nullptr);
    RETURN_LAST_ERROR_IF(fullPathLength == 0);

    DWORD bufferLength = fullPathLength + 1;
    std::size_t prefixLength = 0;
    AutoDeletePtr<wchar_t> buffer;
    if (prefix != nullptr)
    {
        prefixLength = wcsnlen(prefix, MAX_PATH);
        bufferLength += static_cast<DWORD>(prefixLength);
        buffer.Set(new (std::nothrow) wchar_t[bufferLength]());
        RETURN_IF_FAILED(StringCchCopyW(buffer.Data(), bufferLength, prefix));
    }
    else
    {
        buffer.Set(new (std::nothrow) wchar_t[bufferLength]());
    }

    const DWORD remainingLength = bufferLength - static_cast<DWORD>(prefixLength);
    if (isNetworkPath)
    {
        RETURN_IF_FAILED(StringCchCopyW(buffer.Data() + prefixLength, remainingLength, path + 2));
    }
    else
    {
        RETURN_LAST_ERROR_IF(GetFullPathNameW(path, remainingLength, buffer.Data() + prefixLength, nullptr) == 0);
    }

    *accessiblePath = buffer.Detach();
    return S_OK;
}

HRESULT
CUtilities::LoadFile(const wchar_t* const path, std::wstring& contents, IDefStatusEx* const status)
{
    std::FILE* const file = _wfopen(path, L"r, ccs=UTF-8");
    HRESULT result = S_OK;
    if (file != nullptr)
    {
        contents.clear();
        while (true)
        {
            const wint_t character = std::fgetwc(file);
            if (character == WEOF)
            {
                break;
            }
            contents.append(1, static_cast<wchar_t>(character));
        }
    }
    else
    {
        result = ErrnoToHResult(errno);
        if (result == E_FAIL)
        {
            result = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_FILE);
        }
        status->SetError(result, path);
    }

    std::fclose(file);
    return ComputeHResult(result, status);
}

HRESULT CUtilities::ReadUnicodeTextFile(const wchar_t* const path, wchar_t** const contents, ULONG* const unusedSize, const int unusedFlags)
{
    static_cast<void>(unusedSize);
    static_cast<void>(unusedFlags);

    *contents = nullptr;
    HRESULT result = S_OK;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        result = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(result))
        {
            return result;
        }
    }

    DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE)
    {
        result = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(result))
        {
            CloseHandle(file);
            return result;
        }
    }

    DWORD remainingSize = fileSize;
    UINT codePage = CP_ACP;
    bool isUtf16 = false;
    if (fileSize > 2)
    {
        WORD byteOrderMark = 0;
        DWORD bytesRead = 0;
        if (!ReadFile(file, &byteOrderMark, sizeof(byteOrderMark), &bytesRead, nullptr))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }
        if (FAILED(result))
        {
            CloseHandle(file);
            return result;
        }
        if (bytesRead != sizeof(byteOrderMark))
        {
            CloseHandle(file);
            return E_FAIL;
        }

        if (byteOrderMark == 0xFEFF)
        {
            isUtf16 = true;
            remainingSize = fileSize - 2;
        }
        else if (fileSize > 3 && byteOrderMark == 0xBBEF)
        {
            BYTE finalByteOrderMark = 0;
            if (!ReadFile(file, &finalByteOrderMark, sizeof(finalByteOrderMark), &bytesRead, nullptr))
            {
                result = HRESULT_FROM_WIN32(GetLastError());
            }
            if (FAILED(result))
            {
                CloseHandle(file);
                return result;
            }
            if (bytesRead != sizeof(finalByteOrderMark))
            {
                CloseHandle(file);
                return E_FAIL;
            }

            if (finalByteOrderMark == 0xBF)
            {
                codePage = CP_UTF8;
                remainingSize = fileSize - 3;
            }
            else if (SetFilePointer(file, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            {
                result = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else if (SetFilePointer(file, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (FAILED(result))
    {
        CloseHandle(file);
        return result;
    }

    if (isUtf16)
    {
        const DWORD characterCount = (remainingSize / 2) + 1;
        *contents = new (std::nothrow) wchar_t[characterCount];
        if (*contents == nullptr)
        {
            CloseHandle(file);
            return E_OUTOFMEMORY;
        }

        DWORD bytesRead = 0;
        if (!ReadFile(file, *contents, remainingSize, &bytesRead, nullptr))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }

        if (FAILED(result))
        {
            delete[] *contents;
        }
        else
        {
            (*contents)[bytesRead / 2] = L'\0';
        }

        CloseHandle(file);
        return result;
    }

    auto* bytes = new (std::nothrow) char[remainingSize + 1];
    if (bytes == nullptr)
    {
        CloseHandle(file);
        return E_OUTOFMEMORY;
    }
    bytes[remainingSize] = '\0';

    DWORD bytesRead = 0;
    if (!ReadFile(file, bytes, remainingSize, &bytesRead, nullptr))
    {
        result = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(result))
    {
        const DWORD wideCharacterCount = remainingSize + 2;
        *contents = new (std::nothrow) wchar_t[wideCharacterCount];
        const int convertedLength = MultiByteToWideChar(codePage, 0, bytes, -1, *contents, static_cast<int>(remainingSize + 1));
        if (convertedLength <= 0)
        {
            delete[] *contents;
        }
        else
        {
            (*contents)[convertedLength] = L'\0';
        }
    }

    delete[] bytes;
    CloseHandle(file);
    return result;
}

bool CUtilities::GetQualifierTagFromQualifierIndex(
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    const int qualifierIndex,
    IDefStatusEx* const status,
    std::wstring& qualifierTag)
{
    QualifierResult qualifier;
    Atom attribute;
    StringResult attributeName;
    StringResult value;

    static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
    if (status->Succeeded())
    {
        ICondition::ConditionOperator conditionOperator;
        static_cast<void>(Def_HrFailed0(qualifier.GetOperator(&conditionOperator), status));
        if (status->Succeeded() && conditionOperator != static_cast<ICondition::ConditionOperator>(1))
        {
            static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
            if (status->Succeeded())
            {
                static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
                {
                    const HRESULT result = wil::ResultFromException([&] {
                        qualifierTag.assign(attributeName.GetRef());
                        qualifierTag.append(L"-");
                        qualifierTag.append(value.GetRef());
                    });
                    if (FAILED(result) && status != nullptr)
                    {
                        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 678, L"", 0);
                    }
                }
            }
        }
    }
    return status->Succeeded();
}

bool CUtilities::GetQualifierTagFromQualifierSetIndex(
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    const int qualifierSetIndex,
    IDefStatusEx* const status,
    std::wstring& qualifierSetTag)
{
    if (qualifierSetIndex == 0)
    {
        qualifierSetTag.assign(L"neutral");
        return true;
    }

    bool result = false;
    QualifierSetResult qualifierSet;
    static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifierSet(qualifierSetIndex, &qualifierSet), status));
    if (status->Succeeded())
    {
        const int numQualifiers = qualifierSet.GetNumQualifiers();
        for (int index = 0; index < numQualifiers && status->Succeeded(); ++index)
        {
            QualifierResult qualifier;
            static_cast<void>(Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, nullptr), status));
            if (status->Succeeded())
            {
                std::wstring qualifierTag;
                int qualifierIndex;
                static_cast<void>(Def_HrFailed0(qualifier.GetQualifierIndex(&qualifierIndex), status));
                if (GetQualifierTagFromQualifierIndex(decisionInfo, atomPoolGroup, qualifierIndex, status, qualifierTag))
                {
                    const HRESULT appendResult = wil::ResultFromException([&] {
                        qualifierSetTag.append(index > 0 ? L"_" : L"");
                        qualifierSetTag.append(qualifierTag);
                    });
                    if (FAILED(appendResult))
                    {
                        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 723, L"", 0);
                    }
                }
            }
        }
    }

    if (status == nullptr || !status->Failed())
    {
        result = true;
    }
    else
    {
        qualifierSetTag.clear();
    }
    return result;
}

HRESULT
CUtilities::GetQualifierMapFromQualifierTag(std::wstring qualifierTag, std::map<std::wstring, std::wstring>* const qualifiers)
{
    return wil::ResultFromException([&] {
        std::size_t position = 0;
        do
        {
            const std::size_t delimiter = qualifierTag.find_first_of(L"_", position);
            const std::wstring item = qualifierTag.substr(position, delimiter - position);
            const std::size_t separator = item.find_first_of(L"-", 0);
            if (separator != std::wstring::npos)
            {
                const std::wstring name = item.substr(0, separator);
                const std::wstring value = item.substr(separator + 1, item.length() - separator - 1);
                qualifiers->insert(std::make_pair(name, value));
            }
            position = delimiter == std::wstring::npos ? std::wstring::npos : delimiter + 1;
        } while (position != std::wstring::npos);
    });
}

HRESULT CUtilities::GetListOfUnusedQualifiers(
    const IDecisionInfo* const decisionInfo,
    CItemInstanceSink* const sink,
    std::list<int>* const unusedQualifiers,
    IDefStatus* const status)
{
    QualifierSetResult qualifierSet;
    const int numQualifiers = decisionInfo->GetNumQualifiers();
    void* const allocation = ::operator new(static_cast<std::size_t>(numQualifiers));
    if (allocation == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    auto* const usedQualifiers = static_cast<std::uint8_t*>(allocation);
    std::memset(usedQualifiers, 0, static_cast<std::size_t>(numQualifiers));
    if (numQualifiers > 0)
    {
        std::memset(usedQualifiers, 0, static_cast<std::size_t>(numQualifiers));
    }

    std::uint32_t entryIndex = 0;
    if (sink->GetNumberOfEntries() != 0)
    {
        do
        {
            if (!status->Succeeded())
            {
                break;
            }

            CItemInstanceEntry* const entry = sink->GetEntry(entryIndex);
            static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifierSet(entry->qualifierSetIndex, &qualifierSet), status));
            if (status->Succeeded())
            {
                int qualifierIndex = 0;
                while (qualifierIndex < qualifierSet.GetNumQualifiers())
                {
                    int qualifierIndexInPool;
                    if (!Def_HrFailed0(qualifierSet.GetQualifierIndexInPool(qualifierIndex, &qualifierIndexInPool), status))
                    {
                        usedQualifiers[qualifierIndexInPool] = 1;
                    }
                    ++qualifierIndex;
                }
            }
            ++entryIndex;
        } while (entryIndex < sink->GetNumberOfEntries());
    }

    for (int qualifierIndex = 0; qualifierIndex < numQualifiers; ++qualifierIndex)
    {
        if (usedQualifiers[qualifierIndex] == 0)
        {
            unusedQualifiers->push_back(qualifierIndex);
        }
    }

    ::operator delete(allocation);
    return static_cast<IDefStatusEx*>(status)->GetHResult();
}

HRESULT CUtilities::GetListOfExcludedQualifiers(
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    std::list<int>* const excludedQualifiers,
    IDefStatus* const status)
{
    int qualifierIndex = 1;
    while (qualifierIndex < decisionInfo->GetNumQualifiers())
    {
        QualifierResult qualifier;
        Atom attribute;
        StringResult attributeName;
        StringResult value;

        static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
        if (status->Succeeded())
        {
            static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
            if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
            {
                DEFCOMPARISON comparison;
                static_cast<void>(DefStringResult_CompareWithOptions(
                    attributeName.GetStringResult(), L"Language", DefCompare_CaseInsensitive, &comparison));
                if (comparison == Def_Equal)
                {
                    static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                    if (status->Succeeded())
                    {
                        if (value.CompareWithOptions(L"und", DefCompare_Default) == Def_Equal || _wcsnicmp(value.GetRef(), L"und-", 4) == 0)
                        {
                            excludedQualifiers->push_back(qualifierIndex);
                        }
                    }
                }
            }
        }
        ++qualifierIndex;
    }
    return static_cast<IDefStatusEx*>(status)->GetHResult();
}

HRESULT CUtilities::GetQualifierStringMap(
    const IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    CItemInstanceSink* const sink,
    std::map<std::wstring, QualifierValues*>* const result,
    IDefStatus* const status,
    const bool includeExcluded)
{
    if (status == nullptr)
    {
        return E_INVALIDARG;
    }
    if (decisionInfo == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 879, L"decisionInfo", 0);
        return E_INVALIDARG;
    }
    if (atomPoolGroup == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 880, L"poolGroup", 0);
        return E_INVALIDARG;
    }
    if (result == nullptr)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 881, L"qualifiers", 0);
        return E_INVALIDARG;
    }

    HRESULT operationResult = S_OK;
    std::list<int> unusedQualifiers;
    if (sink != nullptr)
    {
        operationResult = GetListOfUnusedQualifiers(decisionInfo, sink, &unusedQualifiers, status);
    }

    std::list<int> excludedQualifiers;
    if (SUCCEEDED(operationResult))
    {
        if (!includeExcluded)
        {
            operationResult = GetListOfExcludedQualifiers(decisionInfo, atomPoolGroup, &excludedQualifiers, status);
        }

        if (SUCCEEDED(operationResult))
        {
            std::map<Atom, std::set<QualifierResult, QualifierResultComparator>, AtomComparator> qualifierResults;
            for (int qualifierIndex = 1; qualifierIndex < decisionInfo->GetNumQualifiers(); ++qualifierIndex)
            {
                if (std::find(unusedQualifiers.begin(), unusedQualifiers.end(), qualifierIndex) != unusedQualifiers.end())
                {
                    continue;
                }
                if (std::find(excludedQualifiers.begin(), excludedQualifiers.end(), qualifierIndex) != excludedQualifiers.end())
                {
                    continue;
                }

                QualifierResult qualifier;
                Atom attribute;
                static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
                if (status->Succeeded())
                {
                    static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
                    if (status->Succeeded())
                    {
                        const auto found = qualifierResults.find(attribute);
                        if (found == qualifierResults.end())
                        {
                            std::set<QualifierResult, QualifierResultComparator> values;
                            values.insert(qualifier);
                            qualifierResults.insert(std::make_pair(attribute, values));
                        }
                        else
                        {
                            found->second.insert(qualifier);
                        }
                    }
                }
            }

            static_cast<void>(_GetQualifierStringMap(qualifierResults, atomPoolGroup, status, result));
        }
    }

    if (operationResult == S_OK)
    {
        operationResult = static_cast<IDefStatusEx*>(status)->GetHResult();
    }
    return operationResult;
}

bool CUtilities::_GetQualifierStringMap(
    std::map<Atom, std::set<QualifierResult, QualifierResultComparator>, AtomComparator> qualifierResults,
    AtomPoolGroup* const atomPoolGroup,
    IDefStatus* const status,
    std::map<std::wstring, QualifierValues*>* const result)
{
    auto entry = qualifierResults.begin();
    if (status->Succeeded())
    {
        while (entry != qualifierResults.end())
        {
            StringResult attributeName;
            if (atomPoolGroup->TryGetString(entry->first, &attributeName))
            {
                QualifierValues* const qualifierValues = new (std::nothrow) QualifierValues();
                if (qualifierValues != nullptr)
                {
                    qualifierValues->qualifierNameAtom = entry->first;
                    std::vector<std::wstring> values;
                    std::set<QualifierResult, QualifierResultComparator> qualifiers(entry->second);
                    auto qualifier = qualifiers.begin();
                    if (status->Succeeded())
                    {
                        while (qualifier != qualifiers.end())
                        {
                            StringResult value;
                            static_cast<void>(Def_HrFailed0(qualifier->GetOperand2Literal(&value), status));
                            if (status->Succeeded())
                            {
                                values.push_back(std::wstring(value.GetRef()));
                            }
                            ++qualifier;
                            if (!status->Succeeded())
                            {
                                break;
                            }
                        }
                    }

                    qualifierValues->wstrValues = DelimitedStringFromItems(values.begin(), values.end());
                    const auto inserted = result->insert(std::make_pair(std::wstring(attributeName.GetRef()), qualifierValues));
                    if (!inserted.second)
                    {
                        delete qualifierValues;
                    }
                }
                else
                {
                    status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 1125, L"qualValues", 0);
                }
            }

            ++entry;
            if (!status->Succeeded())
            {
                break;
            }
        }
    }
    return status->Succeeded();
}

bool CUtilities::DisplayQualifierInformation(
    IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    status->DiagnosticLogA(">>>>>>> CUtilities::DisplayQualifierInformation");

    for (int qualifierIndex = 0; qualifierIndex < decisionInfo->GetNumQualifiers(); ++qualifierIndex)
    {
        QualifierResult qualifier;
        Atom attribute;
        StringResult attributeName;
        StringResult value;
        static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
        if (status->Succeeded())
        {
            ICondition::ConditionOperator conditionOperator;
            static_cast<void>(Def_HrFailed0(qualifier.GetOperator(&conditionOperator), status));
            if (status->Succeeded() && conditionOperator != static_cast<ICondition::ConditionOperator>(1))
            {
                static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
                if (status->Succeeded())
                {
                    static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                    if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
                    {
                        status->DiagnosticLogA("Qualifier: [%d] [%S] [%S]", qualifierIndex, attributeName.GetRef(), value.GetRef());
                    }
                }
            }
        }
    }

    for (int qualifierSetIndex = 0; qualifierSetIndex < decisionInfo->GetNumQualifierSets(); ++qualifierSetIndex)
    {
        DefStatusEx tagStatus;
        std::wstring qualifierSetTag;
        GetQualifierTagFromQualifierSetIndex(decisionInfo, atomPoolGroup, qualifierSetIndex, &tagStatus, qualifierSetTag);
        status->DiagnosticLogA("QualifierSet: [%d] tag: [%S]", qualifierSetIndex, qualifierSetTag.c_str());

        QualifierSetResult qualifierSet;
        static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifierSet(qualifierSetIndex, &qualifierSet), status));
        if (status->Succeeded())
        {
            const int numQualifiers = qualifierSet.GetNumQualifiers();
            for (int index = 0; index < numQualifiers; ++index)
            {
                QualifierResult qualifier;
                Atom attribute;
                StringResult attributeName;
                StringResult value;
                static_cast<void>(Def_HrFailed0(qualifierSet.GetQualifier(index, &qualifier, nullptr), status));
                if (status->Succeeded())
                {
                    ICondition::ConditionOperator conditionOperator;
                    static_cast<void>(Def_HrFailed0(qualifier.GetOperator(&conditionOperator), status));
                    if (status->Succeeded() && conditionOperator != static_cast<ICondition::ConditionOperator>(1))
                    {
                        static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
                        if (status->Succeeded())
                        {
                            static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                            if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
                            {
                                int globalQualifierIndex;
                                static_cast<void>(Def_HrFailed0(qualifier.GetQualifierIndex(&globalQualifierIndex), status));
                                status->DiagnosticLogA(
                                    "QualifierSet [%d] tag: [%S] qualifier: [%d] [%S] [%S]",
                                    qualifierSetIndex,
                                    qualifierSetTag.c_str(),
                                    globalQualifierIndex,
                                    attributeName.GetRef(),
                                    value.GetRef());
                            }
                        }
                    }
                }
            }
        }
    }

    status->DiagnosticLogA("<<<<<<< CUtilities::DisplayQualifierInformation");
    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->Succeeded();
}

bool CUtilities::GetLanguageOnlyQualifierSetIndexList(
    IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    std::list<int>* const qualifierSetIndices,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    int qualifierSetIndex = 0;
    while (qualifierSetIndex < decisionInfo->GetNumQualifierSets())
    {
        QualifierSetResult qualifierSet;
        static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifierSet(qualifierSetIndex, &qualifierSet), status));
        if (status->Succeeded() && qualifierSet.GetNumQualifiers() == 1)
        {
            QualifierResult setQualifier;
            static_cast<void>(Def_HrFailed0(qualifierSet.GetQualifier(0, &setQualifier, nullptr), status));
            if (status->Succeeded())
            {
                int qualifierIndex;
                static_cast<void>(Def_HrFailed0(setQualifier.GetQualifierIndex(&qualifierIndex), status));
                if (status->Succeeded() && qualifierIndex > 0)
                {
                    QualifierResult qualifier;
                    Atom attribute;
                    StringResult attributeName;
                    StringResult value;
                    static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
                    if (status->Succeeded())
                    {
                        ICondition::ConditionOperator conditionOperator;
                        static_cast<void>(Def_HrFailed0(qualifier.GetOperator(&conditionOperator), status));
                        if (status->Succeeded() && conditionOperator != static_cast<ICondition::ConditionOperator>(1))
                        {
                            static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
                            if (status->Succeeded())
                            {
                                static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                                if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
                                {
                                    DEFCOMPARISON comparison;
                                    static_cast<void>(DefStringResult_CompareWithOptions(
                                        attributeName.GetStringResult(), L"Language", DefCompare_CaseInsensitive, &comparison));
                                    if (comparison == Def_Equal)
                                    {
                                        status->DiagnosticLogA("language->QSI: [%S]->[%d]", value.GetRef(), qualifierSetIndex);
                                        qualifierSetIndices->push_back(qualifierSetIndex);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (status->Failed())
        {
            qualifierSetIndices->clear();
        }
        ++qualifierSetIndex;
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->Succeeded();
}

bool CUtilities::GetLanguageOnlyQualifierSetMap(
    IDecisionInfo* const decisionInfo,
    AtomPoolGroup* const atomPoolGroup,
    std::map<std::wstring, int>* const qualifierSetIndices,
    IDefStatusEx* const status)
{
    status->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    int qualifierSetIndex = 0;
    while (qualifierSetIndex < decisionInfo->GetNumQualifierSets())
    {
        QualifierSetResult qualifierSet;
        static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifierSet(qualifierSetIndex, &qualifierSet), status));
        if (status->Succeeded() && qualifierSet.GetNumQualifiers() == 1)
        {
            QualifierResult setQualifier;
            static_cast<void>(Def_HrFailed0(qualifierSet.GetQualifier(0, &setQualifier, nullptr), status));
            if (status->Succeeded())
            {
                int qualifierIndex;
                static_cast<void>(Def_HrFailed0(setQualifier.GetQualifierIndex(&qualifierIndex), status));
                if (status->Succeeded() && qualifierIndex > 0)
                {
                    QualifierResult qualifier;
                    Atom attribute;
                    StringResult attributeName;
                    StringResult value;
                    static_cast<void>(Def_HrFailed0(decisionInfo->GetQualifier(qualifierIndex, &qualifier), status));
                    if (status->Succeeded())
                    {
                        ICondition::ConditionOperator conditionOperator;
                        static_cast<void>(Def_HrFailed0(qualifier.GetOperator(&conditionOperator), status));
                        if (status->Succeeded() && conditionOperator != static_cast<ICondition::ConditionOperator>(1))
                        {
                            static_cast<void>(Def_HrFailed0(qualifier.GetOperand1Attribute(&attribute), status));
                            if (status->Succeeded())
                            {
                                static_cast<void>(Def_HrFailed0(qualifier.GetOperand2Literal(&value), status));
                                if (status->Succeeded() && atomPoolGroup->TryGetString(attribute, &attributeName))
                                {
                                    DEFCOMPARISON comparison;
                                    static_cast<void>(DefStringResult_CompareWithOptions(
                                        attributeName.GetStringResult(), L"Language", DefCompare_CaseInsensitive, &comparison));
                                    if (comparison == Def_Equal)
                                    {
                                        std::wstring language(value.GetRef());
                                        status->DiagnosticLogA("language->QSI: [%S]->[%d]", language.c_str(), qualifierSetIndex);
                                        qualifierSetIndices->insert(std::make_pair(language, qualifierSetIndex));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (status->Failed())
        {
            qualifierSetIndices->clear();
        }
        ++qualifierSetIndex;
    }

    status->DiagnosticLogWithErrorCodeA(__FUNCTION__, status->GetHResult());
    return status->Succeeded();
}

std::wstring CUtilities::NormalizeLanguageTag(const std::wstring& languageTag)
{
    wchar_t normalizedLanguageTag[90];
    const wchar_t* const source = languageTag.c_str();
    const HRESULT result = FormatLanguageTag(source, 127, source, normalizedLanguageTag);
    return std::wstring(FAILED(result) ? L"" : normalizedLanguageTag);
}

void CUtilities::NormalizeAllLanguageTags(std::vector<std::wstring>& languageTags)
{
    for (std::wstring& languageTag : languageTags)
    {
        languageTag.assign(NormalizeLanguageTag(languageTag));
    }

    auto destination = languageTags.begin();
    const auto end = languageTags.end();
    while (destination != end)
    {
        auto source = destination + 1;
        if (destination->empty())
        {
            while (source != end)
            {
                if (!source->empty())
                {
                    destination->assign(*source);
                    ++destination;
                }
                ++source;
            }
            languageTags.erase(destination, languageTags.end());
            return;
        }
        ++destination;
    }
    languageTags.erase(destination, languageTags.end());
}

} // namespace Microsoft::Resources::Indexers
