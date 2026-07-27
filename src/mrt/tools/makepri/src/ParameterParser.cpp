#include "StdAfx.h"

#include "ParameterParser.h"

namespace Microsoft::Resources::Tools::MakePri
{

bool StringsEqual(const wchar_t* left, const wchar_t* right)
{
    return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, left, -1, right, -1) == CSTR_EQUAL;
}

ParameterParser::ParameterParser(InputArgs& inputArgs) : m_inputArgs(inputArgs) {}

HRESULT ParameterParser::Parse(int argc, wchar_t* const argv[])
{
    if (argc < 2)
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_INSUFFICIENT_PARAMETERS, E_INVALIDARG);
        return SetTrueOnce(m_inputArgs.help);
    }

    int optionCount = argc - 1;
    wchar_t* const* options = argv + 1;
    HRESULT result = S_OK;

    if (StringsEqual(argv[1], L"New"))
    {
        result = AssignScenarioOnce(UsageScenario::New);
        if (SUCCEEDED(result))
        {
            Log::InfoVerbose(MAKEPRI_STRING_USAGE_SCENARIO_NEW_SPECIFIED);
        }
    }
    else if (StringsEqual(argv[1], L"Versioned"))
    {
        result = AssignScenarioOnce(UsageScenario::Versioned);
        if (SUCCEEDED(result))
        {
            Log::InfoVerbose(MAKEPRI_STRING_USAGE_SCENARIO_VERSIONED_SPECIFIED);
        }
    }
    else if (StringsEqual(argv[1], L"ResourcePack"))
    {
        result = AssignScenarioOnce(UsageScenario::ResourcePack);
        if (SUCCEEDED(result))
        {
            Log::InfoVerbose(MAKEPRI_STRING_USAGE_SCENARIO_RESOURCEPACK_SPECIFIED);
        }
    }
    else if (StringsEqual(argv[1], L"Dump"))
    {
        result = AssignScenarioOnce(UsageScenario::Dump);
        if (SUCCEEDED(result))
        {
            Log::InfoVerbose(MAKEPRI_STRING_USAGE_SCENARIO_DUMP_SPECIFIED);
        }
    }
    else if (StringsEqual(argv[1], L"CreateConfig"))
    {
        result = AssignScenarioOnce(UsageScenario::CreateConfig);
    }
    else if (StringsEqual(argv[1], L"Help"))
    {
        result = SetTrueOnce(m_inputArgs.help);
    }
    else
    {
        return ParseOptions(optionCount, options);
    }

    if (result < 0)
    {
        return result;
    }

    options = argv + 2;
    optionCount = argc - 2;
    if (m_inputArgs.help || optionCount <= 0)
    {
        return S_OK;
    }

    return ParseOptions(optionCount, options);
}

