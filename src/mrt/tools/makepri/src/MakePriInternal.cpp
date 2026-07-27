#include "StdAfx.h"

#include <ParameterManager.h>

using Microsoft::Resources::DefStatusEx;
using Microsoft::Resources::MrmProfile;
using Microsoft::Resources::PriConfigurationXml;
using Microsoft::Resources::StandalonePriFile;
using Microsoft::Resources::StandalonePriFileXml;
using Microsoft::Resources::UnifiedResourceView;
using Microsoft::Resources::Indexers::CContentChecksumData;
using Microsoft::Resources::Indexers::CHIndexerBase;
using Microsoft::Resources::Indexers::CXmlHelper;
using Microsoft::Resources::Tools::MakePri::FileOperations;
using Microsoft::Resources::Tools::MakePri::InputArgs;
using Microsoft::Resources::Tools::MakePri::Log;
using Microsoft::Resources::Tools::MakePri::ParameterManager;
using Microsoft::Resources::Tools::MakePri::UsageScenario;

const wchar_t* g_temporaryFolder;

namespace Microsoft::Resources::Tools::MakePri
{
InputParams::~InputParams() = default;
} // namespace Microsoft::Resources::Tools::MakePri

BOOL WINAPI MakepriCtrlHandler(const DWORD controlType)
{
    static_cast<void>(controlType);
    DefStatusEx status;
    FileOperations::s_DeleteFolderAndContents(g_temporaryFolder);
    Log::WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_ABORTED);
    return FALSE;
}

HRESULT MakePriCreateConfigInternal(InputArgs* const inputArgs)
{
    DefStatusEx status;
    auto statusCleanup = wil::scope_exit([&] {
        Log::DisplayStatusErrorAndWarnings(&status);
        Log::Flush();
    });

    ParameterManager parameters(*inputArgs);
    HRESULT result = parameters.VerifyParams();
    if (SUCCEEDED(result))
    {
        status.SetDiagnosticLoggingEnabled(parameters.GetInputArgs().diagnosticLogging);
        const wchar_t* const outputFile = parameters.GetConfigXmlFile();
        if (!PriConfigurationXml::GeneratePriConfigToFile(
                parameters.GetInputArgs().platformVersion, outputFile, parameters.GetInputArgs().defaultQualifiers, &status))
        {
            if (status.Succeeded())
            {
                result = E_FAIL;
            }
            else
            {
                result = status.GetHResult();
            }
        }
        else
        {
            Log::WriteWorkOutput(MAKEPRI_STRING_WRITING_PRI_CONFIG_FILE_TO, outputFile);
            Log::WriteWorkOutput(MAKEPRI_STRING_SUCCESSFULLY_COMPLETED);
        }
    }
    else if (result != E_ABORT)
    {
        Log::PrintUsage(inputArgs);
    }
    return result;
}

