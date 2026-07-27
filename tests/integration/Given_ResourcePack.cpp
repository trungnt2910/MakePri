#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::InputLink;
using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::OUTPUT_KEY_WORKING_DIR;
using MakePri::Tests::VerifyIntegrationTest;

class Given_ResourcePack : public IntegrationTest
{};

namespace
{

const std::vector<InputLink> CommonInputs {
    {"priconfig.xml", "priconfig.xml"},
    {"Resources.resw", "project/Strings/vi-VN/Resources.resw"},
    {"resources.pri", "input.pri"},
};

const std::vector<InputLink> PopulatedInputs {
    {"priconfig.xml", "priconfig.xml"},
    {"Resources.resw", "project/Strings/vi-VN/Resources.resw"},
    {"resources_populated.pri", "input.pri"},
};

void Verify(
    const std::string& name,
    const std::vector<std::string>& arguments,
    const std::vector<std::filesystem::path>& outputs = {},
    const std::vector<InputLink>& inputs = CommonInputs)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = name,
            .arguments = arguments,
            .inputFiles = {},
            .inputLinks = inputs,
            .outputFiles = outputs,
        });
}

} // namespace

TEST_F(Given_ResourcePack, When_BasePriIsUsed_ResourcePackIsCreated)
{
    Verify(
        "ResourcePack_BasePri",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/of", "pack.pri", "/o"},
        {"pack.pri"});
}

TEST_F(Given_ResourcePack, When_PopulatedBasePriIsUsed_ResourcePackIsCreated)
{
    Verify(
        "ResourcePack_Populated",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/of", "pack.pri", "/o"},
        {"pack.pri"},
        PopulatedInputs);
}

TEST_F(Given_ResourcePack, When_LongOptionNamesAreUsed_MatchesOutput)
{
    Verify(
        "ResourcePack_LongNames",
        {"ResourcePack",
         "/ProjectRoot",
         "project",
         "/ConfigXML",
         "priconfig.xml",
         "/IndexFile",
         "input.pri",
         "/OutputFile",
         "pack.pri",
         "/Overwrite",
         "/Verbose"},
        {"pack.pri"});
}

TEST_F(Given_ResourcePack, When_AllOutputAndIndexingFlagsInteract_MatchesOutput)
{
    Verify(
        "ResourcePack_AllFlags",
        {"resourcepack",
         "/pr",
         "project",
         "/cf",
         "priconfig.xml",
         "/if",
         "input.pri",
         "/of",
         "pack.pri",
         "/il",
         "index.xml",
         "/sf",
         "schema.xml",
         "/am",
         "/rm",
         "/mf",
         "appx",
         "/io",
         "hiddenfiles,linkedfiles,disableDeduplication,-contentChecksumCreation",
         "/oo",
         "priheader,indexes",
         "/o",
         "/v"});
}

TEST_F(Given_ResourcePack, When_MappingReverseMapAndSchemaOutputInteract_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "ResourcePack_MappingReverseMapSchema",
            .arguments =
                {"resourcepack",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/if",
                 "input.pri",
                 "/of",
                 "pack.pri",
                 "/mf",
                 "appx",
                 "/rm",
                 "/sf",
                 "schema.xml",
                 "/o"},
            .inputFiles = {"project/Strings/vi-VN/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}, {"resources_populated.pri", "input.pri"}},
            .outputFiles = {"pack.pri", "pack.language-vi.pri", "schema.xml"},
            .outputTextFiles = {"pack.map.txt", "pack.language-vi.map.txt"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_ResourcePack, When_RequiredArgumentsAreMissing_MatchesOutput)
{
    Verify("ResourcePack_MissingArguments", {"resourcepack"}, {}, {});
}

TEST_F(Given_ResourcePack, When_InputIndexDoesNotExist_MatchesOutput)
{
    Verify(
        "ResourcePack_MissingIndex",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "missing.pri"},
        {},
        {{"priconfig.xml", "priconfig.xml"}});
}

TEST_F(Given_ResourcePack, When_MappingFileFormatIsInvalid_MatchesOutput)
{
    Verify("ResourcePack_InvalidMappingFile", {"resourcepack", "/mf", "unknown"}, {}, {});
}

TEST_F(Given_ResourcePack, When_IndexOptionIsInvalid_MatchesOutput)
{
    Verify("ResourcePack_InvalidIndexOption", {"resourcepack", "/io", "unknown"}, {}, {});
}

TEST_F(Given_ResourcePack, When_OutputOptionIsInvalid_MatchesOutput)
{
    Verify("ResourcePack_InvalidOutputOption", {"resourcepack", "/oo", "unknown"}, {}, {});
}

TEST_F(Given_ResourcePack, When_IndexNameIsSpecified_DefaultOutputIsCreated)
{
    Verify(
        "ResourcePack_UnexpectedIndexName",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/in", "Application"},
        {"resources.pri"},
        CommonInputs);
}

TEST_F(Given_ResourcePack, When_NewOnlyPlatformFlagIsCombined_MatchesOutput)
{
    Verify(
        "ResourcePack_UnexpectedPlatform",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/pv", "10.0"},
        {},
        CommonInputs);
}

TEST_F(Given_ResourcePack, When_ExtensionDoesNotExist_MatchesOutput)
{
    Verify(
        "ResourcePack_MissingExtension",
        {"resourcepack", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/ex", "missing.dll"},
        {},
        CommonInputs);
}
