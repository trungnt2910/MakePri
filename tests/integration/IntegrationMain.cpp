#include "StdAfx.h"

#include <charconv>
#include <stdexcept>
#include <type_traits>

#include "IntegrationTest.h"

namespace MakePri::Tests
{

IntegrationConfiguration g_integrationConfiguration;

namespace
{

template<typename Value>
bool ConsumeValueArgument(int* const argumentCount, char** arguments, const int index, const std::string_view name, Value* const value)
{
    if (arguments[index] != name)
    {
        return false;
    }

    int consumedArgumentCount = 1;
    if constexpr (std::is_same_v<Value, bool>)
    {
        *value = true;
    }
    else
    {
        if ((index + 1) >= *argumentCount)
        {
            return false;
        }

        const std::string text(arguments[index + 1]);
        if constexpr (std::is_same_v<Value, std::filesystem::path>)
        {
            *value = std::filesystem::path(text);
        }
        else if constexpr (std::is_same_v<Value, int>)
        {
            const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), *value);
            if (error != std::errc() || end != text.data() + text.size())
            {
                return false;
            }
        }
        else
        {
            static_assert(std::is_same_v<Value, void>, "Unsupported integration argument type");
        }
        consumedArgumentCount = 2;
    }

    std::move(arguments + index + consumedArgumentCount, arguments + *argumentCount, arguments + index);
    *argumentCount -= consumedArgumentCount;
    return true;
}

void ConsumeIntegrationArguments(int* const argumentCount, char** arguments)
{
    int index = 1;
    while (index < *argumentCount)
    {
        if (ConsumeValueArgument(argumentCount, arguments, index, "--makepri-under-test", &g_integrationConfiguration.makePriUnderTest) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-official", &g_integrationConfiguration.officialMakePri) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-input-root", &g_integrationConfiguration.inputRoot) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-output-root", &g_integrationConfiguration.outputRoot) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-exit-code-bits", &g_integrationConfiguration.exitCodeBits) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-update-outputs", &g_integrationConfiguration.updateOutputs) ||
            ConsumeValueArgument(
                argumentCount, arguments, index, "--makepri-forward-slash-compat", &g_integrationConfiguration.forwardSlashCompatibility) ||
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-utf8", &g_integrationConfiguration.utf8Console))
        {
            continue;
        }
        ++index;
    }

    if (g_integrationConfiguration.exitCodeBits < 1 || g_integrationConfiguration.exitCodeBits > 32)
    {
        throw std::invalid_argument("--makepri-exit-code-bits must be between 1 and 32");
    }

    if (!g_integrationConfiguration.makePriUnderTest.empty())
    {
        g_integrationConfiguration.makePriUnderTest = std::filesystem::absolute(g_integrationConfiguration.makePriUnderTest);
    }
    if (!g_integrationConfiguration.officialMakePri.empty())
    {
        g_integrationConfiguration.officialMakePri = std::filesystem::absolute(g_integrationConfiguration.officialMakePri);
    }
    if (!g_integrationConfiguration.inputRoot.empty())
    {
        g_integrationConfiguration.inputRoot = std::filesystem::absolute(g_integrationConfiguration.inputRoot);
    }
    if (!g_integrationConfiguration.outputRoot.empty())
    {
        g_integrationConfiguration.outputRoot = std::filesystem::absolute(g_integrationConfiguration.outputRoot);
    }
}

} // namespace

} // namespace MakePri::Tests

int main(int argumentCount, char** arguments)
{
    MakePri::Tests::ConsumeIntegrationArguments(&argumentCount, arguments);
    testing::InitGoogleTest(&argumentCount, arguments);
    return RUN_ALL_TESTS();
}
