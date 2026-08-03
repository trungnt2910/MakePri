#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace MakePri::Tests
{

constexpr std::string_view OUTPUT_KEY_WORKING_DIR = "@@WORKING_DIR@@";

struct IntegrationConfiguration
{
    std::filesystem::path makePriUnderTest;
    std::filesystem::path officialMakePri;
    std::filesystem::path inputRoot;
    std::filesystem::path outputRoot;
    int exitCodeBits = 32;
    bool forwardSlashCompatibility = false;
    bool utf8Console = false;
    bool updateOutputs = false;
};

extern IntegrationConfiguration g_integrationConfiguration;

class IntegrationTest : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

private:
    std::filesystem::path m_originalDirectory;
    std::filesystem::path m_temporaryDirectory;
};

struct InputLink
{
    std::filesystem::path source;
    std::filesystem::path destination;
};

struct IntegrationTestCase
{
    std::string name;
    std::vector<std::string> arguments;
    std::vector<std::filesystem::path> inputFiles = {};
    std::vector<InputLink> inputLinks = {};
    std::vector<std::filesystem::path> outputFiles = {};
    std::vector<std::filesystem::path> outputTextFiles = {};
    std::map<std::string, std::string> replacements = {};
};

void VerifyIntegrationTest(const IntegrationTestCase& testCase);

} // namespace MakePri::Tests
