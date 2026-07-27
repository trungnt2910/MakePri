#include "StdAfx.h"

#include <Log.h>

namespace Microsoft::Resources::Tools::MakePri
{
bool Log::_bUseVerbose {};
std::queue<Indexers::LogItem> Log::s_delayedMessageQueue;
std::queue<Indexers::LogItem> Log::s_messageQueue;

namespace
{
std::wstring FormatResourceString(const std::uint32_t stringId, va_list arguments)
{
    wchar_t format[1024] {};
    wchar_t message[1024] {};
    LoadStringW(nullptr, stringId, format, 1024);
    vswprintf_s(message, 1024, format, arguments);
    return message;
}
} // namespace

void Log::InfoVerbose(const std::uint32_t stringId, ...)
{
    if (!_bUseVerbose)
    {
        return;
    }

    va_list arguments;
    va_start(arguments, stringId);
    std::wstring message = FormatResourceString(stringId, arguments);
    va_end(arguments);
    s_messageQueue.emplace(MrmResourceIndexerMessageSeverityVerbose, stringId, std::move(message));
}

void Log::Error(const std::uint32_t stringId, const HRESULT error, ...)
{
    wchar_t format[1024] {};
    wchar_t formatted[1024] {};
    LoadStringW(nullptr, stringId, format, 1024);

    va_list arguments;
    va_start(arguments, error);
    vswprintf_s(formatted, 1024, format, arguments);
    va_end(arguments);

    std::wstring message = FormatErrorPrefix(L"ERROR:", stringId, error);
    message.append(formatted);
    s_messageQueue.emplace(MrmResourceIndexerMessageSeverityError, stringId, std::move(message));
}

void Log::Item(const std::uint32_t messageId, const MrmResourceIndexerMessageSeverity severity, const wchar_t* const message)
{
    static_cast<void>(severity);
    s_messageQueue.emplace(MrmResourceIndexerMessageSeverityInfo, messageId, std::wstring(message));
}

void Log::ItemWithFormat(const std::uint32_t messageId, const MrmResourceIndexerMessageSeverity severity, const wchar_t* const format, ...)
{
    wchar_t message[1030] {};
    va_list arguments;
    va_start(arguments, format);
    vswprintf_s(message, 1024, format, arguments);
    va_end(arguments);
    s_messageQueue.emplace(severity, messageId, std::wstring(message));
}

void Log::Warning(const std::uint32_t stringId, const HRESULT error, ...)
{
    wchar_t format[1024] {};
    wchar_t formatted[1024] {};
    LoadStringW(nullptr, stringId, format, 1024);

    std::wstring discardedPrefix = FormatErrorPrefix(L"WARNING:", stringId, error);
    discardedPrefix.append(format);

    va_list arguments;
    va_start(arguments, error);
    vswprintf_s(formatted, 1024, format, arguments);
    va_end(arguments);

    s_messageQueue.emplace(MrmResourceIndexerMessageSeverityInfo, stringId, std::wstring(formatted));
}

void Log::WriteWorkOutput(const std::uint32_t stringId, ...)
{
    va_list arguments;
    va_start(arguments, stringId);
    std::wstring message = FormatResourceString(stringId, arguments);
    va_end(arguments);
    s_messageQueue.emplace(MrmResourceIndexerMessageSeverityInfo, stringId, std::move(message));
}

void Log::NewLine() { Item(0, MrmResourceIndexerMessageSeverityInfo, L""); }

void Log::InfoUnderscored(const std::uint32_t stringId, ...)
{
    wchar_t resource[1030] {};
    LoadStringW(nullptr, stringId, resource, 1024);

    std::wstring message(resource);
    message.append(L"\r\n");
    std::uint32_t index = 0;
    while (true)
    {
        if (resource[index] == L':')
        {
            resource[index] = L'-';
            break;
        }
        if (resource[index] == L'\0')
        {
            break;
        }
        resource[index] = L'-';
        ++index;
        if (index >= 1024)
        {
            break;
        }
    }
    message.append(resource);
    Item(stringId, MrmResourceIndexerMessageSeverityInfo, message.c_str());
}

void Log::DisplayErrorMessage(const wchar_t* const message, const HRESULT error)
{
    DWORD code = static_cast<DWORD>(error);
    if ((static_cast<DWORD>(error) & 0xFFFF0000) == 0x80070000)
    {
        code = static_cast<WORD>(error);
    }

    wchar_t* systemMessage {};
    if (FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            code,
            0x400,
            reinterpret_cast<wchar_t*>(&systemMessage),
            0,
            nullptr) != 0)
    {
        const std::size_t messageLength = wcsnlen(message, 1024);
        const std::size_t systemLength = wcsnlen(systemMessage, 1024);
        auto* combined = static_cast<wchar_t*>(LocalAlloc(LMEM_ZEROINIT, 2 * (systemLength + messageLength) + 80));
        if (combined != nullptr &&
            SUCCEEDED(
                StringCchPrintfW(combined, LocalSize(combined) / sizeof(wchar_t), L"%s failed with error: %s", message, systemMessage)))
        {
            Error(MAKEPRI_STRING_FORMAT_STRING, error, combined);
        }
        else
        {
            Error(MAKEPRI_STRING_FORMAT_STRING, error, systemMessage);
        }
        LocalFree(systemMessage);
        if (combined != nullptr)
        {
            LocalFree(combined);
        }
    }
    else
    {
        Error(MAKEPRI_STRING_FORMAT_STRING, error, message);
    }
}

