#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::InputLink;
using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::OUTPUT_KEY_WORKING_DIR;
using MakePri::Tests::VerifyIntegrationTest;

class Given_New : public IntegrationTest
{};

namespace
{

const std::vector<InputLink> CommonInputs {
    {"priconfig.xml", "priconfig.xml"},
    {"Resources.resw", "project/Strings/en-US/Resources.resw"},
};

void Verify(
    const std::string& name,
    const std::vector<std::string>& arguments,
    const std::vector<std::filesystem::path>& outputs = {},
    std::vector<InputLink> inputs = CommonInputs)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = name,
            .arguments = arguments,
            .inputFiles = {},
            .inputLinks = std::move(inputs),
            .outputFiles = outputs,
        });
}

const std::vector<std::string> BaseArguments {
    "new",
    "/pr",
    "project",
    "/cf",
    "priconfig.xml",
    "/of",
    "resources.pri",
    "/in",
    "Application",
    "/o",
};

} // namespace

TEST_F(Given_New, When_MinimalProjectIsIndexed_MatchesOutput) { Verify("New_Minimal", BaseArguments, {"resources.pri"}); }

TEST_F(Given_New, When_ResourceFileIsRegular_TwoCandidatesAreIndexed)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_WithCandidates",
            .arguments = BaseArguments,
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri"},
        });
}

TEST_F(Given_New, When_MultipleLanguagesAreIndexed_FourCandidatesAreCreated)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_MultiLanguage",
            .arguments = BaseArguments,
            .inputFiles = {"priconfig.xml", "project/Strings/en-US/Resources.resw", "project/Strings/vi-VN/Resources.resw"},
            .inputLinks = {},
            .outputFiles = {"resources.pri"},
        });
}

TEST_F(Given_New, When_ResourcePackSchemasAreOmitted_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_OmitSchemaFromResourcePacks",
            .arguments = BaseArguments,
            .inputFiles = {"priconfig.xml", "project/Strings/en-US/Resources.resw", "project/Strings/vi-VN/Resources.resw"},
            .inputLinks = {},
            .outputFiles = {"resources.pri", "resources.language-vi.pri"},
        });
}

TEST_F(Given_New, When_LongOptionNamesAreUsed_MatchesOutput)
{
    Verify(
        "New_LongNames",
        {"New",
         "/ProjectRoot",
         "project",
         "/ConfigXML",
         "priconfig.xml",
         "/OutputFile",
         "resources.pri",
         "/IndexName",
         "Application",
         "/Overwrite"},
        {"resources.pri"});
}

TEST_F(Given_New, When_ManifestSuppliesIndexName_MatchesOutput)
{
    auto inputs = CommonInputs;
    inputs.push_back({"AppxManifest.xml", "AppxManifest.xml"});
    Verify(
        "New_Manifest",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/Manifest", "AppxManifest.xml", "/o"},
        {"resources.pri"},
        std::move(inputs));
}

TEST_F(Given_New, When_ManifestIsMalformed_PrimaryErrorPrecedesParserDetail)
{
    Verify(
        "New_MalformedManifest",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/mn", "AppxManifest.xml", "/o"},
        {},
        {{"priconfig.xml", "priconfig.xml"},
         {"Resources.resw", "project/Strings/en-US/Resources.resw"},
         {"New_MalformedManifest/AppxManifest.xml", "AppxManifest.xml"}});
}

TEST_F(Given_New, When_ManifestIdentityNameContainsPunctuation_MatchesOutput)
{
    Verify(
        "New_UnusualManifest",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/mn", "AppxManifest.xml", "/o"},
        {"resources.pri"},
        {{"priconfig.xml", "priconfig.xml"},
         {"Resources.resw", "project/Strings/en-US/Resources.resw"},
         {"New_UnusualManifest/AppxManifest.xml", "AppxManifest.xml"}});
}

TEST_F(Given_New, When_ManifestHasNoNamespace_MatchesOutput)
{
    Verify(
        "New_ManifestWithoutNamespace",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/mn", "AppxManifest.xml", "/o"},
        {},
        {{"priconfig.xml", "priconfig.xml"},
         {"Resources.resw", "project/Strings/en-US/Resources.resw"},
         {"New_ManifestWithoutNamespace/AppxManifest.xml", "AppxManifest.xml"}});
}

