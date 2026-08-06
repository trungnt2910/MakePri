#include "StdAfx.h"

#include <ParameterManager.h>

namespace Microsoft::Resources::Tools::MakePri
{
namespace
{

constexpr wchar_t PackageIdentityNameXPath[] = L"*[local-name()='Package']/*[local-name()='Identity']/@Name";

const wchar_t* const InputIndexExtensions[] {L"pri", L"xml"};

} // namespace

ParameterManager::ParameterManager(InputArgs inputArgs) : InputParams(inputArgs) { static_cast<void>(m_mappingFileFormat); }

const wchar_t* ParameterManager::GetIndexFile() const
{
    const wchar_t* const path = m_inputIndexFile.GetFullPath();
    return path[0] == L'\0' ? nullptr : path;
}

const wchar_t* ParameterManager::GetOutputFolder() const
{
    const wchar_t* const path = m_outputPath.GetFolderPath();
    return path[0] == L'\0' ? nullptr : path;
}

HRESULT ParameterManager::VerifyParams()
{
    if (m_inputArgs.scenario == UsageScenario::None)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
        return E_INVALIDARG;
    }

    PathHandler::PathFlags overwriteFlags = PathHandler::PathFlags::None;
    if (m_inputArgs.overwrite)
    {
        overwriteFlags = PathHandler::PathFlags::Overwrite;
    }

    if (m_inputArgs.scenario == UsageScenario::New || m_inputArgs.scenario == UsageScenario::Versioned ||
        m_inputArgs.scenario == UsageScenario::ResourcePack)
    {
        HRESULT result = m_projectRootPath.InitializeForInputFolder(m_inputArgs.projectRoot);
        if (FAILED(result))
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_PLEASE_PROVIDE_VALID_PROJECT_ROOT, result);
            return result;
        }

        result = m_configXmlPath.InitializeForInputFile(m_inputArgs.configXml, L"xml");
        if (FAILED(result))
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_CONFIGURATION_FILE_PATH_DOES_NOT_EXIST, result);
            return result;
        }

        result =
            m_outputPath.InitializeForOutputPath(m_inputArgs.outputFile, L"pri", L"resources.pri", static_cast<PathHandler::PathFlags>(3));
        if (FAILED(result))
        {
            return result;
        }

        if (m_inputArgs.indexLog != nullptr)
        {
            result = m_indexLogPath.InitializeForOutputPath(m_inputArgs.indexLog, L"xml", nullptr, overwriteFlags);
            if (FAILED(result))
            {
                return result;
            }
        }

        if (m_inputArgs.schemaFile != nullptr)
        {
            result = m_schemaFilePath.InitializeForOutputPath(m_inputArgs.schemaFile, L"xml", nullptr, overwriteFlags);
            if (FAILED(result))
            {
                return result;
            }
        }
    }

    if (m_inputArgs.outputOptions != nullptr)
    {
        if (m_inputArgs.scenario != UsageScenario::Dump && m_inputArgs.scenario != UsageScenario::CreateConfig &&
            m_inputArgs.indexLog == nullptr && m_inputArgs.schemaFile == nullptr)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_OUTPUTOPTIONS_OO_SPECIFIED_WHEN_NO_OUTPUT, E_INVALIDARG);
            return E_INVALIDARG;
        }

        DefStatusEx status;
        if (!m_outputOptions.Initialize(m_inputArgs.outputOptions, &status))
        {
            m_inputArgs.outputOptionsError = true;
            const HRESULT result = status.GetHResult();
            if (FAILED(result))
            {
                return result;
            }
        }
    }

    if (m_inputArgs.indexOptions != nullptr)
    {
        if (m_inputArgs.scenario != UsageScenario::New && m_inputArgs.scenario != UsageScenario::Versioned &&
            m_inputArgs.scenario != UsageScenario::ResourcePack)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_INDEXOPTIONS_IO_SPECIFIED_WHEN_NO_RESOURCES, E_INVALIDARG);
            return E_INVALIDARG;
        }

        DefStatusEx status;
        if (!m_indexOptions.Initialize(m_inputArgs.indexOptions, &status))
        {
            const IDefStatus* const rawStatus = &status;
            if (rawStatus != nullptr && rawStatus->GetWhat() == E_DEF_INDEXER_SCHEMA_NOT_FOUND &&
                m_indexOptions.GetIndexerSchemaCollection(&status) != nullptr)
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_UNABLE_TO_LOAD_SCHEMA_FROM, S_OK, m_indexOptions.GetIndexerSchemaCollection(&status));
            }
            else if (
                rawStatus != nullptr && rawStatus->GetWhat() == E_DEF_UNSUPPORTED_FILE_TYPE &&
                m_indexOptions.GetIndexerSchemaCollection(&status) != nullptr)
            {
                Log::StoreErrorMessage(
                    MAKEPRI_STRING_UNSUPPORTED_FILE_TYPE_FOR_SCHEMA_FILE, S_OK, m_indexOptions.GetIndexerSchemaCollection(&status));
            }
            else
            {
                m_inputArgs.indexOptionsError = true;
            }

            const HRESULT result = status.GetHResult();
            if (FAILED(result))
            {
                return result;
            }
        }

        if (m_indexOptions.GetShouldBuildDeploymentMergeablePri())
        {
            goto ScenarioValidation;
        }
    }

    if (m_inputArgs.contentChecksumValue != 0)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_SPECIFYING_MICROSOFT_INTERNAL_METADATA_CONTENT_CHECKSUM, E_INVALIDARG);
        return E_INVALIDARG;
    }

