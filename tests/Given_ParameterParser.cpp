#include "StdAfx.h"

#include <ParameterParser.h>

using namespace Microsoft::Resources::Tools::MakePri;

namespace
{

TEST(Given_ParameterParser, When_NoArguments_HelpIsSelected)
{
    InputArgs inputArgs {};
    wchar_t executable[] = L"makepri.exe";
    wchar_t* argv[] {executable};

    ParameterParser parser(inputArgs);
    EXPECT_EQ(parser.Parse(1, argv), S_OK);
    EXPECT_TRUE(inputArgs.help);
    EXPECT_EQ(inputArgs.scenario, UsageScenario::None);
}

TEST(Given_ParameterParser, When_AllRecoveredOptionsAreValid_EveryFieldIsAssigned)
{
    InputArgs inputArgs {};
    wchar_t executable[] = L"makepri.exe";
    wchar_t command[] = L"nEw";
    wchar_t projectRootOption[] = L"-pr";
    wchar_t projectRoot[] = L"C:\\project";
    wchar_t configOption[] = L"-ConfigXML";
    wchar_t config[] = L"priconfig.xml";
    wchar_t dumpTypeOption[] = L"-dt";
    wchar_t dumpType[] = L"Detailed";
    wchar_t mappingOption[] = L"-mf";
    wchar_t mapping[] = L"APPX";
    wchar_t reverseMap[] = L"-rm";
    wchar_t platformOption[] = L"-pv";
    wchar_t platform[] = L"10.0.0.5";
    wchar_t* argv[] {
        executable,
        command,
        projectRootOption,
        projectRoot,
        configOption,
        config,
        dumpTypeOption,
        dumpType,
        mappingOption,
        mapping,
        reverseMap,
        platformOption,
        platform,
    };

    ParameterParser parser(inputArgs);
    EXPECT_EQ(parser.Parse(static_cast<int>(std::size(argv)), argv), S_OK);
    EXPECT_EQ(inputArgs.scenario, UsageScenario::New);
    EXPECT_EQ(inputArgs.projectRoot, projectRoot);
    EXPECT_EQ(inputArgs.configXml, config);
    EXPECT_EQ(inputArgs.dumpType, PriDumpType::Detailed);
    EXPECT_EQ(inputArgs.mappingFileFormat, MappingFileFormat::AppX);
    EXPECT_TRUE(inputArgs.reverseMap);
    EXPECT_EQ(inputArgs.platformVersion, Microsoft::Resources::MrmPlatformVersionInternal::WindowsCoreRS4);
}

TEST(Given_ParameterParser, When_DumpTypeIsRepeated_TheLastValueWins)
{
    InputArgs inputArgs {};
    wchar_t executable[] = L"makepri.exe";
    wchar_t command[] = L"dump";
    wchar_t firstOption[] = L"-dt";
    wchar_t firstValue[] = L"Detailed";
    wchar_t secondOption[] = L"-dt";
    wchar_t secondValue[] = L"Summary";
    wchar_t* argv[] {
        executable,
        command,
        firstOption,
        firstValue,
        secondOption,
        secondValue,
    };

    ParameterParser parser(inputArgs);
    EXPECT_EQ(parser.Parse(static_cast<int>(std::size(argv)), argv), S_OK);
    EXPECT_EQ(inputArgs.dumpType, PriDumpType::Summary);
}

TEST(Given_ParameterParser, When_PointerOptionIsRepeated_DuplicateIsReturned)
{
    InputArgs inputArgs {};
    wchar_t executable[] = L"makepri.exe";
    wchar_t command[] = L"dump";
    wchar_t firstOption[] = L"-if";
    wchar_t firstValue[] = L"one.pri";
    wchar_t secondOption[] = L"-IndexFile";
    wchar_t secondValue[] = L"two.pri";
    wchar_t* argv[] {
        executable,
        command,
        firstOption,
        firstValue,
        secondOption,
        secondValue,
    };

    ParameterParser parser(inputArgs);
    EXPECT_EQ(parser.Parse(static_cast<int>(std::size(argv)), argv), HRESULT_FROM_WIN32(ERROR_DUPLICATE_TAG));
    EXPECT_EQ(inputArgs.indexFile, firstValue);
}

TEST(Given_ParameterParser, When_OutputOptionsValueIsMissing_ItsErrorFlagIsSet)
{
    InputArgs inputArgs {};
    wchar_t executable[] = L"makepri.exe";
    wchar_t command[] = L"dump";
    wchar_t option[] = L"-oo";
    wchar_t* argv[] {executable, command, option};

    ParameterParser parser(inputArgs);
    EXPECT_EQ(parser.Parse(static_cast<int>(std::size(argv)), argv), E_INVALIDARG);
    EXPECT_TRUE(inputArgs.outputOptionsError);
}

} // namespace
