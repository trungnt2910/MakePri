#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::InputLink;
using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::VerifyIntegrationTest;

class Given_Dump : public IntegrationTest
{};

namespace
{

void Verify(
    const std::string& name,
    const std::vector<std::string>& arguments,
    const std::vector<std::filesystem::path>& outputs = {},
    const std::vector<std::filesystem::path>& inputs = {},
    const std::vector<InputLink>& links = {})
{
    VerifyIntegrationTest(
        IntegrationTestCase {
            .name = name,
            .arguments = arguments,
            .inputFiles = inputs,
            .inputLinks = links,
            .outputFiles = outputs,
        });
}

const std::vector<InputLink> PriInput {
    {"resources_populated.pri", "resources.pri"},
};

} // namespace

TEST_F(Given_Dump, When_BasicDumpIsRequested_MatchesOutput)
{
    Verify("Dump_Basic", {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Basic", "/o"}, {"dump.xml"}, {}, PriInput);
}

TEST_F(Given_Dump, When_ResourceNamesContainSpecialCharacters_URIsAreNotPercentEncoded)
{
    Verify(
        "Dump_SpecialCharacters",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Basic", "/o"},
        {"dump.xml"},
        {},
        {{"resources_special_characters.pri", "resources.pri"}});
}

TEST_F(Given_Dump, When_PRIContainsMultipleLanguages_DetailedDumpContainsEveryCandidate)
{
    Verify(
        "Dump_MultiLanguage",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Detailed", "/o"},
        {"dump.xml"},
        {},
        {{"resources_multilanguage.pri", "resources.pri"}});
}

TEST_F(Given_Dump, When_DetailedDumpIsRequested_MatchesOutput)
{
    Verify("Dump_Detailed", {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Detailed", "/o"}, {"dump.xml"}, {}, PriInput);
}

TEST_F(Given_Dump, When_SchemaDumpIsRequested_MatchesOutput)
{
    Verify("Dump_Schema", {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Schema", "/o"}, {"dump.xml"}, {}, PriInput);
}

TEST_F(Given_Dump, When_SummaryDumpIsRequested_MatchesOutput)
{
    Verify("Dump_Summary", {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Summary", "/o"}, {"dump.xml"}, {}, PriInput);
}

TEST_F(Given_Dump, When_LongOptionNamesAreUsed_MatchesOutput)
{
    Verify(
        "Dump_LongNames",
        {"Dump", "/IndexFile", "resources.pri", "/OutputFile", "dump.xml", "/DumpType", "Basic", "/Overwrite", "/Verbose"},
        {"dump.xml"},
        {},
        PriInput);
}

TEST_F(Given_Dump, When_AllOutputOptionsAreCombined_MatchesOutput)
{
    Verify(
        "Dump_OutputOptions",
        {"dump",
         "/if",
         "resources.pri",
         "/of",
         "dump.xml",
         "/oo",
         "priheader,indexes,uris,qualifierinfo,resourcemaps,versioninfo,emptysubtrees,emptynamedresources,links,"
         "namedresourcedecision,candidates,+linkedcandidates,+sanitizexml,+rawlocators",
         "/o"},
        {"dump.xml"},
        {},
        PriInput);
}

TEST_F(Given_Dump, When_ShortNegativeOutputOptionsAreCombined_MatchesOutput)
{
    Verify(
        "Dump_ShortOutputOptions",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/oo", "-ph,-ix,-uri,-qi,-rm,-vi,-es,-enr,-l,-nrd,-c,-lc,-sxml,-rl", "/o"},
        {"dump.xml"},
        {},
        PriInput);
}

TEST_F(Given_Dump, When_DumpTypeIsRepeated_MatchesOutput)
{
    Verify(
        "Dump_RepeatedType",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/dt", "Basic", "/DumpType", "Summary", "/o"},
        {"dump.xml"},
        {},
        PriInput);
}

TEST_F(Given_Dump, When_ExternalSchemaIsUsed_MatchesOutput)
{
    Verify(
        "Dump_ExternalSchema",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/es", "schema.pri", "/o"},
        {"dump.xml"},
        {},
        {{"resources.pri", "resources.pri"}, {"resources.pri", "schema.pri"}});
}

TEST_F(Given_Dump, When_LongExternalSchemaOptionIsUsed_MatchesOutput)
{
    Verify(
        "Dump_LongExternalSchema",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/ExternalSchema", "schema.pri", "/o"},
        {"dump.xml"},
        {},
        {{"resources.pri", "resources.pri"}, {"resources.pri", "schema.pri"}});
}

TEST_F(Given_Dump, When_ExternalSchemaIsCorrupt_MatchesOutput)
{
    Verify(
        "Dump_CorruptExternalSchema",
        {"dump", "/if", "resources.pri", "/of", "dump.xml", "/es", "schema.pri", "/o"},
        {},
        {},
        {{"resources_populated.pri", "resources.pri"}, {"Dump_Corrupt/corrupt.pri", "schema.pri"}});
}

TEST_F(Given_Dump, When_InputDoesNotExist_MatchesOutput)
{
    Verify("Dump_MissingInput", {"dump", "/if", "missing.pri", "/of", "dump.xml", "/o"});
}

TEST_F(Given_Dump, When_InputIsCorrupt_MatchesOutput)
{
    Verify("Dump_Corrupt", {"dump", "/if", "corrupt.pri", "/of", "dump.xml", "/o"}, {}, {"corrupt.pri"});
}

TEST_F(Given_Dump, When_ResourceMapSectionIsMalformed_MatchesOutput)
{
    Verify("Dump_MalformedResourceMap", {"dump", "/if", "input.pri", "/of", "dump.xml", "/dt", "detailed", "/o"}, {}, {"input.pri"});
}

TEST_F(Given_Dump, When_DumpTypeIsInvalid_MatchesOutput) { Verify("Dump_InvalidType", {"dump", "/dt", "unknown"}); }

TEST_F(Given_Dump, When_OutputOptionIsInvalid_MatchesOutput) { Verify("Dump_InvalidOutputOption", {"dump", "/oo", "unknown"}); }

TEST_F(Given_Dump, When_OutputOptionValueIsMissing_MatchesOutput) { Verify("Dump_MissingOutputOptionValue", {"dump", "/oo"}); }

TEST_F(Given_Dump, When_IndexOptionsAreCombined_MatchesOutput) { Verify("Dump_IndexOptions", {"dump", "/io", "hiddenfiles"}); }

TEST_F(Given_Dump, When_ExtensionDoesNotExist_MatchesOutput)
{
    Verify("Dump_MissingExtension", {"dump", "/if", "resources.pri", "/of", "dump.xml", "/ex", "missing.dll"}, {}, {}, PriInput);
}
