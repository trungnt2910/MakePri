#include "StdAfx.h"

#include "IntegrationTest.h"

using MakePri::Tests::IntegrationTest;
using MakePri::Tests::IntegrationTestCase;
using MakePri::Tests::OUTPUT_KEY_WORKING_DIR;
using MakePri::Tests::VerifyIntegrationTest;

namespace
{

enum class InputKind
{
    Resw,
    Resjson,
    Manifest,
};

struct InputCase
{
    const char* name;
    InputKind kind;
    bool succeeds;
};

IntegrationTestCase MakeTestCase(const InputCase& parameter)
{
    const bool isManifest = parameter.kind == InputKind::Manifest;
    const std::filesystem::path resourcePath =
        parameter.kind == InputKind::Resjson ? "project/Strings/en-US/Resources.resjson" : "project/Strings/en-US/Resources.resw";
    std::vector<std::filesystem::path> inputFiles {resourcePath};
    if (isManifest)
    {
        inputFiles.emplace_back("AppxManifest.xml");
    }
    std::vector<std::string> arguments {
        "new",
        "/pr",
        "project",
        "/cf",
        "priconfig.xml",
        "/of",
        "resources.pri",
        isManifest ? "/mn" : "/in",
        isManifest ? "AppxManifest.xml" : "Application",
        "/il",
        "index.xml",
        "/o",
    };

    return IntegrationTestCase {
        .name = "New_" + std::string(parameter.name),
        .arguments = std::move(arguments),
        .inputFiles = std::move(inputFiles),
        .inputLinks = {{"priconfig.xml", "priconfig.xml"}},
        .outputFiles = parameter.succeeds ? std::vector<std::filesystem::path> {"resources.pri"} : std::vector<std::filesystem::path> {},
        .outputTextFiles = parameter.succeeds ? std::vector<std::filesystem::path> {"index.xml"} : std::vector<std::filesystem::path> {},
        .replacements = {{std::string(OUTPUT_KEY_WORKING_DIR), ""}},
    };
}

void Verify(const char* const name, const InputKind kind, const bool succeeds)
{
    VerifyIntegrationTest(MakeTestCase({name, kind, succeeds}));
}

class Given_InputEdgeCases : public IntegrationTest
{};

} // namespace

#define MAKEPRI_INPUT_EDGE_CASE(name, kind, succeeds) \
    TEST_F(Given_InputEdgeCases, When_##name##_IsIndexed_MatchesOutput) { Verify(#name, InputKind::kind, succeeds); }

MAKEPRI_INPUT_EDGE_CASE(ReswUtf8Bom, Resw, true)
MAKEPRI_INPUT_EDGE_CASE(ReswUtf16LeBom, Resw, true)
MAKEPRI_INPUT_EDGE_CASE(ReswUtf16BeBom, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswUtf16LeNoBom, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswUtf32LeBom, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswUtf32BeBom, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswInvalidStart, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswInvalidMiddle, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswInvalidEnd, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswNulStart, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswNulMiddle, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswNulEnd, Resw, true)
MAKEPRI_INPUT_EDGE_CASE(ReswNormalization, Resw, true)
MAKEPRI_INPUT_EDGE_CASE(ReswEmpty, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswWhitespace, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswBomOnly, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswDoubleBom, Resw, false)
MAKEPRI_INPUT_EDGE_CASE(ReswDeclarationMismatch, Resw, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonUtf8Bom, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonUtf16LeBom, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonUtf16BeBom, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonUtf32LeBom, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonUtf32BeBom, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonInvalidStart, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonInvalidMiddle, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonInvalidEnd, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonNulStart, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonNulMiddle, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonNulEnd, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonNormalization, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonRootArray, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonRootNull, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonEmpty, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonWhitespace, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ResjsonBomOnly, Resjson, true)
MAKEPRI_INPUT_EDGE_CASE(ResjsonDoubleBom, Resjson, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestValidPrefixedNs, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestValidCommentsPi, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestWhitespaceName, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestSlashName, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestLongName, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestQualifiedNameAttribute, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestNestedIdentity, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestMissingName, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestMissingIdentity, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestMultipleIdentities, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestWrongNamespace, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestWrongRoot, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestLowercaseIdentity, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestNfcName, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestNfdName, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestUtf8Bom, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestUtf16LeBom, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestUtf16BeBom, Manifest, true)
MAKEPRI_INPUT_EDGE_CASE(ManifestInvalidMiddle, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestNulMiddle, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestInternalDoctype, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestEmpty, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestWhitespace, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestBomOnly, Manifest, false)
MAKEPRI_INPUT_EDGE_CASE(ManifestDuplicateNameAttribute, Manifest, false)

#undef MAKEPRI_INPUT_EDGE_CASE