HRESULT MakePriDumpInternal(InputArgs* const inputArgs, const wchar_t** const outputFileResult)
{
    (void)outputFileResult;

    DefStatusEx status;
    auto statusCleanup = wil::scope_exit([&] {
        Log::DisplayStatusErrorAndWarnings(&status);
        Log::Flush();
    });

    ParameterManager parameters(*inputArgs);
    HRESULT result = parameters.VerifyParams();
    if (FAILED(result))
    {
        if (result != E_ABORT)
        {
            Log::PrintUsage(inputArgs);
        }
        return result;
    }

    status.SetDiagnosticLoggingEnabled(parameters.GetInputArgs().diagnosticLogging);
    const wchar_t* const outputFile = parameters.GetOutputFile();
    const auto dumpType = parameters.GetInputArgs().dumpType;
    const auto& outputOptions = parameters.GetOutputOptions();
    status.Reset();
    status.SetErrorLocation(parameters.GetIndexFile());

    result = S_OK;
    auto resultCleanup = wil::scope_exit([&] {
        if (FAILED(result))
        {
            Log::DisplayErrorMessage(L"MakePri", result);
        }
    });

    Microsoft::Resources::AutoDeletePtr<MrmProfile> profile;
    result = MrmProfile::ChooseDefaultProfile(
        MrmProfile::ProfileType::EmptyInit,
        Microsoft::Resources::MrmPlatformVersionInternal::DefaultPlatformVersion,
        nullptr,
        nullptr,
        nullptr,
        &profile);
    RETURN_IF_FAILED(result);

    Microsoft::Resources::AutoDeletePtr<UnifiedResourceView> resourceView;
    if (profile)
    {
        result = UnifiedResourceView::CreateInstance(profile, &resourceView);
        RETURN_IF_FAILED(result);
    }

    Microsoft::Resources::AutoDeletePtr<StandalonePriFile> schemaFile;
    Microsoft::Resources::AutoDeletePtr<StandalonePriFile> priFileData;
    if (parameters.GetExternalSchemaFile() != nullptr)
    {
        result = StandalonePriFile::CreateInstance(0, parameters.GetExternalSchemaFile(), profile, &schemaFile);
        RETURN_IF_FAILED(result);

        if (schemaFile)
        {
            result = StandalonePriFile::CreateInstance(
                0,
                parameters.GetIndexFile(),
                profile,
                static_cast<const Microsoft::Resources::ISchemaCollection*>(schemaFile),
                &priFileData);
            RETURN_IF_FAILED(result);
        }
    }
    else
    {
        result = StandalonePriFile::CreateInstance(0, parameters.GetIndexFile(), profile, &priFileData);
        RETURN_IF_FAILED(result);
    }

    const Microsoft::Resources::PriFile* priFile = priFileData ? static_cast<Microsoft::Resources::PriFile*>(priFileData) : nullptr;
    if ((priFileData != nullptr) && (priFileData->GetPriDescriptor()->GetNumReferencedFileSections() > 0))
    {
        if (resourceView)
        {
            result = resourceView->SetApplicationPriFile(parameters.GetIndexFile(), nullptr, &priFile);
            RETURN_IF_FAILED(result);
        }
        else
        {
            priFile = nullptr;
        }
    }

    if (priFile != nullptr)
    {
        if (!StandalonePriFileXml::DumpPriFileToXmlFile(
                outputFile, const_cast<Microsoft::Resources::PriFile*>(priFile), dumpType, outputOptions, &status))
        {
            result = status.Succeeded() ? E_FAIL : status.GetHResult();
        }
    }

    if (SUCCEEDED(result) && status.Failed())
    {
        result = status.GetHResult();
    }

    if (FAILED(result))
    {
        Log::Error(MAKEPRI_STRING_FORMAT_STRING, result, L"Dump");
    }
    else
    {
        Log::WriteWorkOutput(MAKEPRI_STRING_OUTPUT_FILE_AT, outputFile);
        Log::WriteWorkOutput(MAKEPRI_STRING_SUCCESSFULLY_COMPLETED);
    }

    return result;
}

