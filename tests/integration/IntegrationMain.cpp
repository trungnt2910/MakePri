#include "StdAfx.h"

#include "IntegrationTest.h"

namespace MakePri::Tests
{

IntegrationConfiguration g_integrationConfiguration;

namespace
{

bool ConsumeValueArgument(
    int* const argumentCount,
    char** arguments,
    const int index,
    const std::string_view name,
    std::filesystem::path* const value)
{
    if (arguments[index] != name)
    {
        return false;
    }
    if ((index + 1) >= *argumentCount)
    {
        return false;
    }

    *value = arguments[index + 1];
    std::move(arguments + index + 2, arguments + *argumentCount, arguments + index);
    *argumentCount -= 2;
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
            ConsumeValueArgument(argumentCount, arguments, index, "--makepri-output-root", &g_integrationConfiguration.outputRoot))
        {
            continue;
        }
        if (std::string_view(arguments[index]) == "--makepri-update-outputs")
        {
            std::move(arguments + index + 1, arguments + *argumentCount, arguments + index);
            --*argumentCount;
            g_integrationConfiguration.updateOutputs = true;
            continue;
        }
        ++index;
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