void Log::StoreErrorMessage(const std::uint32_t stringId, const HRESULT error, ...)
{
    wchar_t format[1024] {};
    wchar_t formatted[1024] {};
    LoadStringW(nullptr, stringId, format, 1024);

    va_list arguments;
    va_start(arguments, error);
    vswprintf_s(formatted, 1024, format, arguments);
    va_end(arguments);

    std::wstring message = FormatErrorPrefix(L"ERROR:", stringId, error);
    message.append(formatted);
    s_delayedMessageQueue.emplace(MrmResourceIndexerMessageSeverityError, stringId, std::move(message));
}

bool Log::DisplayStoredErrorMessages()
{
    bool displayed {};
    while (!s_delayedMessageQueue.empty())
    {
        displayed = true;
        s_messageQueue.push(s_delayedMessageQueue.front());
        s_delayedMessageQueue.pop();
    }
    return displayed;
}

void Log::RetrieveMessages(const std::vector<Indexers::LogItem>* const messages)
{
    for (const Indexers::LogItem& message : *messages)
    {
        s_messageQueue.push(message);
    }
}

bool Log::Flush()
{
    bool flushed {};
    while (!s_messageQueue.empty())
    {
        const Indexers::LogItem& item = s_messageQueue.front();
        if (_bUseVerbose || item.severity != MrmResourceIndexerMessageSeverityVerbose)
        {
            FILE* const stream = item.severity == MrmResourceIndexerMessageSeverityError ? stderr : stdout;
            std::fwprintf(stream, item.message.c_str());
            std::fwprintf(stream, L"\n");
            s_messageQueue.pop();
            flushed = true;
        }
    }
    return flushed;
}

void Log::DisplayStatusErrorAndWarnings(IDefStatusEx* const status)
{
    for (const IDefStatus* const warning : status->GetWarningList())
    {
        if (warning != nullptr)
        {
            _DisplayMappedMsg(warning, true);
        }
    }
    if (status->Failed())
    {
        _DisplayMappedMsg(status, false);
    }
}

std::wstring Log::FormatErrorPrefix(const wchar_t* const prefix, const std::uint32_t stringId, const HRESULT error)
{
    wchar_t buffer[1024] {};
    const wchar_t* const format = error == S_OK ? L"%s %s%u: " : L"%s %s%u: 0x%8x - ";
    _snwprintf_s(buffer, 1024, _TRUNCATE, format, prefix, L"PRI", stringId, error);
    return buffer;
}

void Log::_WriteFormattedLine(
    const std::uint32_t stringId,
    const bool warning,
    const wchar_t* const prefix,
    const HRESULT error,
    const wchar_t* const description,
    const wchar_t* const where,
    const std::uint32_t line,
    const wchar_t* const repeatedDescription)
{
    const std::wstring formattedPrefix = FormatErrorPrefix(prefix, stringId, error);
    wchar_t resource[1024] {};
    LoadStringW(nullptr, stringId, resource, 1024);

    const wchar_t* optionalWhere = L"";
    if (line == 0 && where != nullptr)
    {
        optionalWhere = where;
    }

    ULONG_PTR arguments[4] {
        reinterpret_cast<ULONG_PTR>(formattedPrefix.c_str()),
        reinterpret_cast<ULONG_PTR>(description),
        reinterpret_cast<ULONG_PTR>(optionalWhere),
        reinterpret_cast<ULONG_PTR>(repeatedDescription),
    };
    wchar_t* message {};
    if (FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            resource,
            0,
            0,
            reinterpret_cast<wchar_t*>(&message),
            0,
            reinterpret_cast<va_list*>(arguments)) != 0)
    {
        ItemWithFormat(
            stringId, warning ? MrmResourceIndexerMessageSeverityWarning : MrmResourceIndexerMessageSeverityError, L"%s", message);
        LocalFree(message);
    }
    else
    {
        wchar_t errorFormat[1024] {};
        LoadStringW(nullptr, MAKEPRI_STRING_FORMAT_MESSAGE_FAILED_WITH_0X, errorFormat, 1024);
        ItemWithFormat(stringId, MrmResourceIndexerMessageSeverityError, errorFormat, GetLastError());
    }
}