ScenarioValidation:
    if (m_inputArgs.scenario == UsageScenario::New)
    {
        if (m_inputArgs.indexName != nullptr)
        {
            m_inputArgs.manifest = nullptr;
        }
        else
        {
            const HRESULT result = GetIndexNameFromAppxManifest();
            if (FAILED(result))
            {
                if (result == CLASS_E_CLASSNOTAVAILABLE)
                {
                    return result;
                }
                if (result == E_INVALIDARG)
                {
                    m_indexName.assign(L"Application");
                    return S_OK;
                }
                Log::StoreErrorMessage(MAKEPRI_STRING_APPX_MANIFEST_NOT_FOUND_OR_IS, result);
                Log::StoreLastErrorInfo();
                return result;
            }
        }
        goto OptionalInputFiles;
    }

    if (m_inputArgs.scenario == UsageScenario::Versioned)
    {
        if (m_inputArgs.indexName != nullptr)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_UNEXPECTED_PARAMETER_FOUND, E_INVALIDARG);
            return E_INVALIDARG;
        }
    }

    if (m_inputArgs.scenario == UsageScenario::Versioned || m_inputArgs.scenario == UsageScenario::ResourcePack)
    {
        if (m_inputArgs.versionMajor != 0 || m_inputArgs.manifest != nullptr ||
            m_inputArgs.platformVersion != MrmPlatformVersionInternal::DefaultPlatformVersion)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_UNEXPECTED_PARAMETER_FOUND, E_INVALIDARG);
            return E_INVALIDARG;
        }

        const HRESULT result = InitializeInputIndexFile();
        if (FAILED(result))
        {
            return result;
        }
        goto OptionalInputFiles;
    }

    if (m_inputArgs.scenario == UsageScenario::Dump)
    {
        if (m_inputArgs.projectRoot != nullptr || m_inputArgs.versionMajor != 0 || m_inputArgs.configXml != nullptr ||
            m_inputArgs.indexName != nullptr || m_inputArgs.manifest != nullptr || m_inputArgs.schemaFile != nullptr ||
            m_inputArgs.platformVersion != MrmPlatformVersionInternal::DefaultPlatformVersion)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_UNEXPECTED_PARAMETER_FOUND, E_INVALIDARG);
            return E_INVALIDARG;
        }

        HRESULT result = InitializeInputIndexFile();
        if (FAILED(result))
        {
            return result;
        }
        result = m_outputPath.InitializeForOutputPath(m_inputArgs.outputFile, L"xml", L"resources.pri.xml", overwriteFlags);
        if (FAILED(result))
        {
            return result;
        }
        goto OptionalInputFiles;
    }

    if (m_inputArgs.scenario == UsageScenario::CreateConfig)
    {
        if (m_inputArgs.configXml == nullptr)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_PLEASE_PROVIDE_CONFIGURATION_FILE_DESTINATION_USING, E_INVALIDARG);
            return E_INVALIDARG;
        }
        if (m_inputArgs.defaultQualifiers == nullptr)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_REQUIRED_QUALIFIER_NOT_SPECIFIED_FOR_LANGUAGE, E_INVALIDARG);
            return E_INVALIDARG;
        }

        if (m_inputArgs.platformVersion == MrmPlatformVersionInternal::DefaultPlatformVersion)
        {
            Log::Warning(MAKEPRI_STRING_PLATFORM_PV_NOT_SPECIFIED_DEFAULT_IS, S_OK);
        }
        else if (m_inputArgs.extensionDll != nullptr)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_CANNOT_SPECIFY_BOTH_PLATFORM_PV_AND, E_INVALIDARG);
            return E_INVALIDARG;
        }

        const HRESULT result = m_configXmlPath.InitializeForOutputPath(m_inputArgs.configXml, L"xml", nullptr, overwriteFlags);
        if (FAILED(result))
        {
            return result;
        }
    }

