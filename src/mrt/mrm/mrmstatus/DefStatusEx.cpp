#include "StdAfx.h"

#include <DefStatus.h>

namespace Microsoft::Resources
{
void DefStatusEx::Init()
{
    _status.what = S_OK;
    _status.where = nullptr;
    _status.desc = nullptr;
    _status.line = 0;
    _status.details = 0;
}

void DefStatusEx::Reset() { SetError(S_OK, L"", 0, L"", 0); }

void DefStatusEx::ResetErrorLocation() { m_errorLocation = nullptr; }

void DefStatusEx::SetErrorLocation(const wchar_t* const location)
{
    if (location != nullptr)
    {
        m_errorLocation = _AddStringToStore(location);
    }
}

bool DefStatusEx::SetError(
    const HRESULT error,
    const wchar_t* const where,
    const std::uint32_t line,
    const wchar_t* const description,
    const std::uint32_t descriptionLength)
{
    _status.what = error;
    _status.where = where != nullptr ? _AddStringToStore(where) : m_errorLocation;
    _status.line = line;
    _status.desc = description != nullptr ? _AddStringToStore(description) : nullptr;
    _status.details = descriptionLength;
    return Failed();
}

bool DefStatusEx::SetError(const HRESULT error, const wchar_t* const where, const std::uint32_t line, const wchar_t* const description)
{
    return SetError(error, where, line, description, 0);
}

bool DefStatusEx::SetError(const HRESULT error, const wchar_t* const description) { return SetError(error, nullptr, 0, description, 0); }

bool DefStatusEx::AddWarning(
    const HRESULT warning,
    const wchar_t* const where,
    const std::uint32_t line,
    const wchar_t* const description,
    const std::uint32_t descriptionLength)
{
    const wchar_t* const storedWhere = where != nullptr ? _AddStringToStore(where) : nullptr;
    const wchar_t* const storedDescription = description != nullptr ? _AddStringToStore(description) : nullptr;

    auto status = std::make_unique<DefStatus>();
    if (!status->SetError(warning, storedWhere, line, storedDescription, descriptionLength))
    {
        return false;
    }
    m_warningView.push_back(status.get());
    m_warnings.push_back(std::move(status));
    return true;
}

bool DefStatusEx::AddWarning(const HRESULT warning, const wchar_t* const where, const std::uint32_t line, const wchar_t* const description)
{
    return AddWarning(warning, where, line, description, 0);
}

bool DefStatusEx::AddWarning(const HRESULT warning, const wchar_t* const description)
{
    return AddWarning(warning, nullptr, 0, description, 0);
}

bool DefStatusEx::TryAddSpecificErrorCode(const HRESULT error)
{
    const HRESULT mapped = GetHResult();
    if (Succeeded() || mapped == E_FAIL || mapped == E_INVALIDARG)
    {
        _status.what = error;
        return true;
    }
    return false;
}

HRESULT DefStatusEx::GetHResult()
{
    if (Succeeded())
    {
        return S_OK;
    }

    switch (_status.what)
    {
    case E_DEF_BAD_VALUE:
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    case E_DEF_OUT_OF_RANGE:
    case E_DEF_RANGE_NOT_FOUND:
    case E_DEF_COUNT_BEYOND_MAX_RANGE:
        return HRESULT_FROM_WIN32(ERROR_RANGE_NOT_FOUND);
    case E_DEF_INVALID_ARG:
        return E_INVALIDARG;
    case E_DEF_INVALID_OBJECT:
    case E_DEF_NOT_INITIALIZED:
        return HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);
    case E_DEF_OUT_OF_MEMORY:
        return E_OUTOFMEMORY;
    case E_DEF_NOT_WRITABLE:
    case E_DEF_ATOM_POOL_MISMATCH:
    case E_DEF_COLLECTION_OWNER_MISMATCH:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_FILE_TYPE);
    case E_DEF_INSUFFICIENT_BUFFER:
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    case E_DEF_BAD_PATH:
        return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
    case E_DEF_NOT_IMPLEMENTED:
        return E_NOTIMPL;
    case E_DEF_INTERNAL_ERROR:
        return E_ABORT;
    case E_DEF_ATOM_BAD_INDEX:
        return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    case E_DEF_ATOM_NAME_NOT_FOUND:
    case E_DEF_KEY_NOT_FOUND:
    case E_DEF_VALUE_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    case E_DEF_ENTRY_ALREADY_EXISTS:
    case E_DEF_UNSUPPORTED_VERSION:
        return HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY);
    case E_DEF_SCHEMA_MISMATCH:
        return HRESULT_FROM_WIN32(ERROR_MRM_MAP_NOT_FOUND);
    case E_DEF_MAJOR_VERSION_MISMATCH:
    case E_DEF_MINOR_VERSION_PRIOR:
    case E_DEF_MINOR_VERSION_LATER:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_VERSION_MISMATCH);
    case E_DEF_DUPLICATE_SCHEMA_NAME:
        return HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_MAP_NAME);
    case E_DEF_INVALID_QUALIFIER_NAME:
        return HRESULT_FROM_WIN32(ERROR_MRM_UNKNOWN_QUALIFIER);
    case E_DEF_INVALID_CONDITION_OPERATOR:
    case E_DEF_CONDITION_ILLEGAL_OPERATOR:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_OPERATOR);
    case E_DEF_INVALID_ATTRIBUTE_VALUE:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
    case E_DEF_FILE_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    case E_DEF_INVALID_PATH:
        return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    case E_DEF_INVALID_HANDLE:
        return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    case E_DEF_ACCESS_DENIED:
        return E_ACCESSDENIED;
    case E_DEF_ITEM_NAME_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_MRM_NAMED_RESOURCE_NOT_FOUND);
    case E_DEF_TYPE_MISMATCH:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_FILE_TYPE);
    case E_DEFFILE_BAD_MAGIC_NUMBER:
    case E_DEFFILE_BAD_TOC_ENTRY:
    case E_DEFFILE_TOC_MISMATCH:
    case E_DEFFILE_BAD_SECTION_TRAILER:
    case E_DEFFILE_BAD_FILE_TRAILER:
    case E_DEFFILE_FILE_DATA_TOO_SMALL:
    case E_DEFFILE_BAD_TOC_OFFSET:
    case E_DEFFILE_BAD_DATA_OFFSET:
    case E_DEFFILE_BAD_SECTION_HEADER:
    case E_DEFFILE_BAD_SECTION_TYPE:
    case E_DEFFILE_BAD_SECTION_QUALIFIER:
    case E_DEFFILE_SECTION_DATA_TOO_SMALL:
    case E_DEFFILE_BAD_ALIGNMENT:
    case E_DEFFILE_NO_SECTIONS:
    case E_DEFFILE_NO_ATOMS:
    case E_DEFFILE_ATOM_COUNT_MISMATCH:
    case E_DEFFILE_DUPLICATE_SECTION:
    case E_DEFFILE_SECTION_NOT_FOUND:
    case E_DEFFILE_ATOMPOOL_MISMATCH:
    case E_DEFFILE_BAD_SECTION_INDEX:
    case E_DEFFILE_FORMAT_ERROR:
    case E_DEFFILE_SECTION_DATA_TOO_LARGE:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
    case E_DEFFILE_BUILD_BAD_PHASE:
        return HRESULT_FROM_WIN32(ERROR_INVALID_OPERATION);
    case E_DEFFILE_BUILD_DEPLOYMENT_MERGE_UNSUPPORTED_FILE_TYPE:
        return HRESULT_FROM_WIN32(ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE);
    case E_DEF_QUALAPPL_INVALID_QUALIFIER:
        return HRESULT_FROM_WIN32(ERROR_MRM_UNKNOWN_QUALIFIER);
    case E_DEF_IBC_SCOPE_ITEM_CONFLICT:
    case E_DEF_IBC_CONFLICTING_VALUES:
        return HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY);
    case E_MRM_COLLECTION_NOT_FOUND:
    case E_MRM_NO_PRIMARY_INDEX:
    case E_MRM_INVALID_MAP_NAME:
    case E_MRM_MULTIPLE_MAPS:
        return HRESULT_FROM_WIN32(ERROR_MRM_MAP_NOT_FOUND);
    case E_MRM_ITEM_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_MRM_NAMED_RESOURCE_NOT_FOUND);
    case E_MRM_BAD_INSTANCE_TYPE:
        return HRESULT_FROM_WIN32(ERROR_MRM_RESOURCE_TYPE_MISMATCH);
    case E_MRM_READ_ONLY_SCHEMA:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_FILE_TYPE);
    case E_MRM_NO_MATCHING_CANDIDATE:
        return HRESULT_FROM_WIN32(ERROR_MRM_NO_MATCH_OR_DEFAULT_CANDIDATE);
    case E_MRM_PACKAGE_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_MRM_PACKAGE_NOT_FOUND);
    case E_MRM_UNSUPPORTED_PROFILE_TYPE:
        return HRESULT_FROM_WIN32(ERROR_MRM_UNSUPPORTED_PROFILE_TYPE);
    case E_MRM_API_PERMISSION_DENIED:
        return E_ACCESSDENIED;
    case E_MRM_UNSUPPORTED_PLATFORM:
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
    case E_MRM_TOO_MANY_RESOURCES:
        return HRESULT_FROM_WIN32(ERROR_MRM_TOO_MANY_RESOURCES);
    case E_MRM_NO_DEFAULT_OR_NEUTRAL_VALUE:
        return HRESULT_FROM_WIN32(ERROR_MRM_RUNTIME_NO_DEFAULT_OR_NEUTRAL_RESOURCE);
    case E_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILES:
    case E_MRM_SCHEMALESS_PRI_LOAD_FAILED:
        return HRESULT_FROM_WIN32(ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE);
    case E_MRM_MERGE_MISSING_SCHEMA:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_MISSING_SCHEMA);
    case E_MRM_MERGE_LOAD_FILE_FAILED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_LOAD_FILE_FAILED);
    case E_MRM_MERGE_ADD_FILE_FAILED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_ADD_FILE_FAILED);
    case E_MRM_MERGE_WRITE_FILE_FAILED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_WRITE_FILE_FAILED);
    case E_MRM_MULTIPLE_PACKAGE_FAMILIES_NOT_ALLOWED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_MULTIPLE_PACKAGE_FAMILIES_NOT_ALLOWED);
    case E_MRM_MULTIPLE_MAIN_PACKAGES_NOT_ALLOWED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_MULTIPLE_MAIN_PACKAGES_NOT_ALLOWED);
    case E_MRM_BUNDLE_PACKAGES_NOT_ALLOWED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_BUNDLE_PACKAGES_NOT_ALLOWED);
    case E_MRM_MAIN_PACKAGE_REQUIRED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_MAIN_PACKAGE_REQUIRED);
    case E_MRM_RESOURCE_PACKAGE_REQUIRED:
        return HRESULT_FROM_WIN32(ERROR_PRI_MERGE_RESOURCE_PACKAGE_REQUIRED);
    default:
        if (_status.what >= E_DEFFILE_BAD_SECTION_QUALIFIER && _status.what <= E_DEFFILE_ATOMPOOL_MISMATCH)
        {
            return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
        }
        return _status.what;
    }
}

