#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <limits>
#include <mutex>
#include <new>
#include <string>
#include <vector>

#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <uni_algo/conv.h>

#include "internal/strings.h"

namespace
{
constexpr int FirstNumberedFileDescriptor = 3;
std::mutex NumberedFilesMutex;
std::vector<std::FILE*> NumberedFiles;
std::array<int, 3> DescriptorModes {};

std::FILE* FileForDescriptor(const int descriptor)
{
    if (descriptor == 0)
        return stdin;
    if (descriptor == 1)
        return stdout;
    if (descriptor == 2)
        return stderr;
    if (descriptor < FirstNumberedFileDescriptor)
        return nullptr;
    const auto index = static_cast<std::size_t>(descriptor - FirstNumberedFileDescriptor);
    return index < NumberedFiles.size() ? NumberedFiles[index] : nullptr;
}
} // namespace

extern "C" std::FILE* _wfopen(const wchar_t* const path, const wchar_t* const mode)
{
    std::string narrowMode = una::utf16to8<char16_t, char>(win32_compat::WideView(mode));
    const bool utf8Text = narrowMode.find("ccs=UTF-8") != std::string::npos;
    const std::size_t comma = narrowMode.find(',');
    if (comma != std::string::npos)
    {
        narrowMode.erase(comma);
    }

    std::u16string normalizedPath(win32_compat::WideView(path));
    std::replace(normalizedPath.begin(), normalizedPath.end(), u'\\', u'/');
    std::FILE* const stream = std::fopen(una::utf16to8<char16_t, char>(normalizedPath).c_str(), narrowMode.c_str());
    if (stream != nullptr && !narrowMode.empty() && narrowMode[0] == 'r' && utf8Text)
    {
        const int first = std::fgetc(stream);
        const int second = std::fgetc(stream);
        std::rewind(stream);
        if (first == 0xfe && second == 0xff)
        {
            std::fclose(stream);
            errno = EINVAL;
            return nullptr;
        }
    }
    else if (stream != nullptr && !narrowMode.empty() && narrowMode[0] == 'w' && utf8Text)
    {
        constexpr std::array<unsigned char, 3> Utf8Bom {0xef, 0xbb, 0xbf};
        std::fwrite(Utf8Bom.data(), 1, Utf8Bom.size(), stream);
    }
    return stream;
}

extern "C" int _wfopen_s(std::FILE** const stream, const wchar_t* const path, const wchar_t* const mode)
{
    if (stream == nullptr)
    {
        return EINVAL;
    }
    *stream = _wfopen(path, mode);
    return *stream == nullptr ? errno : 0;
}

extern "C" int _fileno(std::FILE* const file)
{
    if (file == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    if (file == stdin)
        return 0;
    if (file == stdout)
        return 1;
    if (file == stderr)
        return 2;

    const std::lock_guard lock(NumberedFilesMutex);
    if (NumberedFiles.size() > static_cast<std::size_t>(std::numeric_limits<int>::max() - FirstNumberedFileDescriptor))
    {
        errno = EMFILE;
        return -1;
    }
    try
    {
        NumberedFiles.push_back(file);
    }
    catch (const std::bad_alloc&)
    {
        errno = ENOMEM;
        return -1;
    }
    return FirstNumberedFileDescriptor + static_cast<int>(NumberedFiles.size() - 1);
}

extern "C" long long _filelengthi64(const int descriptor)
{
    const std::lock_guard lock(NumberedFilesMutex);
    std::FILE* const file = FileForDescriptor(descriptor);
    if (file == nullptr)
    {
        errno = EBADF;
        return -1;
    }
    std::fpos_t position;
    if (std::fgetpos(file, &position) != 0 || std::fseek(file, 0, SEEK_END) != 0)
        return -1;
    const long length = std::ftell(file);
    if (std::fsetpos(file, &position) != 0)
        return -1;
    return length;
}

extern "C" int _setmode(const int descriptor, const int mode)
{
    if (descriptor < 0 || descriptor >= static_cast<int>(DescriptorModes.size()))
        return -1;
    const int previous = DescriptorModes[static_cast<std::size_t>(descriptor)];
    DescriptorModes[static_cast<std::size_t>(descriptor)] = mode;
    return previous;
}

extern "C" wint_t _getwche()
{
    const wint_t character = std::fgetwc(stdin);
    if (character != WEOF)
    {
        std::fwprintf(stdout, L"%lc", character);
    }
    return character;
}
