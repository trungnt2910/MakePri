#include <algorithm>
#include <cstddef>
#include <cstdarg>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

#include <uni_algo/conv.h>

#include "internal.h"

namespace
{
enum class PrintfLength
{
    Normal,
    Char,
    Short,
    Long,
    LongLong,
    Size,
    LongDouble,
};

std::string NarrowPrintfFormat(
    const std::u16string_view input,
    const std::size_t specificationStart,
    const std::size_t lengthStart,
    const PrintfLength length,
    const char conversion)
{
    std::string result("%");
    for (std::size_t index = specificationStart; index < lengthStart; ++index)
    {
        if (input[index] <= 0x7f)
        {
            result.push_back(static_cast<char>(input[index]));
        }
    }
    switch (length)
    {
    case PrintfLength::Char:
        result.append("hh");
        break;
    case PrintfLength::Short:
        result.push_back('h');
        break;
    case PrintfLength::Long:
        result.push_back('l');
        break;
    case PrintfLength::LongLong:
        result.append("ll");
        break;
    case PrintfLength::Size:
        result.push_back('z');
        break;
    case PrintfLength::LongDouble:
        result.push_back('L');
        break;
    case PrintfLength::Normal:
        break;
    }
    result.push_back(conversion);
    return result;
}

template<typename Value>
void AppendPrintf(
    std::u16string& output,
    const std::string& format,
    const std::optional<int> width,
    const std::optional<int> precision,
    const Value value)
{
    const auto print = [&](char* const buffer, const std::size_t size) {
        if (width && precision)
            return std::snprintf(buffer, size, format.c_str(), *width, *precision, value);
        if (width)
            return std::snprintf(buffer, size, format.c_str(), *width, value);
        if (precision)
            return std::snprintf(buffer, size, format.c_str(), *precision, value);
        return std::snprintf(buffer, size, format.c_str(), value);
    };
    const int length = print(nullptr, 0);
    if (length < 0)
    {
        return;
    }
    std::string buffer(static_cast<std::size_t>(length) + 1, '\0');
    print(buffer.data(), buffer.size());
    output.append(una::utf8to16<char, char16_t>(std::string_view(buffer.data(), static_cast<std::size_t>(length))));
}

std::u16string Format(const char16_t* const format, va_list sourceArguments)
{
    va_list arguments;
    va_copy(arguments, sourceArguments);
    std::u16string output;
    const std::u16string_view input = wchar_compat::View(format);
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        if (input[index] != u'%')
        {
            output.push_back(input[index]);
            continue;
        }
        if (++index >= input.size())
        {
            break;
        }
        if (input[index] == u'%')
        {
            output.push_back(u'%');
            continue;
        }
        const std::size_t specificationStart = index;

        while (index < input.size() &&
               (input[index] == u'-' || input[index] == u'+' || input[index] == u' ' || input[index] == u'#' || input[index] == u'0'))
        {
            ++index;
        }
        std::optional<int> width;
        if (index < input.size() && input[index] == u'*')
        {
            width = va_arg(arguments, int);
            ++index;
        }
        else
        {
            while (index < input.size() && input[index] >= u'0' && input[index] <= u'9')
                ++index;
        }
        std::optional<int> precision;
        if (index < input.size() && input[index] == u'.')
        {
            ++index;
            if (index < input.size() && input[index] == u'*')
            {
                precision = va_arg(arguments, int);
                ++index;
            }
            else
            {
                while (index < input.size() && input[index] >= u'0' && input[index] <= u'9')
                    ++index;
            }
        }
        const std::size_t lengthStart = index;
        PrintfLength length = PrintfLength::Normal;
        if (index + 1 < input.size() && input[index] == u'h' && input[index + 1] == u'h')
        {
            length = PrintfLength::Char;
            index += 2;
        }
        else if (index < input.size() && input[index] == u'h')
        {
            length = PrintfLength::Short;
            ++index;
        }
        if (index < input.size() && input[index] == u'l')
        {
            length = PrintfLength::Long;
            if (++index < input.size() && input[index] == u'l')
            {
                length = PrintfLength::LongLong;
                ++index;
            }
        }
        else if (index + 2 < input.size() && input[index] == u'I' && input[index + 1] == u'6' && input[index + 2] == u'4')
        {
            length = PrintfLength::LongLong;
            index += 3;
        }
        else if (index + 2 < input.size() && input[index] == u'I' && input[index + 1] == u'3' && input[index + 2] == u'2')
        {
            index += 3;
        }
        else if (index < input.size() && input[index] == u'z')
        {
            length = PrintfLength::Size;
            ++index;
        }
        else if (index < input.size() && input[index] == u'L')
        {
            length = PrintfLength::LongDouble;
            ++index;
        }
        if (index >= input.size())
        {
            break;
        }
        const char16_t conversion = input[index];
        if (conversion == u's')
        {
            const auto* const value = va_arg(arguments, const char16_t*);
            const std::string text = value == nullptr ? std::string("(null)") : una::utf16to8<char16_t, char>(wchar_compat::View(value));
            AppendPrintf(
                output,
                NarrowPrintfFormat(input, specificationStart, lengthStart, PrintfLength::Normal, 's'),
                width,
                precision,
                text.c_str());
        }
        else if (conversion == u'S')
        {
            const char* const value = va_arg(arguments, const char*);
            AppendPrintf(
                output,
                NarrowPrintfFormat(input, specificationStart, lengthStart, PrintfLength::Normal, 's'),
                width,
                precision,
                value == nullptr ? "(null)" : value);
        }
        else if (conversion == u'c' || conversion == u'C')
        {
            const char16_t character = static_cast<char16_t>(va_arg(arguments, int));
            const std::string text = una::utf16to8<char16_t, char>(std::u16string_view(&character, 1));
            AppendPrintf(
                output,
                NarrowPrintfFormat(input, specificationStart, lengthStart, PrintfLength::Normal, 's'),
                width,
                precision,
                text.c_str());
        }
        else if (conversion == u'd' || conversion == u'i')
        {
            const std::string narrow = NarrowPrintfFormat(input, specificationStart, lengthStart, length, static_cast<char>(conversion));
            if (length == PrintfLength::LongLong)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, long long));
            else if (length == PrintfLength::Long)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, long));
            else if (length == PrintfLength::Size)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, std::ptrdiff_t));
            else
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, int));
        }
        else if (conversion == u'u' || conversion == u'x' || conversion == u'X' || conversion == u'o')
        {
            const std::string narrow = NarrowPrintfFormat(input, specificationStart, lengthStart, length, static_cast<char>(conversion));
            if (length == PrintfLength::LongLong)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, unsigned long long));
            else if (length == PrintfLength::Long)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, unsigned long));
            else if (length == PrintfLength::Size)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, std::size_t));
            else
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, unsigned int));
        }
        else if (conversion == u'p')
        {
            AppendPrintf(
                output,
                NarrowPrintfFormat(input, specificationStart, lengthStart, PrintfLength::Normal, 'p'),
                width,
                precision,
                va_arg(arguments, void*));
        }
        else if (
            conversion == u'f' || conversion == u'F' || conversion == u'g' || conversion == u'G' || conversion == u'e' ||
            conversion == u'E' || conversion == u'a' || conversion == u'A')
        {
            const std::string narrow = NarrowPrintfFormat(input, specificationStart, lengthStart, length, static_cast<char>(conversion));
            if (length == PrintfLength::LongDouble)
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, long double));
            else
                AppendPrintf(output, narrow, width, precision, va_arg(arguments, double));
        }
        else
        {
            output.push_back(u'%');
            output.push_back(conversion);
        }
    }
    va_end(arguments);
    return output;
}
} // namespace

extern "C" int __wrap_vswprintf(char16_t* const buffer, const std::size_t size, const char16_t* const format, va_list arguments)
{
    return wchar_compat::CopyResult(buffer, size, Format(format, arguments));
}

extern "C" int __wrap_fwprintf(std::FILE* const stream, const char16_t* const format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    const std::u16string output = Format(format, arguments);
    va_end(arguments);
    return wchar_compat::WriteWide(stream, output);
}

extern "C" int __wrap_wprintf(const char16_t* const format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    const std::u16string output = Format(format, arguments);
    va_end(arguments);
    return wchar_compat::WriteWide(stdout, output);
}