HRESULT MakePriNewVersionedPack(InputArgs* const inputArgs, const UsageScenario scenario)
{

    DefStatusEx status;
    auto statusCleanup = wil::scope_exit([&] {
        Log::DisplayStatusErrorAndWarnings(&status);
        Log::Flush();
    });

    ParameterManager parameters(*inputArgs);
    HRESULT result = parameters.VerifyParams();
    if (FAILED(result))
    {
        if (result != E_ABORT)
        {
            Log::PrintUsage(inputArgs);
        }
        return result;
    }

    status.SetDiagnosticLoggingEnabled(parameters.m_inputArgs.diagnosticLogging);

    Microsoft::Resources::StringResult temporaryFolder;
    result = FileOperations::s_CreateUniqueTempFolder(&status, &temporaryFolder);
    if (FAILED(result))
    {
        return result;
    }

    const wchar_t* const temporaryFolderPath = temporaryFolder.GetRef();
    if (temporaryFolderPath == nullptr || temporaryFolderPath[0] == L'\0')
    {
        result = E_FAIL;
        status.DiagnosticLogWithErrorCodeA("Makepri Failed! Temporary file path is empty.", E_FAIL);
        return result;
    }

    FileOperations fileOperations(temporaryFolderPath, parameters.GetOutputFolder());
    g_temporaryFolder = temporaryFolder.GetRef();
    status.DiagnosticLogA("Temporary output location is [%S]", temporaryFolder.GetRef());

    try
    {
        status.DiagnosticLogA("Start - Makepri new/resourcepack/versioned");

        const wchar_t* projectRoot = parameters.m_projectRootPath.GetFolderPath();
        if (projectRoot[0] == L'\0')
        {
            projectRoot = nullptr;
        }

        const wchar_t* configXml = parameters.m_configXmlPath.GetFullPath();
        if (configXml[0] == L'\0')
        {
            configXml = nullptr;
        }

        const wchar_t* configurationFile = parameters.m_indexLogPath.GetFullPath();
        if (configurationFile[0] == L'\0')
        {
            configurationFile = nullptr;
        }

        CXmlHelper* const xmlValue = new (std::nothrow) CXmlHelper;
        Microsoft::Resources::AutoDeletePtr<CXmlHelper> xml(xmlValue);

        Microsoft::Resources::IndexOptions& indexOptions = parameters.m_indexOptions;
        if (parameters.m_inputArgs.autoMerge)
        {
            indexOptions.m_flags |= 0x4;
        }
        if (parameters.m_inputArgs.reverseMap)
        {
            indexOptions.m_flags |= 0x80;
        }

        if (DefString_IsEmpty(indexOptions.GetIndexerSchemaPath()))
        {
            const wchar_t* indexerSchemaPath = parameters.m_externalSchemaPath.GetFullPath();
            if (indexerSchemaPath[0] == L'\0')
            {
                indexerSchemaPath = nullptr;
            }
            if (scenario != UsageScenario::New && DefString_IsEmpty(indexerSchemaPath))
            {
                indexerSchemaPath = parameters.GetIndexFile();
            }
            if (!DefString_IsEmpty(indexerSchemaPath) && !indexOptions.SetIndexerSchemaPath(indexerSchemaPath, &status))
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_UNABLE_TO_LOAD_SCHEMA_FROM, S_OK, indexerSchemaPath);
                result = status.GetHResult();
            }
        }

        Log::Flush();

        CHIndexerBase* indexer = nullptr;
        do
        {
            if (FAILED(result))
            {
                break;
            }

            if (configXml == nullptr || xmlValue == nullptr)
            {
                Log::Error(MAKEPRI_STRING_INVALID_XML, result, configXml);
                Log::Error(MAKEPRI_STRING_FORMAT_STRING, result, L"Schema of ConfigXML");
                break;
            }

            result = xmlValue->Init(configXml, CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_FILE_PATH, L"resources", &status);
            if (FAILED(result))
            {
                Log::Error(MAKEPRI_STRING_INVALID_XML, result, configXml);
                Log::Error(MAKEPRI_STRING_FORMAT_STRING, result, L"Invalid ConfigXML");
                break;
            }

            IXMLDOMNode* configuration = nullptr;
            result = xmlValue->GetCurrentNode(&configuration);
            if (FAILED(result) || configuration == nullptr)
            {
                if (SUCCEEDED(result))
                {
                    result = E_INVALIDARG;
                }
                Log::Error(MAKEPRI_STRING_INVALID_XML, E_INVALIDARG, configXml);
                Log::Error(MAKEPRI_STRING_FORMAT_STRING, E_INVALIDARG, L"Invalid ConfigXML");
                break;
            }

            if (scenario == UsageScenario::New)
            {
                ULONG majorVersion = static_cast<ULONG>(parameters.m_inputArgs.versionMajor);
                if (majorVersion == 0)
                {
                    majorVersion = 1;
                }

                const wchar_t* simpleId = parameters.m_inputArgs.indexName;
                if (simpleId == nullptr && !parameters.m_indexName.empty())
                {
                    simpleId = parameters.m_indexName.c_str();
                }

                result = CHIndexerBase::NewForNew(
                    configuration,
                    projectRoot,
                    temporaryFolder.GetRef(),
                    &indexOptions,
                    configurationFile,
                    simpleId,
                    majorVersion,
                    &status,
                    &indexer);
            }
            else if (scenario == UsageScenario::Versioned)
            {
                result = CHIndexerBase::NewForVersioned(
                    configuration,
                    projectRoot,
                    temporaryFolder.GetRef(),
                    &indexOptions,
                    configurationFile,
                    parameters.GetIndexFile(),
                    &status,
                    &indexer);
            }
            else if (scenario == UsageScenario::ResourcePack)
            {
                result = CHIndexerBase::NewForResourcePack(
                    configuration,
                    projectRoot,
                    temporaryFolder.GetRef(),
                    &indexOptions,
                    configurationFile,
                    nullptr,
                    parameters.GetIndexFile(),
                    &status,
                    &indexer);
            }

            if (FAILED(result))
            {
                Log::Error(MAKEPRI_STRING_FORMAT_STRING, result, L"Initializing Indexer");
                break;
            }

            bool outputUnderRoot = false;
            result = indexer->CheckIfOutputUnderRoot(parameters.GetOutputFolder(), &status, &outputUnderRoot);
            if (FAILED(result))
            {
                break;
            }
            if (outputUnderRoot)
            {
                Log::Warning(MAKEPRI_STRING_OUTPUT_LOCATION_OF_PRI_FILE_S, S_OK);
            }

            CContentChecksumData::NeutralLanguageCandidateCreation neutralLanguage;
            if (scenario == UsageScenario::New)
            {
                neutralLanguage = CContentChecksumData::NeutralLanguageCandidateCreation::Always;
            }
            else
            {
                neutralLanguage = scenario == UsageScenario::ResourcePack ?
                                      CContentChecksumData::NeutralLanguageCandidateCreation::IfNoCandidates :
                                      CContentChecksumData::NeutralLanguageCandidateCreation::Never;
            }

            result = indexer->Process(
                parameters.m_outputPath.GetFileNameNoExt(), neutralLanguage, parameters.m_inputArgs.contentChecksumValue, &status);
            Log::RetrieveMessages(&indexer->_logItems);
            if (FAILED(result))
            {
                Log::DisplayErrorMessage(L"Processing Resources", result);
                if (result == E_INVALIDARG && status.GetWhere() != nullptr)
                {
                    Log::DisplayErrorMessage(status.GetWhere(), E_INVALIDARG);
                }
                break;
            }

            const wchar_t* const schemaFile = parameters.m_schemaFilePath.GetFullPath();
            if (schemaFile[0] != L'\0')
            {
                StandalonePriFile* const builtPriFile = indexer->GetBuiltPriFile(&status);
                Microsoft::Resources::AutoDeletePtr<StandalonePriFile> builtPriFileOwner(builtPriFile);
                if (builtPriFile == nullptr || !StandalonePriFileXml::DumpPriFileToXmlFile(
                                                   schemaFile,
                                                   static_cast<Microsoft::Resources::PriFile*>(builtPriFile),
                                                   Microsoft::Resources::Tools::MakePri::PriDumpType::Schema,
                                                   parameters.m_outputOptions,
                                                   &status))
                {
                    result = status.GetHResult();
                    Log::DisplayErrorMessage(L"Creating schema file", result);
                }
            }
            if (FAILED(result))
            {
                break;
            }

            if (parameters.m_mappingFileFormat != Microsoft::Resources::Tools::MakePri::MappingFileFormat::NoMapping)
            {
                result = indexer->GenerateMappingFiles(
                    static_cast<Microsoft::Resources::Indexers::MappingFileFormat>(parameters.m_mappingFileFormat),
                    parameters.GetOutputFolder(),
                    &status);
                if (FAILED(result))
                {
                    break;
                }
            }

            const bool overwrite = parameters.m_inputArgs.overwrite || parameters.m_inputArgs.verbose || parameters.m_inputArgs.autoMerge ||
                                   parameters.m_inputArgs.diagnosticLogging;
            result = fileOperations.MoveFiles(overwrite, &status);

            if (SUCCEEDED(FileOperations::s_DeleteFolderAndContents(temporaryFolder.GetRef(), &status)))
            {
                RemoveDirectoryW(temporaryFolder.GetRef());
            }

            if (FAILED(result))
            {
                status.DiagnosticLogA("Move Failed");
                Log::Error(MAKEPRI_STRING_FILE_MOVE_FAILED_FROM_TO, result, temporaryFolder.GetRef(), parameters.GetOutputFolder());
            }
        } while (false);

        delete indexer;
        status.DiagnosticLogWithErrorCodeA("Makepri new/resourcepack/versioned", result);
    }
    catch (const std::bad_alloc&)
    {
        if (status.Succeeded())
        {
            status.SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 589, L"", 0);
            result = E_OUTOFMEMORY;
        }
    }
    catch (...)
    {
        result = E_ABORT;
    }

    if (SUCCEEDED(result))
    {
        Log::WriteWorkOutput(MAKEPRI_STRING_SUCCESSFULLY_COMPLETED);
    }
    return result;
}