void DefStatusEx::SetDiagnosticLoggingEnabled(const bool enabled)
{
    if (enabled)
    {
        m_diagnosticFlags |= 1;
    }
    else
    {
        m_diagnosticFlags &= ~1U;
    }
}

bool DefStatusEx::DiagnosticLogA(const char* const format, ...)
{
    if ((m_diagnosticFlags & 1) == 0)
    {
        return true;
    }

    char message[1024] {};
    va_list arguments;
    va_start(arguments, format);
    const HRESULT hr = StringCchVPrintfA(message, 1024, format, arguments);
    va_end(arguments);
    if (FAILED(hr))
    {
        return false;
    }
    std::printf("[Makepri.exe] ");
    std::printf("%s", message);
    std::printf("\n");
    return true;
}

bool DefStatusEx::DiagnosticLogWithPrefixA(const char* const prefix, const char* const format, ...)
{
    if ((m_diagnosticFlags & 1) == 0)
    {
        return true;
    }

    char message[1024] {};
    va_list arguments;
    va_start(arguments, format);
    const HRESULT hr = StringCchVPrintfA(message, 1024, format, arguments);
    va_end(arguments);
    if (FAILED(hr))
    {
        return false;
    }
    std::printf("[Makepri.exe] ");
    std::printf("%s", prefix);
    std::printf("%s", message);
    std::printf("\n");
    return true;
}

bool DefStatusEx::DiagnosticLogWithErrorCodeA(const char* const message, const HRESULT error)
{
    if ((m_diagnosticFlags & 1) == 0)
    {
        return true;
    }
    if (SUCCEEDED(error))
    {
        return DiagnosticLogWithPrefixA("Succeeded - ", message);
    }

    char prefix[1024] {};
    if (FAILED(StringCchPrintfA(prefix, 1024, "Failed - %s", message)))
    {
        return false;
    }
    return DiagnosticLogWithPrefixA(prefix, " - 0x%8x", error);
}

const wchar_t* DefStatusEx::_AddStringToStore(const wchar_t* const value)
{
    if (value == nullptr || value[0] == L'\0')
    {
        return nullptr;
    }
    m_stringStore.emplace_back(value);
    return m_stringStore.back().c_str();
}
} // namespace Microsoft::Resources
