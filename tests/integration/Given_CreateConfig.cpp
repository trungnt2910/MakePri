#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::VerifyIntegrationTest;

class Given_CreateConfig : public IntegrationTest
{};

namespace
{

void Verify(const std::string& name, const std::vector<std::string>& arguments, const std::vector<std::filesystem::path>& outputs = {})
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = name,
            .arguments = arguments,
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = outputs,
        });
}

} // namespace

TEST_F(Given_CreateConfig, When_MinimalConfiguration_MatchesOutput)
{
    Verify("CreateConfig_Minimal", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/pv", "10.0.0", "/o"}, {"priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_LongOptionNamesAreUsed_MatchesOutput)
{
    Verify(
        "CreateConfig_LongNames",
        {"CreateConfig", "/ConfigXML", "priconfig.xml", "/Default", "en-US", "/PlatformVersion", "10.0.0", "/Overwrite"},
        {"priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_PlatformVersionIsOmitted_MatchesOutput)
{
    Verify("CreateConfig_DefaultPlatform", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/o"}, {"priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_VerboseHiddenOptionIsUsed_MatchesOutput)
{
    Verify(
        "CreateConfig_Verbose", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/pv", "10.0.0", "/o", "/v"}, {"priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_OutputOptionsAreUsed_MatchesOutput)
{
    Verify(
        "CreateConfig_OutputOptions",
        {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/oo", "priheader", "/o"},
        {"priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_RequiredArgumentsAreMissing_MatchesOutput) { Verify("CreateConfig_MissingArguments", {"createconfig"}); }

TEST_F(Given_CreateConfig, When_OptionValueIsMissing_MatchesOutput) { Verify("CreateConfig_MissingValue", {"createconfig", "/cf"}); }

TEST_F(Given_CreateConfig, When_DefaultQualifierIsMissing_MatchesOutput)
{
    Verify("CreateConfig_MissingDefault", {"createconfig", "/cf", "priconfig.xml"});
}

TEST_F(Given_CreateConfig, When_PlatformVersionIsInvalid_MatchesOutput)
{
    Verify("CreateConfig_InvalidPlatform", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/pv", "not-a-version"});
}

TEST_F(Given_CreateConfig, When_DocumentedPlatformOptionNameIsUsed_MatchesOutput)
{
    Verify("CreateConfig_DocumentedPlatformName", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/Platform", "10.0", "/o"});
}

TEST_F(Given_CreateConfig, When_PlatformVersionIsDuplicated_MatchesOutput)
{
    Verify("CreateConfig_DuplicatePlatform", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/pv", "10.0", "/pv", "10.0"});
}

TEST_F(Given_CreateConfig, When_ConfigPathIsDuplicated_MatchesOutput)
{
    Verify("CreateConfig_DuplicateConfig", {"createconfig", "/cf", "one.xml", "/cf", "two.xml", "/dq", "en-US"});
}

TEST_F(Given_CreateConfig, When_OverwriteIsDuplicated_MatchesOutput)
{
    Verify("CreateConfig_DuplicateOverwrite", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/o", "/Overwrite"});
}

TEST_F(Given_CreateConfig, When_PlatformAndExtensionAreCombined_MatchesOutput)
{
    Verify(
        "CreateConfig_PlatformWithExtension",
        {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/pv", "10.0", "/ex", "missing.dll"});
}

TEST_F(Given_CreateConfig, When_ExtensionDoesNotExist_MatchesOutput)
{
    Verify("CreateConfig_MissingExtension", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/ex", "missing.dll"});
}

TEST_F(Given_CreateConfig, When_IndexOptionsAreCombined_MatchesOutput)
{
    Verify("CreateConfig_IndexOptions", {"createconfig", "/cf", "priconfig.xml", "/dq", "en-US", "/io", "hiddenfiles"});
}
