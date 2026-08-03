#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <uni_algo/conv.h>

int wmain(int argumentCount, wchar_t** arguments);

int main(const int argumentCount, char** const arguments)
{
    std::vector<std::u16string> convertedArguments;
    convertedArguments.reserve(static_cast<std::size_t>(argumentCount));

    const char preferredSeparator = std::filesystem::path::preferred_separator;
    for (int index = 0; index < argumentCount; ++index)
    {
        std::u16string converted = una::utf8to16<char, char16_t>(arguments[index]);
        if (!converted.empty() && arguments[index][0] == preferredSeparator)
        {
            for (char16_t& character : converted)
            {
                if (character == static_cast<unsigned char>(preferredSeparator))
                {
                    character = u'\\';
                }
            }
        }
        convertedArguments.push_back(std::move(converted));
    }

    std::vector<char16_t*> argumentPointers;
    argumentPointers.reserve(convertedArguments.size() + 1);
    for (std::u16string& argument : convertedArguments)
    {
        argumentPointers.push_back(argument.data());
    }
    argumentPointers.push_back(nullptr);

    return wmain(argumentCount, reinterpret_cast<wchar_t**>(argumentPointers.data()));
}