OptionalInputFiles:
    if (m_inputArgs.extensionDll != nullptr)
    {
        const HRESULT result = m_extensionDllPath.InitializeForInputFile(m_inputArgs.extensionDll, L"dll");
        if (FAILED(result))
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, result, m_inputArgs.extensionDll);
            return result;
        }
    }

    if (m_inputArgs.externalSchema != nullptr)
    {
        const HRESULT result = m_externalSchemaPath.InitializeForInputFile(m_inputArgs.externalSchema, L"pri");
        if (FAILED(result))
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, result, m_inputArgs.externalSchema);
            return result;
        }
    }

    return S_OK;
}

HRESULT ParameterManager::InitializeInputIndexFile()
{
    const wchar_t* path = m_inputArgs.indexFile;
    std::wstring combinedPath;
    if (path == nullptr)
    {
        const HRESULT result = PathHandler::CombinePath(m_inputArgs.projectRoot, L"resources.pri", combinedPath);
        if (FAILED(result))
        {
            return result;
        }
        path = combinedPath.c_str();
    }

    const HRESULT result =
        m_inputIndexFile.InitializeForInputFile(path, static_cast<std::uint32_t>(std::size(InputIndexExtensions)), InputIndexExtensions);
    if (FAILED(result))
    {
        return result;
    }
    return S_OK;
}

HRESULT ParameterManager::GetIndexNameFromAppxManifest()
{
    const wchar_t* path = m_inputArgs.manifest;
    std::wstring combinedPath;
    if (path == nullptr)
    {
        const HRESULT result = PathHandler::CombinePath(m_inputArgs.projectRoot, L"AppxManifest.xml", combinedPath);
        if (FAILED(result))
        {
            return result;
        }
        path = combinedPath.c_str();
    }

    HRESULT result = m_manifestPath.InitializeForInputFile(path, nullptr);
    if (FAILED(result))
    {
        return result;
    }

    IXMLDOMDocument2* document = nullptr;
    result = CXMLUtil::CreateXMLDocument(&document);
    if (FAILED(result))
    {
        return result;
    }

    result = CXMLUtil::LoadXMLDataFromFile(document, m_manifestPath.GetFullPath());
    if (FAILED(result))
    {
        document->Release();
        return result;
    }

    wchar_t* value = nullptr;
    result = CXMLUtil::GetSingleNodeValue(document, PackageIdentityNameXPath, &value);
    if (SUCCEEDED(result))
    {
        m_indexName.assign(value);
    }

    delete[] value;
    document->Release();
    return result;
}