HRESULT ParameterParser::ParseOptions(int argc, wchar_t* const argv[])
{
    for (int index = 0; index < argc; ++index)
    {
        const wchar_t* argument = argv[index];
        if (argument[0] != L'-' && argument[0] != L'/')
        {
            Log::StoreErrorMessage(MAKEPRI_STRING_UNKNOWN_OPTION_SPECIFIED, E_INVALIDARG, argument);
            return E_INVALIDARG;
        }

        const wchar_t* option = argument + 1;
        if (StringsEqual(option, L"verbose") || StringsEqual(option, L"v"))
        {
            HRESULT result = SetTrueOnce(m_inputArgs.verbose);
            if (result < 0)
            {
                return result;
            }
            result = SetTrueOnce(Log::_bUseVerbose);
            if (result < 0)
            {
                return result;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_VERBOSE_SPECIFIED);
        }
        else if (StringsEqual(option, L"help") || StringsEqual(option, L"h") || StringsEqual(option, L"?"))
        {
            const HRESULT result = SetTrueOnce(m_inputArgs.help);
            if (result < 0)
            {
                return result;
            }
        }
        else if (StringsEqual(option, L"Overwrite") || StringsEqual(option, L"o"))
        {
            const HRESULT result = SetTrueOnce(m_inputArgs.overwrite);
            if (result < 0)
            {
                return result;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_OVERWRITE_SPECIFIED);
        }
        else if (StringsEqual(option, L"VersionMajor") || StringsEqual(option, L"vma"))
        {
            if (++index >= argc)
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
                return E_INVALIDARG;
            }
            if (m_inputArgs.versionMajor != 0)
            {
                Log::Error(MAKEPRI_STRING_SAME_OPTION_WAS_SET_MULTIPLE_TIMES, HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
                return HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG);
            }

            const int version = _wtoi(argv[index]);
            if (version <= 0)
            {
                Log::Error(MAKEPRI_STRING_INVALID_VERSION_SPECIFIED, E_INVALIDARG);
                return E_INVALIDARG;
            }
            m_inputArgs.versionMajor = version;
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_VERSIONMAJOR_SPECIFIED);
            Log::Warning(MAKEPRI_STRING_VERSIONMAJOR_VMA_INPUT_PARAMETER_HAS_BEEN, S_OK);
        }
        else if (StringsEqual(option, L"DumpType") || StringsEqual(option, L"dt"))
        {
            if (++index >= argc)
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
                return E_INVALIDARG;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_DUMPTYPE_SPECIFIED, argument);
            const HRESULT result = AssignDumpTypeOnce(argv[index]);
            if (result < 0)
            {
                return result;
            }
        }
        else if (StringsEqual(option, L"MappingFile") || StringsEqual(option, L"mf"))
        {
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_MAPPINGFILE_SPECIFIED);
            if (++index >= argc)
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
                return E_INVALIDARG;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_MAPPINGFILE_SPECIFIED, argument);
            if (!StringsEqual(argv[index], L"appx"))
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_IS_NOT_SUPPORTED_MAPPING_FILE_FORMAT, E_INVALIDARG, argv[index]);
                return E_INVALIDARG;
            }
            m_inputArgs.mappingFileFormat = MappingFileFormat::AppX;
        }
        else if (StringsEqual(option, L"AutoMerge") || StringsEqual(option, L"am"))
        {
            const HRESULT result = SetTrueOnce(m_inputArgs.autoMerge);
            if (result < 0)
            {
                return result;
            }
            Log::InfoVerbose(MAKEPRI_STRING_USAGE_SCENARIO_DUMP_SPECIFIED);
        }
        else if (StringsEqual(option, L"ReverseMap") || StringsEqual(option, L"rm"))
        {
            const HRESULT result = SetTrueOnce(m_inputArgs.reverseMap);
            if (result < 0)
            {
                return result;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_REVERSEMAP_SPECIFIED);
        }
        else if (StringsEqual(option, L"PlatformVersion") || StringsEqual(option, L"pv"))
        {
            if (++index >= argc)
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
                return E_INVALIDARG;
            }
            Log::InfoVerbose(MAKEPRI_STRING_OPTION_PLATFORM_SPECIFIED, argv[index]);
            const HRESULT result = AssignPlatformVersionOnce(argv[index], &m_inputArgs.platformVersion);
            if (result < 0)
            {
                return result;
            }
        }
        else
        {
            const wchar_t** destination = nullptr;
            bool* missingValueFlag = nullptr;

            if (StringsEqual(option, L"ProjectRoot") || StringsEqual(option, L"pr"))
            {
                destination = &m_inputArgs.projectRoot;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_PROJECTROOT_SPECIFIED);
            }
            else if (StringsEqual(option, L"ConfigXML") || StringsEqual(option, L"cf"))
            {
                destination = &m_inputArgs.configXml;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_CONFIGXML_SPECIFIED);
            }
            else if (StringsEqual(option, L"IndexLog") || StringsEqual(option, L"il"))
            {
                destination = &m_inputArgs.indexLog;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_INDEXLOG_FILE_SPECIFIED);
            }
            else if (StringsEqual(option, L"OutputFile") || StringsEqual(option, L"of"))
            {
                destination = &m_inputArgs.outputFile;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_OUTPUTFILE_SPECIFIED);
            }
            else if (StringsEqual(option, L"IndexName") || StringsEqual(option, L"in"))
            {
                destination = &m_inputArgs.indexName;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_INDEXNAME_SPECIFIED);
            }
            else if (StringsEqual(option, L"Manifest") || StringsEqual(option, L"mn"))
            {
                destination = &m_inputArgs.manifest;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_MANIFEST_SPECIFIED);
            }
            else if (StringsEqual(option, L"Default") || StringsEqual(option, L"dq"))
            {
                destination = &m_inputArgs.defaultQualifiers;
                Log::InfoVerbose(MAKEPRI_STRING_DEFAULTQUALIFIER_SPECIFIED, m_inputArgs.defaultQualifiers);
            }
            else if (StringsEqual(option, L"IndexFile") || StringsEqual(option, L"if"))
            {
                destination = &m_inputArgs.indexFile;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_INDEXFILE_SPECIFIED);
            }
            else if (StringsEqual(option, L"OutputOptions") || StringsEqual(option, L"oo"))
            {
                destination = &m_inputArgs.outputOptions;
                missingValueFlag = &m_inputArgs.outputOptionsError;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_OUTPUTOPTIONS_SPECIFIED);
            }
            else if (StringsEqual(option, L"IndexOptions") || StringsEqual(option, L"io"))
            {
                destination = &m_inputArgs.indexOptions;
                missingValueFlag = &m_inputArgs.indexOptionsError;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_INDEXOPTIONS_SPECIFIED);
            }
            else if (StringsEqual(option, L"SchemaFile") || StringsEqual(option, L"sf"))
            {
                destination = &m_inputArgs.schemaFile;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_SCHEMAFILE_SPECIFIED);
            }
            else if (StringsEqual(option, L"ExtensionDll") || StringsEqual(option, L"ex"))
            {
                destination = &m_inputArgs.extensionDll;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_EXTENSIONDLL_SPECIFIED, index + 1 < argc ? argv[index + 1] : L"");
            }
            else if (StringsEqual(option, L"ExternalSchema") || StringsEqual(option, L"es"))
            {
                destination = &m_inputArgs.externalSchema;
                Log::InfoVerbose(MAKEPRI_STRING_OPTION_EXTERNAL_SCHEMA_SPECIFIED, index + 1 < argc ? argv[index + 1] : L"");
            }
            else
            {
                Log::StoreErrorMessage(MAKEPRI_STRING_UNKNOWN_OPTION_SPECIFIED, E_INVALIDARG, option);
                return E_INVALIDARG;
            }

            if (++index >= argc)
            {
                if (missingValueFlag != nullptr)
                {
                    *missingValueFlag = true;
                }
                Log::StoreErrorMessage(MAKEPRI_STRING_EXPECTED_PARAMETER_NOT_FOUND, E_INVALIDARG);
                return E_INVALIDARG;
            }
            if (*destination != nullptr)
            {
                Log::Error(MAKEPRI_STRING_SAME_OPTION_WAS_SET_MULTIPLE_TIMES, HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
                return HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG);
            }
            *destination = argv[index];
        }
    }

    return S_OK;
}

