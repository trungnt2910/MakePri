#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::VerifyIntegrationTest;

class Given_Command : public IntegrationTest
{};

TEST_F(Given_Command, When_CommandIsUnknown_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Command_Unknown",
            .arguments = {"unknown-command"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Command, When_UnknownSlashOptionIsUsed_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Command_UnknownSlashOption",
            .arguments = {"/unknown"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Command, When_UnknownHyphenOptionIsUsed_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Command_UnknownHyphenOption",
            .arguments = {"-unknown"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Command, When_BareValueFollowsCommand_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Command_BareValue",
            .arguments = {"new", "bare-value"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}

TEST_F(Given_Command, When_CommandNameHasSlashPrefix_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Command_SlashPrefixedName",
            .arguments = {"/new"},
            .inputFiles = {},
            .inputLinks = {},
            .outputFiles = {},
        });
}