namespace
{

bool HasFlag(const PathHandler::PathFlags value, const PathHandler::PathFlags flag)
{
    return (static_cast<std::uint8_t>(value) & static_cast<std::uint8_t>(flag)) != 0;
}

} // namespace

PathHandler::~PathHandler() = default;

HRESULT PathHandler::InitializeForInputFolder(const wchar_t* const path)
{
    if (path == nullptr)
    {
        return E_INVALIDARG;
    }

    std::wstring adjustedPath(path);
    if (!adjustedPath.empty())
    {
        _AdjustSlashes(adjustedPath);
    }
    else
    {
        adjustedPath.assign(L".");
    }

    HRESULT hr = _VerifyPathString(adjustedPath);
    if (FAILED(hr))
    {
        return hr;
    }

    FileState state {};
    hr = _GetFileState(adjustedPath.c_str(), state);
    if (FAILED(hr))
    {
        return hr;
    }

    switch (state)
    {
    case FileState::DoesNotExist:
        Log::StoreErrorMessage(MAKEPRI_STRING_TARGET_DOES_NOT_EXIST, E_INVALIDARG, path);
        return E_INVALIDARG;
    case FileState::Directory:
        m_folderPath.assign(adjustedPath);
        break;
    case FileState::File:
        Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, E_INVALIDARG, path);
        return E_INVALIDARG;
    }

    m_initialized = true;
    return S_OK;
}

HRESULT PathHandler::InitializeForInputFile(const wchar_t* const path, const wchar_t* const extension)
{
    if (extension != nullptr)
    {
        return InitializeForInputFile(path, 1, &extension);
    }
    return InitializeForInputFile(path, 0, nullptr);
}

HRESULT PathHandler::InitializeForInputFile(
    const wchar_t* const path,
    const std::uint32_t extensionCount,
    const wchar_t* const* const extensions)
{
    if (path == nullptr)
    {
        return E_INVALIDARG;
    }

    std::wstring adjustedPath(path);
    _AdjustSlashes(adjustedPath);

    HRESULT hr = _VerifyPathString(adjustedPath);
    if (FAILED(hr))
    {
        return hr;
    }

    FileState state {};
    hr = _GetFileState(adjustedPath.c_str(), state);
    if (FAILED(hr))
    {
        return hr;
    }
    if (state == FileState::DoesNotExist)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_TARGET_DOES_NOT_EXIST, E_INVALIDARG, path);
        return E_INVALIDARG;
    }
    if (state == FileState::Directory)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, E_INVALIDARG, path);
        return E_INVALIDARG;
    }

    if (extensionCount != 0 && extensions != nullptr)
    {
        std::wstring extension;
        _GetFileExtension(adjustedPath, extension);
        if (!extension.empty())
        {
            bool found {};
            for (std::uint32_t index = 0; index < extensionCount && !found; ++index)
            {
                found = DefString_CompareWithOptions(extension.c_str(), extensions[index], DefCompare_CaseInsensitive) == Def_Equal;
            }
            if (!found)
            {
                DEFSTRINGRESULT expectedExtensions {};
                DefStringResult_InitBuf(&expectedExtensions, nullptr);
                hr = DefStringResult_SetCopy(&expectedExtensions, extensions[0]);
                if (SUCCEEDED(hr))
                {
                    for (std::uint32_t index = 1; index < extensionCount; ++index)
                    {
                        hr = DefStringResult_ConcatPathElement(&expectedExtensions, extensions[index], L'\\');
                        if (FAILED(hr))
                        {
                            break;
                        }
                    }
                }

                const wchar_t* expected = extensions[0];
                if (SUCCEEDED(hr))
                {
                    DefStringResult_GetRef(&expectedExtensions, &expected);
                }
                Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_FILE_EXTENSION_FOUND_IN_ONLY, E_INVALIDARG, path, expected);
                DefStringResult_Clear(&expectedExtensions, true);
                return E_INVALIDARG;
            }
        }
    }

    m_fullPath.assign(adjustedPath);
    m_initialized = true;
    return S_OK;
}