HRESULT ParameterParser::SetTrueOnce(bool& value)
{
    if (value)
    {
        Log::Error(MAKEPRI_STRING_SAME_OPTION_WAS_SET_MULTIPLE_TIMES, HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
        return HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG);
    }
    value = true;
    return S_OK;
}

HRESULT ParameterParser::AssignScenarioOnce(UsageScenario scenario)
{
    if (m_inputArgs.scenario != UsageScenario::None)
    {
        Log::Error(MAKEPRI_STRING_SAME_OPTION_WAS_SET_MULTIPLE_TIMES, HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
        return HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG);
    }
    m_inputArgs.scenario = scenario;
    return S_OK;
}

HRESULT ParameterParser::AssignDumpTypeOnce(const wchar_t* value)
{
    // Despite its name, the original function does not reject repeated /DumpType options.
    if (value == nullptr)
    {
        return S_OK;
    }
    if (StringsEqual(value, L"Basic"))
    {
        m_inputArgs.dumpType = PriDumpType::Basic;
    }
    else if (StringsEqual(value, L"Detailed"))
    {
        m_inputArgs.dumpType = PriDumpType::Detailed;
    }
    else if (StringsEqual(value, L"Schema"))
    {
        m_inputArgs.dumpType = PriDumpType::Schema;
    }
    else if (StringsEqual(value, L"Summary"))
    {
        m_inputArgs.dumpType = PriDumpType::Summary;
    }
    else
    {
        Log::StoreErrorMessage(MAKEPRI_STRING_UNKNOWN_OPTION_SPECIFIED, E_INVALIDARG, value);
        return E_INVALIDARG;
    }
    return S_OK;
}

HRESULT ParameterParser::AssignPlatformVersionOnce(const wchar_t* const value, MrmPlatformVersionInternal* const platformVersion)
{
    if (*platformVersion != MrmPlatformVersionInternal::DefaultPlatformVersion)
    {
        Log::Error(MAKEPRI_STRING_SAME_OPTION_WAS_SET_MULTIPLE_TIMES, HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
        return HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG);
    }

    MrmPlatformVersionInternal version {};
    if (!Indexers::CUtilities::GetVersionFromString(value, nullptr, &version))
    {
        Log::Error(MAKEPRI_STRING_INVALID_PLATFORM_VERSION, E_INVALIDARG, value);
        return E_INVALIDARG;
    }
    *platformVersion = version;
    return S_OK;
}

} // namespace Microsoft::Resources::Tools::MakePri