TEST_F(Given_New, When_VersionMajorIsSpecified_MatchesOutput)
{
    Verify(
        "New_VersionMajor",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/VersionMajor", "7", "/o"},
        {"resources.pri"});
}

TEST_F(Given_New, When_IndexLogAndOutputOptionsInteract_MatchesOutput)
{
    Verify(
        "New_IndexLog",
        {"new",
         "/pr",
         "project",
         "/cf",
         "priconfig.xml",
         "/of",
         "resources.pri",
         "/in",
         "Application",
         "/IndexLog",
         "index.xml",
         "/OutputOptions",
         "priheader,indexes,+linkedcandidates,-rawlocators",
         "/o"},
        {"resources.pri", "index.xml"});
}

TEST_F(Given_New, When_IndexLogContainsDifferentPathSeparators_UsesBackslashes)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_IndexLogPathSeparators",
            .arguments =
                {"new",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/of",
                 "resources.pri",
                 "/in",
                 "Application",
                 "/IndexLog",
                 "index.xml",
                 "/o"},
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {},
            .outputTextFiles = {"index.xml"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_SchemaFileIsRequested_MatchesOutput)
{
    Verify(
        "New_SchemaFile",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/SchemaFile", "schema.xml", "/o"},
        {"resources.pri", "schema.xml"});
}

TEST_F(Given_New, When_SchemaIndexesAreDisabled_IndexAttributesAreOmitted)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_SchemaFileWithoutIndexes",
            .arguments =
                {"new",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/of",
                 "resources.pri",
                 "/in",
                 "Application",
                 "/sf",
                 "schema.xml",
                 "/oo",
                 "-indexes",
                 "/o"},
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri", "schema.xml"},
        });
}

TEST_F(Given_New, When_ResjsonContainsNestedResourcesAndMetadata_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_ResjsonNestedMetadata",
            .arguments =
                {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/il", "index.xml", "/o"},
            .inputFiles = {"project/Strings/en-US/Resources.resjson"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri"},
            .outputTextFiles = {"index.xml"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_ReswIsMalformed_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_MalformedResw",
            .arguments = BaseArguments,
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {},
        });
}

TEST_F(Given_New, When_ResjsonIsMalformed_MatchesOutput)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_MalformedResjson",
            .arguments = BaseArguments,
            .inputFiles = {"project/Strings/en-US/Resources.resjson"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_AutoMergeAndReverseMapInteract_MatchesOutput)
{
    Verify(
        "New_AutoMergeReverseMap",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/am", "/ReverseMap", "/o"});
}

TEST_F(Given_New, When_MappingFileFormatIsAppx_MatchesOutput)
{
    Verify(
        "New_MappingFile",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/MappingFile", "appx", "/o"},
        {"resources.pri"});
}

TEST_F(Given_New, When_MappingFileContainsPlainFolderResource_UsesPhysicalProjectPath)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_MappingFileFolderPlain",
            .arguments =
                {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/mf", "appx", "/o"},
            .inputFiles = {"project/Assets/logo.txt"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri"},
            .outputTextFiles = {"resources.map.txt"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_MappingFileContainsQualifiedFolderResource_PreservesPhysicalName)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_MappingFileFolderQualified",
            .arguments =
                {"new", "/pr", "project", "/cf", "priconfig.xml", "/of", "resources.pri", "/in", "Application", "/mf", "appx", "/o"},
            .inputFiles = {"project/Assets/logo.scale-200.txt"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri", "resources.scale-200.pri"},
            .outputTextFiles = {"resources.map.txt", "resources.scale-200.map.txt"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_LinkedDirectoryIsIgnored_WarningUsesFullPath)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_IgnoredLinkedDirectory",
            .arguments = BaseArguments,
            .inputFiles = {},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}, {"New_MultiLanguage/project/Strings", "project/Strings"}},
            .outputFiles = {},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_LinkedResourceFileIsEnabled_FileIsIndexed)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_LinkedResourceFile",
            .arguments =
                {"new",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/of",
                 "resources.pri",
                 "/in",
                 "Application",
                 "/io",
                 "+linkedfiles",
                 "/il",
                 "index.xml",
                 "/o"},
            .inputFiles = {},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}, {"Resources.resw", "project/Strings/en-US/Resources.resw"}},
            .outputFiles = {"resources.pri"},
            .outputTextFiles = {"index.xml"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_ReswLinksToAnotherResource_LinkCandidateIsIndexed)
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = "New_ValidResourceLink",
            .arguments =
                {"new",
                 "/pr",
                 "project",
                 "/cf",
                 "priconfig.xml",
                 "/of",
                 "resources.pri",
                 "/in",
                 "Application",
                 "/il",
                 "index.xml",
                 "/oo",
                 "links,+linkedcandidates",
                 "/o"},
            .inputFiles = {"project/Strings/en-US/Resources.resw"},
            .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
            .outputFiles = {"resources.pri"},
            .outputTextFiles = {"index.xml"},
            .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
        });
}

