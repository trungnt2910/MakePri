#include <string>

#include <winbase.h>

#include "internal/strings.h"

namespace
{
bool AppendInsert(std::u16string& message, const std::u16string_view format, std::size_t& index, const ULONG_PTR* const argumentArray)
{
    std::size_t numberEnd = index + 1;
    std::size_t argumentNumber = 0;
    while (numberEnd < format.size() && format[numberEnd] >= u'0' && format[numberEnd] <= u'9')
    {
        argumentNumber = argumentNumber * 10 + static_cast<std::size_t>(format[numberEnd] - u'0');
        ++numberEnd;
    }
    if (argumentArray == nullptr || argumentNumber == 0 || numberEnd >= format.size() || format[numberEnd] != u'!')
    {
        return false;
    }
    const std::size_t conversionEnd = format.find(u'!', numberEnd + 1);
    if (conversionEnd == std::u16string_view::npos)
        return false;
    if (format.substr(numberEnd + 1, conversionEnd - numberEnd - 1) == u"s")
    {
        message.append(win32_compat::WideView(reinterpret_cast<LPCWSTR>(argumentArray[argumentNumber - 1])));
    }
    index = conversionEnd;
    return true;
}

std::u16string FormatFromString(const DWORD flags, LPCVOID const source, va_list* const arguments)
{
    const std::u16string_view format = win32_compat::WideView(static_cast<LPCWSTR>(source));
    const auto* const argumentArray =
        (flags & FORMAT_MESSAGE_ARGUMENT_ARRAY) != 0 ? reinterpret_cast<const ULONG_PTR*>(arguments) : nullptr;
    std::u16string message;
    for (std::size_t index = 0; index < format.size(); ++index)
    {
        if (format[index] != u'%' || index + 1 >= format.size())
        {
            message.push_back(format[index]);
        }
        else if (format[index + 1] == u'%')
        {
            message.push_back(u'%');
            ++index;
        }
        else if (format[index + 1] == u'0')
        {
            break;
        }
        else if (format[index + 1] == u'n')
        {
            message.append(u"\r\n");
            ++index;
        }
        else if (!AppendInsert(message, format, index, argumentArray))
        {
            message.push_back(format[index]);
        }
    }
    return message;
}

std::u16string SystemMessage(const DWORD messageId)
{
    switch (messageId)
    {
    case ERROR_FILE_NOT_FOUND:
        return u"The system cannot find the file specified.\r\n";
    case ERROR_PATH_NOT_FOUND:
        return u"The system cannot find the path specified.\r\n";
    case ERROR_ACCESS_DENIED:
        return u"Access is denied.\r\n";
    case ERROR_INVALID_DATA:
        return u"The data is invalid.\r\n";
    case ERROR_BAD_FORMAT:
        return u"An attempt was made to load a program with an incorrect format.\r\n";
    case ERROR_INVALID_PARAMETER:
        return u"The parameter is incorrect.\r\n";
    case ERROR_MRM_INVALID_PRI_FILE:
        return u"Invalid PRI File.\r\n";
    case static_cast<DWORD>(E_FAIL):
        return u"Unspecified error\r\n";
    default:
        return {};
    }
}
} // namespace

extern "C" DWORD WINAPI FormatMessageW(
    const DWORD flags,
    LPCVOID const source,
    const DWORD messageId,
    DWORD,
    LPWSTR const buffer,
    const DWORD size,
    va_list* const arguments)
{
    const std::u16string message = (flags & FORMAT_MESSAGE_FROM_STRING) != 0 && source != nullptr ?
                                       FormatFromString(flags, source, arguments) :
                                       SystemMessage(messageId);
    if (message.empty())
    {
        SetLastError(ERROR_MR_MID_NOT_FOUND);
        return 0;
    }
    if ((flags & FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0)
    {
        auto** const output = reinterpret_cast<LPWSTR*>(buffer);
        *output = static_cast<LPWSTR>(LocalAlloc(LMEM_ZEROINIT, (message.size() + 1) * sizeof(char16_t)));
        if (*output == nullptr)
            return 0;
        win32_compat::CopyWide(message, *output, message.size() + 1);
    }
    else if (!win32_compat::CopyWide(message, buffer, size))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    return static_cast<DWORD>(message.size());
}