std::uint32_t Log::_GetErrorStringId(const HRESULT error)
{
    if (error > E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE)
    {
        if (error <= E_MRMEXT_APISET_NOT_PRESENT)
        {
            if (error == E_MRMEXT_APISET_NOT_PRESENT)
            {
                return MAKEPRI_STRING_EXTENSION_REQUIREMENTS_NOT_MET;
            }
            switch (error)
            {
            case E_DEF_PRICONFIG_PACKAGING_INVALID_OS:
                return MAKEPRI_STRING_PACKAGING_NODE_UNSUPPORTED_TARGET_OS;
            case E_DEF_PRICONFIG_INVALID_IDM_AM:
                return MAKEPRI_STRING_AUTOMERGE_CONFLICTS_WITH_PACKAGING;
            case E_DEF_PRICONFIG_INVALID_IDM_WIN8:
                return MAKEPRI_STRING_DEPLOYMENT_MERGEABLE_UNSUPPORTED_TARGET_OS;
            case E_DEF_FSI_FILE_PATH_TOO_LONG:
                return MAKEPRI_STRING_CONFIG_FILE_PATH_TOO_LONG;
            case E_DEF_FSI_UNSUPPORTED_DIR_TYPE:
            case E_DEF_FSI_INVALID_FILE_TYPE:
                return MAKEPRI_STRING_CONFIG_FILE_OR_FOLDER_TYPE_INVALID;
            case E_DEF_FSI_INVALID_DELIMITER:
                return MAKEPRI_STRING_CONFIG_QUALIFIER_DELIMITER_INVALID;
            case E_DEF_FSI_INSTANTIATION_FAILED:
                return MAKEPRI_STRING_INDEXER_INSTANTIATION_FAILED;
            case E_DEF_FSI_RESJSON_DUPLICATE:
                return MAKEPRI_STRING_JSON_DUPLICATE_ENTRY;
            case E_DEF_FSI_RESJSON_MULTIPLE_OBJECTS:
                return MAKEPRI_STRING_JSON_MULTIPLE_OBJECTS;
            case E_DEF_FSI_RESJSON_MISSING_COMMA_BRACE:
                return MAKEPRI_STRING_JSON_MISSING_COMMA_OR_OBJECT_END;
            case E_DEF_FSI_RESJSON_MISSING_OBJECT_NAME:
                return MAKEPRI_STRING_JSON_INVALID_COMMA_OR_MISSING_OBJECT_NAME;
            case E_DEF_FSI_RESJSON_MISSING_COLON:
                return MAKEPRI_STRING_JSON_MISSING_COLON;
            case E_DEF_FSI_RESJSON_INVALID_INPUTSTR:
                return MAKEPRI_STRING_JSON_INVALID_STRING_ENTRY;
            case E_DEF_FSI_RESJSON_INVALID_CHAR:
                return MAKEPRI_STRING_JSON_INVALID_CHARACTER;
            case E_DEF_FSI_RESJSON_MISSING_ROOT_OBJ:
                return MAKEPRI_STRING_JSON_MISSING_ROOT_OBJECT;
            case E_DEF_FSI_RESJSON_INVALID_PROP_OBJ:
                return MAKEPRI_STRING_JSON_INVALID_PROPERTY;
            case E_DEF_FSI_RESJSON_INVALID_ITEM_TYPE:
                return MAKEPRI_STRING_JSON_INVALID_ITEM_TYPE;
            case E_DEF_FSI_RESJSON_NODE_DEPTH_MAX_EXCEEDED:
                return MAKEPRI_STRING_JSON_MAX_NODE_DEPTH_EXCEEDED;
            case E_DEF_QUALAPPL_INVALID_QUALIFIER:
                return MAKEPRI_STRING_INVALID_QUALIFIER;
            case E_DEF_QUALAPPL_INVALID_QUAL_FILENAME:
                return MAKEPRI_STRING_INVALID_RESOURCE_QUALIFIER_IN_FILENAME;
            case E_DEF_QUALAPPL_MISSING_DEFAULT_LANG:
                return MAKEPRI_STRING_DEFAULT_LANGUAGE_RESOURCES_MISSING;
            case E_DEF_IBC_INVALID_CANDIDATE:
                return MAKEPRI_STRING_NAMED_RESOURCE_OUTSIDE_PROJECT_ROOT;
            case E_DEF_IBC_CANDIDATE_NOT_EMBEDDED:
                return MAKEPRI_STRING_FILE_TOO_LARGE_TO_EMBED;
            case E_DEF_PRICONFIG_PACKAGING_INVALID_IDM:
                return MAKEPRI_STRING_DEPLOYMENT_MERGEABLE_FALSE_WITH_PACKAGING;
            case E_DEF_PRICONFIG_INVALID_MRT_ARP:
                return MAKEPRI_STRING_PACKAGING_MULTIPLE_MODES;
            case E_DEF_PRICONFIG_INVALID_MRP_MULTIQUAL:
                return MAKEPRI_STRING_RESOURCE_PACKAGE_MULTIPLE_QUALIFIERS;
            case E_DEF_PRICONFIG_INVALID_MRP_DUP_NAME:
                return MAKEPRI_STRING_RESOURCE_PACKAGE_ALREADY_DEFINED;
            case E_DEF_PRICONFIG_INVALID_MRP_DUP_QSI:
                return MAKEPRI_STRING_CONFIG_MULTIPLE_INSTANCES;
            case E_DEF_PRICONFIG_INVALID_MRP_DEFAULT:
                return MAKEPRI_STRING_DEFAULT_QUALIFIER_CANDIDATE_IN_RESOURCE_PACKAGE;
            case E_DEF_PRICONFIG_INVALID_MRP_EMPTY:
                return MAKEPRI_STRING_RESOURCE_PACKAGE_NO_CANDIDATES;
            case E_DEF_PRICONFIG_NON_ARP_QUALIFIER:
                return MAKEPRI_STRING_AUTOMATIC_RESOURCE_PACKAGE_UNSUPPORTED_QUALIFIER;
            case E_DEF_IBC_SCOPE_ITEM_CONFLICT:
                return MAKEPRI_STRING_RESOURCE_AND_SCOPE_CONFLICT;
            case E_DEF_IBC_CONFLICTING_VALUES:
                return MAKEPRI_STRING_CONFLICTING_RESOURCE_VALUES;
            case E_DEF_QUALAPPL_VALUE_NOT_ALLOWED:
                return MAKEPRI_STRING_QUALIFIER_VALUE_NOT_ALLOWED;
            case E_DEF_PRICONFIG_INVALID_ALLOWED_NODE:
                return MAKEPRI_STRING_ALLOWED_NODE_PARSE_FAILED;
            case E_DEF_FSI_SPECIAL_FILE_IGNORED:
                return MAKEPRI_STRING_SPECIAL_FILE_OR_FOLDER_IGNORED;
            case E_DEF_UNSUPPORTED_FILE_TYPE:
                return MAKEPRI_STRING_UNSUPPORTED_FILE_TYPE_FOR_SCHEMA_FILE;
            default:
                return MAKEPRI_STRING_UNSPECIFIED_ERROR;
            }
        }
        if (error == E_MRMEXT_EXTENSION_NOT_SUPPORTED)
        {
            return MAKEPRI_STRING_EXTENSION_UNSUPPORTED;
        }
        if (error == E_MRM_READ_ONLY_SCHEMA)
        {
            return MAKEPRI_STRING_READ_ONLY_RESOURCE_MAP;
        }
        if (error == E_MRM_TOO_MANY_RESOURCES)
        {
            return MAKEPRI_STRING_TOO_MANY_RESOURCE_NAMES;
        }
        if (error == E_MRM_NO_DEFAULT_OR_NEUTRAL_VALUE)
        {
            return MAKEPRI_STRING_NO_DEFAULT_OR_NEUTRAL_RESOURCE;
        }
        if (error == E_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILES)
        {
            return MAKEPRI_STRING_EXTERNAL_SCHEMA_REQUIRED_TO_DUMP_RESOURCE_PACKAGE;
        }
        return error == E_MRM_SCHEMALESS_PRI_LOAD_FAILED ? MAKEPRI_STRING_MAIN_PRI_REQUIRED_TO_INDEX_RESOURCE_PACKAGE :
                                                           MAKEPRI_STRING_UNSPECIFIED_ERROR;
    }

    if (error == E_DEF_PRICONFIG_INVALID_ATTRIB_VALUE)
    {
        return MAKEPRI_STRING_CONFIG_ATTRIBUTE_VALUE_INVALID;
    }
    if (error <= E_DEFFILE_BAD_SECTION_TYPE)
    {
        if (error != E_DEFFILE_BAD_SECTION_TYPE)
        {
            if (error <= E_DEF_FILE_NOT_FOUND)
            {
                if (error == E_DEF_FILE_NOT_FOUND)
                {
                    return MAKEPRI_STRING_FILE_NOT_FOUND;
                }
                if (error > HRESULT_FROM_WIN32(ERROR_MRM_TOO_MANY_RESOURCES))
                {
                    switch (error)
                    {
                    case HRESULT_FROM_WIN32(ERROR_MRM_MISSING_DEFAULT_LANGUAGE):
                        return MAKEPRI_STRING_CONFIG_DEFAULT_LANGUAGE_MISSING;
                    case HRESULT_FROM_WIN32(ERROR_MRM_SCOPE_ITEM_CONFLICT):
                        return MAKEPRI_STRING_RESOURCE_AND_SCOPE_CONFLICT;
                    case NTE_BAD_SIGNATURE:
                        return MAKEPRI_STRING_EXTENSION_NOT_MICROSOFT_SIGNED;
                    case E_DEF_UNSUPPORTED_VERSION:
                        return MAKEPRI_STRING_MULTIPLE_ENTRIES_FOUND;
                    case E_DEF_INVALID_ATTRIBUTE_VALUE:
                        return MAKEPRI_STRING_INVALID_QUALIFIER;
                    default:
                        return MAKEPRI_STRING_UNSPECIFIED_ERROR;
                    }
                }
                if (error == HRESULT_FROM_WIN32(ERROR_MRM_TOO_MANY_RESOURCES))
                {
                    return MAKEPRI_STRING_TOO_MANY_RESOURCE_NAMES;
                }
                if (error == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    return MAKEPRI_STRING_FILE_NOT_FOUND;
                }
                if (error == E_ACCESSDENIED)
                {
                    return MAKEPRI_STRING_ACCESS_DENIED_PROCESSING_RESOURCE;
                }
                if (error == HRESULT_FROM_WIN32(ERROR_MRM_RUNTIME_NO_DEFAULT_OR_NEUTRAL_RESOURCE))
                {
                    return MAKEPRI_STRING_NO_DEFAULT_OR_NEUTRAL_RESOURCE;
                }
                if (error == HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE))
                {
                    return MAKEPRI_STRING_INVALID_QUALIFIER;
                }
                return error == HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY) ? MAKEPRI_STRING_CONFLICTING_RESOURCE_VALUES :
                                                                                MAKEPRI_STRING_UNSPECIFIED_ERROR;
            }
            if (error > E_DEFFILE_BAD_FILE_TRAILER)
            {
                if (error != E_DEFFILE_FILE_DATA_TOO_SMALL && error != E_DEFFILE_BAD_TOC_OFFSET && error != E_DEFFILE_BAD_DATA_OFFSET &&
                    error != E_DEFFILE_BAD_SECTION_HEADER)
                {
                    return MAKEPRI_STRING_UNSPECIFIED_ERROR;
                }
            }
            else if (error != E_DEFFILE_BAD_FILE_TRAILER)
            {
                if (error == E_DEF_ACCESS_DENIED)
                {
                    return MAKEPRI_STRING_ACCESS_DENIED_PROCESSING_RESOURCE;
                }
                if (error != E_DEFFILE_BAD_MAGIC_NUMBER && error != E_DEFFILE_BAD_TOC_ENTRY && error != E_DEFFILE_TOC_MISMATCH &&
                    error != E_DEFFILE_BAD_SECTION_TRAILER)
                {
                    return MAKEPRI_STRING_UNSPECIFIED_ERROR;
                }
            }
        }
        return MAKEPRI_STRING_PRI_FILE_INVALID;
    }
    if (error <= E_DEFFILE_SECTION_DATA_TOO_LARGE)
    {
        if (error == E_DEFFILE_SECTION_DATA_TOO_LARGE || static_cast<std::uint32_t>(error - E_DEFFILE_BAD_SECTION_QUALIFIER) <= 8)
        {
            return MAKEPRI_STRING_PRI_FILE_INVALID;
        }
        return MAKEPRI_STRING_UNSPECIFIED_ERROR;
    }
    if (error > E_DEF_PRICONFIG_UKNOWN)
    {
        switch (error)
        {
        case E_DEF_PRICONFIG_MULTIPLE_CONFIG:
            return MAKEPRI_STRING_DUPLICATE_INDEXER;
        case E_DEF_PRICONFIG_NO_MAIN_PACKAGE:
            return MAKEPRI_STRING_MAIN_PACKAGE_NODE_NOT_FOUND;
        case E_DEF_PRICONFIG_EMPTY_PACKAGING:
            return MAKEPRI_STRING_PACKAGING_NODE_EMPTY;
        case E_DEF_PRICONFIG_INVALID_PATH:
            return MAKEPRI_STRING_CONFIG_PATH_INVALID;
        case E_DEF_PRICONFIG_INVALID_QUAL:
            return MAKEPRI_STRING_CONFIG_QUALIFIER_INVALID;
        default:
            return MAKEPRI_STRING_UNSPECIFIED_ERROR;
        }
    }
    switch (error)
    {
    case E_DEF_PRICONFIG_UKNOWN:
        return MAKEPRI_STRING_CONFIG_PROCESSING_UNKNOWN_ERROR;
    case E_DEFFILE_BUILD_LINK_TO_LINKED_RESOURCE:
        return MAKEPRI_STRING_RESOURCE_LINK_TARGET_IS_LINK;
    case E_DEFFILE_BUILD_LINK_TO_INCOMPATIBLE_RESOURCE:
        return MAKEPRI_STRING_LINKED_RESOURCE_CANDIDATE_CONFLICT;
    case E_DEF_XML_SCHEMA_VALIDATION_FAIL:
        return MAKEPRI_STRING_SCHEMA_VALIDATION_FAILED;
    case E_DEF_XML_NODE_NOT_FOUND:
        return MAKEPRI_STRING_CONFIG_NODE_NOT_FOUND;
    case E_DEF_XML_ATTRIB_NOT_FOUND:
        return MAKEPRI_STRING_CONFIG_ATTRIBUTE_NOT_FOUND;
    case E_DEF_PRICONFIG_MISSING_ATTRIB:
        return MAKEPRI_STRING_CONFIG_ATTRIBUTE_NOT_FOUND_IN_FILE;
    default:
        return MAKEPRI_STRING_UNSPECIFIED_ERROR;
    }
}