TEST_F(Given_New, When_AllBooleanIndexOptionsAreCombined_MatchesOutput)
{
    Verify(
        "New_IndexOptions",
        {"new",
         "/pr",
         "project",
         "/cf",
         "priconfig.xml",
         "/of",
         "resources.pri",
         "/in",
         "Application",
         "/IndexOptions",
         "+hiddenfiles,+linkedfiles,+disableDeduplication,-contentChecksumCreation",
         "/o"},
        {"resources.pri"});
}

TEST_F(Given_New, When_ShortIndexOptionNamesAreCombined_MatchesOutput)
{
    auto inputs = CommonInputs;
    inputs.push_back({"schema.xml", "schema.xml"});
    Verify(
        "New_ShortIndexOptions",
        {"new",
         "/pr",
         "project",
         "/cf",
         "priconfig.xml",
         "/of",
         "resources.pri",
         "/in",
         "Application",
         "/io",
         "+hf,+lf,+dd,-cc,is=schema.xml",
         "/o"},
        {"resources.pri"},
        std::move(inputs));
}

TEST_F(Given_New, When_VerboseIsUsed_MatchesOutput)
{
    auto arguments = BaseArguments;
    arguments.push_back("/v");
    Verify("New_Verbose", arguments, {"resources.pri"});
}

TEST_F(Given_New, When_RequiredArgumentsAreMissing_MatchesOutput) { Verify("New_MissingArguments", {"new"}, {}, {}); }

TEST_F(Given_New, When_OptionValueIsMissing_MatchesOutput) { Verify("New_MissingValue", {"new", "/pr"}, {}, {}); }

TEST_F(Given_New, When_VersionMajorIsInvalid_MatchesOutput) { Verify("New_InvalidVersionMajor", {"new", "/vma", "zero"}, {}, {}); }

TEST_F(Given_New, When_MappingFileFormatIsInvalid_MatchesOutput) { Verify("New_InvalidMappingFile", {"new", "/mf", "other"}, {}, {}); }

TEST_F(Given_New, When_OutputOptionsHaveNoOutputConsumer_MatchesOutput)
{
    Verify(
        "New_OutputOptionsWithoutLog",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/in", "Application", "/oo", "priheader"},
        {},
        CommonInputs);
}

TEST_F(Given_New, When_OutputOptionIsInvalid_MatchesOutput) { Verify("New_InvalidOutputOption", {"new", "/oo", "unknown"}, {}, {}); }

TEST_F(Given_New, When_IndexOptionIsInvalid_MatchesOutput) { Verify("New_InvalidIndexOption", {"new", "/io", "unknown"}, {}, {}); }

TEST_F(Given_New, When_IndexerSchemaDoesNotExist_MatchesOutput)
{
    Verify("New_MissingIndexerSchema", {"new", "/io", "indexerschema=missing.xml"}, {}, {});
}

TEST_F(Given_New, When_SamePathOptionIsDuplicated_MatchesOutput)
{
    Verify("New_DuplicateProjectRoot", {"new", "/pr", "one", "/ProjectRoot", "two"}, {}, {});
}

TEST_F(Given_New, When_SameFlagIsDuplicated_MatchesOutput) { Verify("New_DuplicateAutoMerge", {"new", "/am", "/AutoMerge"}, {}, {}); }

TEST_F(Given_New, When_ExtensionDoesNotExist_MatchesOutput)
{
    Verify(
        "New_MissingExtension",
        {"new", "/pr", "project", "/cf", "priconfig.xml", "/in", "Application", "/ExtensionDll", "missing.dll"},
        {},
        CommonInputs);
}
