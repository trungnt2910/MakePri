#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::VerifyIntegrationTest;

class Given_Help : public IntegrationTest
{};

namespace
{

void Verify(const std::string& name, const std::vector<std::string>& arguments)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = name,
            .arguments = arguments,
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

} // namespace

TEST_F(Given_Help, When_NoArguments_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_NoArguments",
            .arguments = {},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_QuestionMark_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_QuestionMark",
            .arguments = {"/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_HelpCommand_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_Command",
            .arguments = {"help"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_CreateConfigHelp_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_CreateConfig",
            .arguments = {"createconfig", "/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_NewHelp_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_New",
            .arguments = {"new", "/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_VersionedHelp_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_Versioned",
            .arguments = {"versioned", "/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_ResourcePackHelp_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_ResourcePack",
            .arguments = {"resourcepack", "/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_DumpHelp_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Help_Dump",
            .arguments = {"dump", "/?"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Help, When_LongHelpOption_MatchesOutput) { Verify("Help_LongOption", {"/help"}); }

TEST_F(Given_Help, When_ShortHelpOption_MatchesOutput) { Verify("Help_ShortOption", {"/h"}); }

TEST_F(Given_Help, When_HyphenPrefixIsUsed_MatchesOutput) { Verify("Help_HyphenPrefix", {"new", "-h"}); }

TEST_F(Given_Help, When_OptionCasingDiffers_MatchesOutput) { Verify("Help_CaseInsensitive", {"DuMp", "/HeLp"}); }

TEST_F(Given_Help, When_HelpPrecedesOtherOptions_MatchesOutput) { Verify("Help_WithOtherOptions", {"new", "/h", "/pr", "missing"}); }

TEST_F(Given_Help, When_HelpIsDuplicated_MatchesOutput) { Verify("Help_Duplicated", {"new", "/h", "/?"}); }