void Log::_DisplayMappedMsg(const IDefStatus* const status, const bool warning)
{
    wchar_t prefix[1024] {};
    LoadStringW(nullptr, static_cast<std::uint32_t>(warning) + MAKEPRI_STRING_ERROR_PREFIX, prefix, 1024);
    const HRESULT error = status->GetWhat();
    const wchar_t* const description = status->GetDesc();
    _WriteFormattedLine(
        _GetErrorStringId(status->GetWhat()),
        warning,
        prefix,
        error,
        description,
        status->GetWhere(),
        status->GetLine(),
        status->GetDesc());
}

void Log::StoreLastErrorInfo()
{
    IErrorInfo* errorInfo {};
    GetErrorInfo(0, &errorInfo);
    if (errorInfo != nullptr)
    {
        Common::AutoBStr description;
        if (SUCCEEDED(errorInfo->GetDescription(&description)))
        {
            s_delayedMessageQueue.emplace(MrmResourceIndexerMessageSeverityError, MAKEPRI_STRING_FORMAT_STRING, std::wstring(description));
        }
        errorInfo->Release();
    }
}

void Log::PrintUsage(InputArgs* const args)
{
    WriteWorkOutput(MAKEPRI_STRING_PRODUCT_NAME);
    WriteWorkOutput(MAKEPRI_STRING_COPYRIGHT);
    NewLine();
    if (DisplayStoredErrorMessages())
    {
        NewLine();
    }

    switch (args->scenario)
    {
    case UsageScenario::None:
        InfoUnderscored(MAKEPRI_STRING_USAGE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_COMMAND_OPTIONS);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_EXAMPLE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_NEW_PR_C_MYAPP);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_DESCRIPTION);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_CREATES_DUMPS_AND_DOES);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_COMMAND_OPTIONS);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_CREATECONFIG_CREATES_PRI_CONFIG);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_NEW_CREATES_NEW_PRI);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_VERSIONED_CREATES_PRI_FILE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_RESOURCEPACK_CREATES_PRI_FILE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_DUMP_DUMPS_CONTENTS_OF);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_HELP_SHOW_THIS_HELP);
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_HELP);
        WriteWorkOutput(MAKEPRI_STRING_SPECIFY_COMMAND_WITH_HELP_FOR_MORE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_NEW);
        NewLine();
        break;

    case UsageScenario::New:
        InfoUnderscored(MAKEPRI_STRING_USAGE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_NEW_PR_PROJECT_ROOT);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_EXAMPLE);
        WriteWorkOutput(MAKEPRI_STRING_NEW_COMMAND_EXAMPLE);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_DESCRIPTION);
        WriteWorkOutput(MAKEPRI_STRING_CREATES_PRI_FILE_AT_OUTPUTFILE_BY);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_REQUIRED_PARAMETERS);
        WriteWorkOutput(MAKEPRI_STRING_PROJECTROOT_PR_FOLDERPATH_ROOT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_CONFIGXML_CF_FILEPATH_CONFIGURATION_FILE_LOCATION);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_OPTIONS);
        WriteWorkOutput(MAKEPRI_STRING_OUTPUTFILE_OF_FILEPATH_OUTPUT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_MANIFEST_MN_FILEPATH_LOCATION_OF_APPLICATION);
        WriteWorkOutput(MAKEPRI_STRING_INDEXNAME_IN_STRING_NAME_FOR_GENERATED);
        WriteWorkOutput(MAKEPRI_STRING_VERSIONMAJOR_VMA_INTEGER_DEPRECATED_MAJOR_VERSION);
        WriteWorkOutput(MAKEPRI_STRING_INDEXLOG_IL_FILEPATH_XML_LOG_OF);
        WriteWorkOutput(MAKEPRI_STRING_AUTOMERGE_AM_THIS_FLAG_IS_NOT);
        WriteWorkOutput(MAKEPRI_STRING_REVERSEMAP_RM_GENERATE_REVERSE_MAPPING_SECTION);
        WriteWorkOutput(MAKEPRI_STRING_MAPPINGFILE_MF_MAPPINGFILETYPE_GENERATE_MAPPING_FILE);
        WriteWorkOutput(MAKEPRI_STRING_SCHEMAFILE_SF_FILEPATH_OUTPUT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_INDEXOPTIONS_IO_OPTIONS_OPTIONS_TO_PROVIDE);
        WriteWorkOutput(MAKEPRI_STRING_OVERWRITE_O_OVERWRITE_EXISTING_OUTPUT_FILE);
        WriteWorkOutput(MAKEPRI_STRING_VERBOSE_V_CAUSES_VERBOSE_MESSAGES_TO);
        WriteWorkOutput(MAKEPRI_STRING_HELP_H_DISPLAY_USAGE_HELP_TEXT);
        WriteWorkOutput(MAKEPRI_STRING_EXTENSIONDLL_EX_FILEPATH_LOCATION_OF_MRT);
        NewLine();
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_FOLDERPATH_IS_VALID_PATH_TO_FOLDER);
        WriteWorkOutput(MAKEPRI_STRING_FILEPATH_IS_PATH_TO_FILE_EITHER);
        WriteWorkOutput(MAKEPRI_STRING_MAPPINGFILETYPE_SUPPORTED_FILE_TYPE_S_APPX);
        NewLine();
        break;

    case UsageScenario::Versioned:
    case UsageScenario::ResourcePack:
        InfoUnderscored(MAKEPRI_STRING_USAGE);
        WriteWorkOutput(
            args->scenario == UsageScenario::Versioned ? MAKEPRI_STRING_MAKEPRI_EXE_VERSIONED_PR_PROJECT_ROOT :
                                                         MAKEPRI_STRING_MAKEPRI_EXE_RESOURCEPACK_PR_PROJECT_ROOT);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_EXAMPLE);
        WriteWorkOutput(
            args->scenario == UsageScenario::Versioned ? MAKEPRI_STRING_MAKEPRI_EXE_VERSIONED_O_IF_C :
                                                         MAKEPRI_STRING_MAKEPRI_EXE_RESOURCEPACK_O_IF_C);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_DESCRIPTION);
        WriteWorkOutput(
            args->scenario == UsageScenario::Versioned ? MAKEPRI_STRING_MAKEPRI_EXE_VERSIONED_CREATES_VERSIONED_PRI :
                                                         MAKEPRI_STRING_RESOURCEPACK_COMMAND_DESCRIPTION);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_REQUIRED_PARAMETERS);
        WriteWorkOutput(MAKEPRI_STRING_PROJECTROOT_PR_FOLDERPATH_ROOT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_CONFIGXML_CF_FILEPATH_CONFIGURATION_FILE_LOCATION);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_OPTIONS);
        WriteWorkOutput(MAKEPRI_STRING_OUTPUTFILE_OF_FILEPATH_OUTPUT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_INDEXFILE_IF_FILEPATH_LOCATION_OF_BASE);
        WriteWorkOutput(MAKEPRI_STRING_INDEXLOG_IL_FILEPATH_XML_LOG_OF);
        WriteWorkOutput(MAKEPRI_STRING_RESOURCEPACK_AUTOMERGE_OPTION_HELP);
        WriteWorkOutput(MAKEPRI_STRING_REVERSEMAP_RM_GENERATE_REVERSE_MAPPING_SECTION);
        WriteWorkOutput(MAKEPRI_STRING_MAPPINGFILE_MF_MAPPINGFILETYPE_GENERATE_MAPPING_FILE);
        WriteWorkOutput(MAKEPRI_STRING_SCHEMAFILE_SF_FILEPATH_OUTPUT_LOCATION_OF);
        WriteWorkOutput(MAKEPRI_STRING_INDEXOPTIONS_IO_OPTIONS_OPTIONS_TO_PROVIDE);
        WriteWorkOutput(MAKEPRI_STRING_OVERWRITE_O_OVERWRITE_EXISTING_OUTPUT_FILE);
        WriteWorkOutput(MAKEPRI_STRING_VERBOSE_V_CAUSES_VERBOSE_MESSAGES_TO);
        WriteWorkOutput(MAKEPRI_STRING_HELP_H_DISPLAY_USAGE_HELP_TEXT);
        WriteWorkOutput(MAKEPRI_STRING_EXTENSIONDLL_EX_FILEPATH_LOCATION_OF_MRT);
        NewLine();
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_FOLDERPATH_IS_VALID_PATH_TO_FOLDER);
        WriteWorkOutput(MAKEPRI_STRING_FILEPATH_IS_PATH_TO_FILE_EITHER);
        WriteWorkOutput(MAKEPRI_STRING_MAPPINGFILETYPE_SUPPORTED_FILE_TYPE_S_APPX);
        NewLine();
        break;

    case UsageScenario::Dump:
        InfoUnderscored(MAKEPRI_STRING_USAGE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_DUMP_OPTIONS);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_EXAMPLE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_DUMP_IF_C_MYAPP);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_DESCRIPTION);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_DUMP_OUTPUTS_DUMPED_XML);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_OPTIONS);
        WriteWorkOutput(MAKEPRI_STRING_DUMP_OUTPUT_FILE_OPTION_HELP);
        WriteWorkOutput(MAKEPRI_STRING_INDEXFILE_IF_FILEPATH_LOCATION_OF_PRI);
        WriteWorkOutput(MAKEPRI_STRING_DUMPTYPE_DT_STRING_FORMAT_OF_DUMPED);
        WriteWorkOutput(MAKEPRI_STRING_OVERWRITE_O_OVERWRITE_EXISTING_OUTPUT_FILE);
        WriteWorkOutput(MAKEPRI_STRING_VERBOSE_V_CAUSES_VERBOSE_MESSAGES_TO);
        WriteWorkOutput(MAKEPRI_STRING_OUTPUTOPTIONS_OO_OPTIONS_OPTIONS_TO_PROVIDE);
        WriteWorkOutput(MAKEPRI_STRING_EXTERNALSCHEMA_ES_FILEPATH_LOCATION_OF_EXTERNAL);
        WriteWorkOutput(MAKEPRI_STRING_EXTENSIONDLL_EX_FILEPATH_LOCATION_OF_MRT);
        WriteWorkOutput(MAKEPRI_STRING_HELP_H_DISPLAY_USAGE_HELP_TEXT);
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_DUMP_TYPE);
        WriteWorkOutput(MAKEPRI_STRING_EITHER_BASIC_DETAILED_SCHEMA_OR_SUMMARY);
        NewLine();
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_FOLDERPATH_IS_VALID_PATH_TO_FOLDER);
        WriteWorkOutput(MAKEPRI_STRING_FILEPATH_IS_PATH_TO_FILE_EITHER);
        NewLine();
        break;

    case UsageScenario::CreateConfig:
        InfoUnderscored(MAKEPRI_STRING_USAGE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_CREATECONFIG_CF_CONFIG_FILE);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_EXAMPLE);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_CREATECONFIG_CF_C_MYAPP);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_DESCRIPTION);
        WriteWorkOutput(MAKEPRI_STRING_MAKEPRI_EXE_CREATECONFIG_CREATES_PRI_CONFIGURATION);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_REQUIRED_PARAMETERS);
        WriteWorkOutput(MAKEPRI_STRING_CONFIGXML_CF_FILEPATH_CONFIGURATION_FILE_OUTPUT);
        WriteWorkOutput(MAKEPRI_STRING_DEFAULT_DQ_QUALIFIERS_DEFAULT_QUALIFIER_SET);
        NewLine();
        InfoUnderscored(MAKEPRI_STRING_OPTIONS);
        WriteWorkOutput(MAKEPRI_STRING_PLATFORM_PV_VERSION_PLATFORM_VERSION_TO);
        WriteWorkOutput(MAKEPRI_STRING_OVERWRITE_O_OVERWRITE_EXISTING_OUTPUT_FILE);
        WriteWorkOutput(MAKEPRI_STRING_HELP_H_DISPLAY_USAGE_HELP_TEXT);
        WriteWorkOutput(MAKEPRI_STRING_EXTENSIONDLL_EX_FILEPATH_LOCATION_OF_MRT);
        NewLine();
        NewLine();
        WriteWorkOutput(MAKEPRI_STRING_FOLDERPATH_IS_VALID_PATH_TO_FOLDER);
        WriteWorkOutput(MAKEPRI_STRING_QUALIFIERS_IS_VALID_QUALIFIER_TOKEN_I);
        NewLine();
        break;
    }

    if (args->indexOptionsError)
    {
        InfoUnderscored(MAKEPRI_STRING_INDEX_OPTIONS_ARE_COMMA_SEPARATED_LIST);
        WriteWorkOutput(MAKEPRI_STRING_HIDDENFILES_HF_INDEX_IGNORE_HIDDEN_FILES);
        WriteWorkOutput(MAKEPRI_STRING_LINKEDFILES_LF_INDEX_IGNORE_LINKED_FILES);
        WriteWorkOutput(MAKEPRI_STRING_INDEXOPTIONS_ARE_DISABLED_BY_DEFAULT);
    }
    if (args->outputOptionsError)
    {
        InfoUnderscored(MAKEPRI_STRING_OUTPUT_OPTIONS_ARE_COMMA_SEPARATED_LIST);
        WriteWorkOutput(MAKEPRI_STRING_PRIHEADER_PH_WRITE_OMIT_PRIHEADER_ELEMENT);
        WriteWorkOutput(MAKEPRI_STRING_INDEXES_IX_WRITE_OMIT_INDEX_ATTRIBUTE);
        WriteWorkOutput(MAKEPRI_STRING_URIS_URI_WRITE_OMIT_URI_ATTRIBUTE);
        WriteWorkOutput(MAKEPRI_STRING_QUALIFIERINFO_QI_WRITE_OMIT_GLOBAL_QUALIFIERINFO);
        WriteWorkOutput(MAKEPRI_STRING_RESOURCEMAPS_RM_WRITE_OMIT_RESOURCEMAP_ELEMENT);
        WriteWorkOutput(MAKEPRI_STRING_VERSIONINFO_VI_WRITE_OMIT_VERSIONINFO_ELEMENT);
        WriteWorkOutput(MAKEPRI_STRING_EMPTYSUBTREES_ES_WRITE_OMIT_RESOURCEMAPSUBTREE_ELEMENTS);
        WriteWorkOutput(MAKEPRI_STRING_EMPTYNAMEDRESOURCES_ENR_WRITE_OMIT_NAMEDRESOURCE_ELEMENTS);
        WriteWorkOutput(MAKEPRI_STRING_LINKS_L_WRITE_OMIT_LINK_ELEMENTS);
        WriteWorkOutput(MAKEPRI_STRING_NAMEDRESEOURCEDECISION_NRD_WRITE_OMIT_DECISION_ELEMENT);
        WriteWorkOutput(MAKEPRI_STRING_CANDIDATES_C_WRITE_OMIT_CANDIDATE_ELEMENT);
        WriteWorkOutput(MAKEPRI_STRING_LINKEDCANDIDATES_LC_WRITE_OMIT_DEFAULT_CANDIDATE);
        WriteWorkOutput(MAKEPRI_STRING_SANITIZEXML_SXML_SANITIZE_PRESERVE_DEFAULT_CERTAIN);
        WriteWorkOutput(MAKEPRI_STRING_OUTPUTOPTIONS_ARE_ENABLED_BY_DEFAULT_UNLESS);
    }
}
} // namespace Microsoft::Resources::Tools::MakePri

namespace Microsoft::Resources::Indexers
{
LogItem::LogItem(const MrmResourceIndexerMessageSeverity severity, const DWORD messageId, std::wstring message) :
    severity(severity), messageId(messageId), message(std::move(message))
{}
} // namespace Microsoft::Resources::Indexers