HRESULT PathHandler::InitializeForOutputPath(
    const wchar_t* const path,
    const wchar_t* const extension,
    const wchar_t* const defaultFileName,
    const PathFlags flags)
{
    if (path == nullptr && defaultFileName == nullptr)
    {
        return E_INVALIDARG;
    }

    std::wstring adjustedPath(path != nullptr ? path : defaultFileName);
    _AdjustSlashes(adjustedPath);

    HRESULT hr = _VerifyPathString(adjustedPath);
    if (SUCCEEDED(hr))
    {
        hr = _VerifyParentFolderExists(adjustedPath);
    }
    if (FAILED(hr))
    {
        return hr;
    }

    FileState state {};
    hr = _GetFileState(adjustedPath.c_str(), state);
    if (FAILED(hr))
    {
        return hr;
    }

    if (state == FileState::Directory)
    {
        if (!HasFlag(flags, PathFlags::AllowDirectory))
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, E_INVALIDARG, path);
            return E_INVALIDARG;
        }
        if (defaultFileName == nullptr)
        {
            m_folderPath.assign(adjustedPath);
            m_initialized = true;
            return S_OK;
        }
        hr = CombinePath(adjustedPath.c_str(), defaultFileName, adjustedPath);
        if (FAILED(hr))
        {
            return hr;
        }
        state = FileState::File;
    }

    if (state == FileState::File || state == FileState::DoesNotExist)
    {
        if (extension != nullptr)
        {
            std::wstring existingExtension;
            _GetFileExtension(adjustedPath, existingExtension);
            if (existingExtension.empty() ||
                DefString_CompareWithOptions(existingExtension.c_str(), extension, DefCompare_CaseInsensitive) != Def_Equal)
            {
                adjustedPath.append(L".");
                adjustedPath.append(extension);
            }
        }

        if (PathFileExistsW(adjustedPath.c_str()))
        {
            hr = _HandleOverwrite(flags, adjustedPath.c_str());
            if (FAILED(hr))
            {
                return hr;
            }
        }
        _SeparateParentFolderAndFile(adjustedPath);
    }

    m_fullPath.assign(adjustedPath);
    m_initialized = true;
    return S_OK;
}

const wchar_t* PathHandler::GetFileNameNoExt()
{
    if (m_fileNameNoExt.empty())
    {
        std::wstring extension;
        _GetFileExtension(m_fileName, extension);
        if (!extension.empty())
        {
            m_fileNameNoExt.assign(m_fileName.substr(0, m_fileName.size() - extension.size() - 1));
        }
        else
        {
            m_fileNameNoExt.assign(m_fileName);
        }
    }
    return m_fileNameNoExt.c_str();
}

HRESULT PathHandler::_VerifyParentFolderExists(std::wstring& path)
{
    const std::wstring::size_type separator = path.rfind(L'\\');
    if (separator == std::wstring::npos)
    {
        return S_OK;
    }

    const std::wstring parent = path.substr(0, separator);
    FileState state {};
    const HRESULT hr = _GetFileState(parent.c_str(), state);
    if (FAILED(hr))
    {
        return hr;
    }
    if (state != FileState::DoesNotExist)
    {
        return S_OK;
    }

    Log::StoreErrorMessage(MAKEPRI_STRING_TARGET_DOES_NOT_EXIST, E_INVALIDARG, path.c_str());
    return E_INVALIDARG;
}

void PathHandler::_GetFileExtension(std::wstring& path, std::wstring& extension)
{
    extension.clear();
    const std::wstring::size_type dot = path.rfind(L'.');
    if (dot != std::wstring::npos)
    {
        extension.assign(path.substr(dot + 1));
    }
}

