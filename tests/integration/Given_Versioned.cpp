#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::InputLink;
using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::OUTPUT_KEY_WORKING_DIR;
using MakePri::Tests::VerifyIntegrationTest;

class Given_Versioned : public IntegrationTest
{};

namespace
{

const std::vector<InputLink> CommonInputs {
    {"priconfig.xml", "priconfig.xml"},
    {"Resources.resw", "project/Strings/en-US/Resources.resw"},
    {"resources.pri", "input.pri"},
};

const std::vector<InputLink> PopulatedInputs {
    {"priconfig.xml", "priconfig.xml"},
    {"Resources.resw", "project/Strings/en-US/Resources.resw"},
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

TEST_F(Given_Versioned, When_MinimalProjectIsIndexed_MatchesOutput)
{
    Verify(
        "Versioned_Minimal",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/of", "versioned.pri", "/o"},
        {"versioned.pri"});
}

TEST_F(Given_Versioned, When_PopulatedBasePriIsIndexed_MatchesOutput)
{
    Verify(
        "Versioned_Populated",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/of", "versioned.pri", "/o"},
        {"versioned.pri"},
        PopulatedInputs);
}

TEST_F(Given_Versioned, When_LongOptionNamesAreUsed_MatchesOutput)
{
    Verify(
        "Versioned_LongNames",
        {"Versioned",
         "/ProjectRoot",
         "project",
         "/ConfigXML",
         "priconfig.xml",
         "/IndexFile",
         "input.pri",
         "/OutputFile",
         "versioned.pri",
         "/Overwrite",
         "/Verbose"},
        {"versioned.pri"});
}

TEST_F(Given_Versioned, When_AllOutputAndIndexingFlagsInteract_MatchesOutput)
{
    Verify(
        "Versioned_AllFlags",
        {"versioned",
         "/pr",
         "project",
         "/cf",
         "priconfig.xml",
         "/if",
         "input.pri",
         "/of",
         "versioned.pri",
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

TEST_F(Given_Versioned, When_MappingReverseMapAndSchemaOutputInteract_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "Versioned_MappingReverseMapSchema",
            .arguments =
                {"versioned",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/if",
                 "input.pri",
                 "/of",
                 "versioned.pri",
                 "/mf",
                 "appx",
                 "/rm",
                 "/sf",
                 "schema.xml",
                 "/o"},
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}, {"resources_populated.pri", "input.pri"}},
            .outputFiles = {"versioned.pri", "schema.xml"},
            .outputTextFiles = {"versioned.map.txt"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_Versioned, When_RequiredArgumentsAreMissing_MatchesOutput) { Verify("Versioned_MissingArguments", {"versioned"}, {}, {}); }

TEST_F(Given_Versioned, When_InputIndexDoesNotExist_MatchesOutput)
{
    Verify(
        "Versioned_MissingIndex",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "missing.pri"},
        {},
        {{"priconfig.xml", "priconfig.xml"}});
}

TEST_F(Given_Versioned, When_MappingFileFormatIsInvalid_MatchesOutput)
{
    Verify("Versioned_InvalidMappingFile", {"versioned", "/mf", "unknown"}, {}, {});
}

TEST_F(Given_Versioned, When_IndexOptionIsInvalid_MatchesOutput)
{
    Verify("Versioned_InvalidIndexOption", {"versioned", "/io", "unknown"}, {}, {});
}

TEST_F(Given_Versioned, When_OutputOptionIsInvalid_MatchesOutput)
{
    Verify("Versioned_InvalidOutputOption", {"versioned", "/oo", "unknown"}, {}, {});
}

TEST_F(Given_Versioned, When_NewOnlyVersionFlagIsCombined_MatchesOutput)
{
    Verify(
        "Versioned_UnexpectedVersionMajor",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/vma", "2"},
        {},
        CommonInputs);
}

TEST_F(Given_Versioned, When_NewOnlyManifestFlagIsCombined_MatchesOutput)
{
    Verify(
        "Versioned_UnexpectedManifest",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/mn", "manifest.xml"},
        {},
        CommonInputs);
}

TEST_F(Given_Versioned, When_ExtensionDoesNotExist_MatchesOutput)
{
    Verify(
        "Versioned_MissingExtension",
        {"versioned", "/pr", "project", "/cf", "priconfig.xml", "/if", "input.pri", "/ex", "missing.dll"},
        {},
        CommonInputs);
}
