#include <cerrno>
#include <cstdio>
#include <cwchar>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <uni_algo/conv.h>

#include "internal.h"

namespace
{
enum class WideInputEncoding
{
    Undetermined,
    Utf8,
    Utf16LittleEndian,
};

struct WideInputState
{
    WideInputEncoding encoding = WideInputEncoding::Undetermined;
    std::deque<unsigned char> pendingBytes;
    std::deque<char16_t> pendingCodeUnits;
};

struct WideOutputState
{
    std::optional<char16_t> pendingHighSurrogate;
};

std::unordered_map<std::FILE*, WideInputState> WideInputStates;
std::unordered_map<std::FILE*, WideOutputState> WideOutputStates;

int ReadInputByte(std::FILE* const stream, WideInputState& state)
{
    if (!state.pendingBytes.empty())
    {
        const int result = state.pendingBytes.front();
        state.pendingBytes.pop_front();
        return result;
    }
    return std::fgetc(stream);
}

bool DetermineInputEncoding(std::FILE* const stream, WideInputState& state)
{
    const int first = std::fgetc(stream);
    if (first == EOF)
    {
        return false;
    }
    if (first == 0xff)
    {
        const int second = std::fgetc(stream);
        if (second == 0xfe)
        {
            state.encoding = WideInputEncoding::Utf16LittleEndian;
            return true;
        }
        state.pendingBytes.push_back(static_cast<unsigned char>(first));
        if (second != EOF)
            state.pendingBytes.push_back(static_cast<unsigned char>(second));
    }
    else if (first == 0xef)
    {
        const int second = std::fgetc(stream);
        const int third = second == EOF ? EOF : std::fgetc(stream);
        if (second != 0xbb || third != 0xbf)
        {
            state.pendingBytes.push_back(static_cast<unsigned char>(first));
            if (second != EOF)
                state.pendingBytes.push_back(static_cast<unsigned char>(second));
            if (third != EOF)
                state.pendingBytes.push_back(static_cast<unsigned char>(third));
        }
    }
    else
    {
        state.pendingBytes.push_back(static_cast<unsigned char>(first));
    }
    state.encoding = WideInputEncoding::Utf8;
    return true;
}

std::size_t Utf8SequenceLength(const unsigned char first)
{
    if (first <= 0x7f)
        return 1;
    if (first >= 0xc2 && first <= 0xdf)
        return 2;
    if (first >= 0xe0 && first <= 0xef)
        return 3;
    if (first >= 0xf0 && first <= 0xf4)
        return 4;
    return 1;
}

bool FillWideInput(std::FILE* const stream, WideInputState& state)
{
    if (state.encoding == WideInputEncoding::Undetermined && !DetermineInputEncoding(stream, state))
    {
        return false;
    }

    if (state.encoding == WideInputEncoding::Utf16LittleEndian)
    {
        const int low = ReadInputByte(stream, state);
        if (low == EOF)
            return false;
        const int high = ReadInputByte(stream, state);
        if (high == EOF)
        {
            state.pendingCodeUnits.push_back(u'\ufffd');
        }
        else
        {
            state.pendingCodeUnits.push_back(static_cast<char16_t>(static_cast<unsigned>(low) | (static_cast<unsigned>(high) << 8)));
        }
        return true;
    }

    const int first = ReadInputByte(stream, state);
    if (first == EOF)
        return false;
    std::string bytes(1, static_cast<char>(static_cast<unsigned char>(first)));
    const std::size_t sequenceLength = Utf8SequenceLength(static_cast<unsigned char>(first));
    for (std::size_t index = 1; index < sequenceLength; ++index)
    {
        const int next = ReadInputByte(stream, state);
        if (next == EOF)
            break;
        if ((next & 0xc0) != 0x80)
        {
            state.pendingBytes.push_front(static_cast<unsigned char>(next));
            break;
        }
        bytes.push_back(static_cast<char>(static_cast<unsigned char>(next)));
    }
    const std::u16string converted = una::utf8to16<char, char16_t>(bytes);
    state.pendingCodeUnits.insert(state.pendingCodeUnits.end(), converted.begin(), converted.end());
    return !state.pendingCodeUnits.empty();
}

bool IsHighSurrogate(const char16_t codeUnit) { return codeUnit >= 0xd800 && codeUnit <= 0xdbff; }

std::u16string BufferWideOutput(WideOutputState& state, const std::u16string_view value, const bool translateNewlines)
{
    std::u16string buffered;
    buffered.reserve(value.size() + (state.pendingHighSurrogate ? 1 : 0));
    if (state.pendingHighSurrogate)
    {
        buffered.push_back(*state.pendingHighSurrogate);
        state.pendingHighSurrogate.reset();
    }
    buffered.append(value);
    if (!buffered.empty() && IsHighSurrogate(buffered.back()))
    {
        state.pendingHighSurrogate = buffered.back();
        buffered.pop_back();
    }

    std::u16string translated;
    translated.reserve(buffered.size());
    for (const char16_t codeUnit : buffered)
    {
        if (codeUnit == u'\n' && translateNewlines)
        {
            translated.push_back(u'\r');
        }
        translated.push_back(codeUnit);
    }
    return translated;
}
} // namespace

int wchar_compat::WriteWide(std::FILE* const stream, const std::u16string_view value)
{
    if (stream == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    const bool translateNewlines = stream == stdout || stream == stderr;
    const std::u16string buffered = BufferWideOutput(WideOutputStates[stream], value, translateNewlines);
    const std::string output = una::utf16to8<char16_t, char>(buffered);
    const std::size_t written = std::fwrite(output.data(), 1, output.size(), stream);
    return written == output.size() ? static_cast<int>(value.size()) : -1;
}

extern "C" int __real_fclose(std::FILE* stream);

extern "C" int __wrap_fclose(std::FILE* const stream)
{
    if (stream == nullptr)
    {
        errno = EINVAL;
        return EOF;
    }
    WideInputStates.erase(stream);
    const auto outputState = WideOutputStates.find(stream);
    if (outputState != WideOutputStates.end() && outputState->second.pendingHighSurrogate)
    {
        outputState->second.pendingHighSurrogate.reset();
        wchar_compat::WriteWide(stream, u"\ufffd");
    }
    WideOutputStates.erase(stream);
    return __real_fclose(stream);
}

extern "C" wint_t __wrap_fgetwc(std::FILE* const stream)
{
    auto iterator = WideInputStates.find(stream);
    if (iterator == WideInputStates.end())
    {
        iterator = WideInputStates.emplace(stream, WideInputState {}).first;
    }
    WideInputState& state = iterator->second;
    if (state.pendingCodeUnits.empty() && !FillWideInput(stream, state))
        return WEOF;
    const wint_t result = static_cast<wint_t>(state.pendingCodeUnits.front());
    state.pendingCodeUnits.pop_front();
    return result;
}