HRESULT PathHandler::_GetFileState(const wchar_t* const path, FileState& state)
{
    state = FileState::DoesNotExist;
    WIN32_FIND_DATAW findData {};
    const HANDLE find = FindFirstFileW(path, &findData);
    if (find == INVALID_HANDLE_VALUE)
    {
        if (m_hadTrailingSlash)
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_TARGET_DOES_NOT_EXIST, E_INVALIDARG, path);
            return E_INVALIDARG;
        }
        return S_OK;
    }

    state = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? FileState::Directory : FileState::File;
    FindClose(find);
    return S_OK;
}

void PathHandler::_SeparateParentFolderAndFile(std::wstring& path)
{
    const std::wstring::size_type separator = path.rfind(L'\\');
    if (separator == std::wstring::npos)
    {
        m_folderPath.assign(L".");
        m_fileName.assign(path);
        return;
    }

    m_folderPath.assign(path.substr(0, separator));
    m_fileName.assign(path.substr(separator + 1));
}

HRESULT PathHandler::_HandleOverwrite(const PathFlags flags, const wchar_t* const path)
{
    if (HasFlag(flags, PathFlags::Overwrite))
    {
        return S_OK;
    }

    Log::WriteWorkOutput(MAKEPRI_STRING_OUTPUT_FILE_ALREADY_EXISTS_OVERWRITE_Y, path);
    Log::Flush();
    const wint_t input = _getwche();
    std::fwprintf(stdout, L"\n");

    wchar_t yes[MAX_PATH] {};
    LoadStringW(nullptr, MAKEPRI_STRING_YES, yes, MAX_PATH);
    return std::towupper(input) == yes[0] ? S_OK : E_ABORT;
}

HRESULT PathHandler::_VerifyPathString(std::wstring& path)
{
    if (path.find_first_of(L"*\"<>|\n\t\r") != std::wstring::npos)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_INVALID_PATH, E_INVALIDARG, path.c_str());
        return E_INVALIDARG;
    }

    const std::wstring::size_type colon = path.find_first_of(L':');
    if (colon == std::wstring::npos || colon >= path.size() - 1)
    {
        return S_OK;
    }

    const std::wstring::size_type afterColon = colon + 1;
    if (path[afterColon] == L'\\' && path.find_first_of(L':', afterColon) == std::wstring::npos)
    {
        return S_OK;
    }

    Log::Error(MAKEPRI_STRING_INVALID_PATH, E_INVALIDARG, path.c_str());
    return E_INVALIDARG;
}

void PathHandler::_AdjustSlashes(std::wstring& path)
{
    std::replace(path.begin(), path.end(), L'/', L'\\');
    if (path != L".\\" && path != L"\\" && !path.empty() && path != L"." && path.back() == L'\\')
    {
        m_hadTrailingSlash = true;
        std::replace(path.end() - 1, path.end(), L'\\', L'\0');
    }
}

HRESULT PathHandler::CombinePath(const wchar_t* const parent, const wchar_t* const child, std::wstring& result)
{
    if (parent == nullptr)
    {
        result.assign(child);
        return S_OK;
    }

    DEFSTRINGRESULT combined {};
    HRESULT hr = DefStringResult_InitBuf(&combined, nullptr);
    if (SUCCEEDED(hr))
    {
        hr = DefStringResult_InitRef(&combined, parent);
    }
    if (SUCCEEDED(hr))
    {
        hr = DefStringResult_ConcatPathElement(&combined, child, L'\\');
    }
    if (SUCCEEDED(hr))
    {
        const wchar_t* reference {};
        hr = DefStringResult_GetRef(&combined, &reference);
        if (SUCCEEDED(hr))
        {
            result.assign(reference);
        }
    }
    DefStringResult_Clear(&combined, true);
    return hr;
}

} // namespace Microsoft::Resources::Tools::MakePri
