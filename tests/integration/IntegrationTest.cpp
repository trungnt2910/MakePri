#include "StdAfx.h"

#include "IntegrationTest.h"

#include <uni_algo/conv.h>

namespace MakePri::Tests
{

void IntegrationTest::SetUp()
{
    m_originalDirectory = std::filesystem::current_path();
    const testing::TestInfo* const testInfo = testing::UnitTest::GetInstance()->current_test_info();
    static std::random_device randomDevice;
    for (std::size_t attempt = 0; attempt < 100; ++attempt)
    {
        const std::string directoryName = "makepri-integration-" + std::string(testInfo->test_suite_name()) + "-" + testInfo->name() + "-" +
                                          std::to_string(randomDevice()) + "-" + std::to_string(randomDevice());
        const std::filesystem::path candidate = std::filesystem::temp_directory_path() / directoryName;
        std::error_code error;
        if (std::filesystem::create_directory(candidate, error))
        {
            m_temporaryDirectory = candidate;
            std::filesystem::current_path(m_temporaryDirectory);
            return;
        }
        ASSERT_FALSE(error) << "Unable to create temporary directory " << candidate.string() << ": " << error.message();
    }
    FAIL() << "Unable to choose a unique integration-test directory";
}

void IntegrationTest::TearDown()
{
    std::error_code error;
    std::filesystem::current_path(m_originalDirectory, error);
    EXPECT_FALSE(error) << "Unable to restore the integration-test directory: " << error.message();
    if (!m_temporaryDirectory.empty())
    {
        error.clear();
        std::filesystem::remove_all(m_temporaryDirectory, error);
        EXPECT_FALSE(error) << "Unable to remove temporary directory " << m_temporaryDirectory.string() << ": " << error.message();
    }
}

namespace
{

struct ProcessResult
{
    std::uint32_t exitCode = 0;
    std::vector<std::byte> standardOutput;
    std::vector<std::byte> standardError;
    std::map<std::filesystem::path, std::vector<std::byte>> outputFiles;
    std::map<std::filesystem::path, std::string> outputTextFiles;
};

std::uint32_t ExitCodeMask()
{
    const auto bits = static_cast<std::uint32_t>(g_integrationConfiguration.exitCodeBits);
    return bits == 32 ? std::numeric_limits<std::uint32_t>::max() : (std::uint32_t {1} << bits) - 1;
}

std::vector<std::byte> ReadFileBytes(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream)
    {
        return {};
    }

    const std::streamoff end = stream.tellg();
    if ((end < 0) || (static_cast<std::uintmax_t>(end) > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max())))
    {
        ADD_FAILURE() << "File is too large to read: " << path.string();
        return {};
    }
    const auto size = static_cast<std::streamsize>(end);
    std::vector<std::byte> result(static_cast<std::size_t>(end));
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(result.data()), size);
    return result;
}

void WriteFileBytes(const std::filesystem::path& path, const std::vector<std::byte>& contents)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(stream) << "Unable to open sample output " << path.string();
    stream.write(reinterpret_cast<const char*>(contents.data()), static_cast<std::streamsize>(contents.size()));
    ASSERT_TRUE(stream) << "Unable to write sample output " << path.string();
}

std::string ReadFileText(const std::filesystem::path& path)
{
    const std::vector<std::byte> contents = ReadFileBytes(path);
    return {reinterpret_cast<const char*>(contents.data()), contents.size()};
}

void WriteFileText(const std::filesystem::path& path, const std::string_view contents)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(stream) << "Unable to open sample output " << path.string();
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    ASSERT_TRUE(stream) << "Unable to write sample output " << path.string();
}

void ReplaceAll(std::string* const contents, const std::string_view value, const std::string_view replacement)
{
    std::size_t position = 0;
    while ((position = contents->find(value, position)) != std::string::npos)
    {
        contents->replace(position, value.size(), replacement);
        position += replacement.size();
    }
}

std::string ResolveOutputValue(const std::string_view key, const std::string_view value, const std::filesystem::path& workingDirectory)
{
    if ((key == OUTPUT_KEY_WORKING_DIR) && value.empty())
    {
        return workingDirectory.string();
    }
    return std::string(value);
}

void ExpandOutputKeys(
    std::string* const contents,
    const std::map<std::string, std::string>& replacements,
    const std::filesystem::path& workingDirectory)
{
    for (const auto& [key, configuredValue] : replacements)
    {
        ReplaceAll(contents, key, ResolveOutputValue(key, configuredValue, workingDirectory));
    }
}

void ContractOutputValues(
    std::string* const contents,
    const std::map<std::string, std::string>& replacements,
    const std::filesystem::path& workingDirectory)
{
    for (const auto& [key, configuredValue] : replacements)
    {
        ReplaceAll(contents, ResolveOutputValue(key, configuredValue, workingDirectory), key);
    }
}

std::string QuoteArgument(const std::string& argument)
{
    if (argument.find_first_of(" \t") == std::string::npos)
    {
        return argument;
    }
    return "\"" + argument + "\"";
}

std::string MakeCommandLine(const std::filesystem::path& executable, const std::vector<std::string>& arguments)
{
    std::string commandLine = QuoteArgument(executable.string());
    for (const std::string& argument : arguments)
    {
        std::string compatibleArgument = argument;
        if (g_integrationConfiguration.forwardSlashCompatibility && compatibleArgument.starts_with('/'))
        {
            compatibleArgument.front() = '-';
        }
        commandLine.push_back(' ');
        commandLine.append(QuoteArgument(compatibleArgument));
    }
    return commandLine;
}

void CopyInputs(const IntegrationTestCase& testCase, const std::filesystem::path& workingDirectory)
{
    const std::filesystem::path caseDirectory = g_integrationConfiguration.inputRoot / testCase.name;
    for (const std::filesystem::path& relativePath : testCase.inputFiles)
    {
        const std::filesystem::path destination = workingDirectory / relativePath;
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::copy_file(caseDirectory / relativePath, destination, std::filesystem::copy_options::overwrite_existing);
    }

    for (const InputLink& link : testCase.inputLinks)
    {
        const std::filesystem::path source = g_integrationConfiguration.inputRoot / link.source;
        const std::filesystem::path destination = workingDirectory / link.destination;
        std::filesystem::create_directories(destination.parent_path());
        if (std::filesystem::is_directory(source))
        {
            std::filesystem::create_directory_symlink(source, destination);
        }
        else
        {
            std::filesystem::create_symlink(source, destination);
        }
    }
}

std::string EncodeUtf16(const std::string_view value)
{
    const std::u16string utf16 = una::utf8to16<char, char16_t>(value);
    std::string bytes;
    bytes.reserve(utf16.size() * sizeof(char16_t));
    for (const char16_t codeUnit : utf16)
    {
        bytes.push_back(static_cast<char>(codeUnit & 0xff));
        bytes.push_back(static_cast<char>((codeUnit >> 8) & 0xff));
    }
    return bytes;
}

void ConvertUtf16ToUtf8(std::vector<std::byte>* const contents)
{
    if (!g_integrationConfiguration.utf8Console || contents->empty())
    {
        return;
    }
    if ((contents->size() % sizeof(char16_t)) != 0)
    {
        ADD_FAILURE() << "Saved console output has an incomplete UTF-16 code unit";
        return;
    }

    std::u16string utf16;
    utf16.reserve(contents->size() / sizeof(char16_t));
    for (std::size_t index = 0; index < contents->size(); index += sizeof(char16_t))
    {
        const auto low = std::to_integer<unsigned char>((*contents)[index]);
        const auto high = std::to_integer<unsigned char>((*contents)[index + 1]);
        utf16.push_back(static_cast<char16_t>(low | (static_cast<unsigned>(high) << 8)));
    }
    const std::string utf8 = una::utf16to8<char16_t, char>(utf16);
    contents->assign(reinterpret_cast<const std::byte*>(utf8.data()), reinterpret_cast<const std::byte*>(utf8.data() + utf8.size()));
}

void ConvertUtf8ToUtf16(std::vector<std::byte>* const contents)
{
    if (!g_integrationConfiguration.utf8Console || contents->empty())
    {
        return;
    }

    const std::string_view utf8(reinterpret_cast<const char*>(contents->data()), contents->size());
    const std::string utf16 = EncodeUtf16(utf8);
    contents->assign(reinterpret_cast<const std::byte*>(utf16.data()), reinterpret_cast<const std::byte*>(utf16.data() + utf16.size()));
}

std::string EncodeConsoleText(const std::string_view value)
{
    return g_integrationConfiguration.utf8Console ? std::string(value) : EncodeUtf16(value);
}

void ExpandStreamKeys(
    std::vector<std::byte>* const contents,
    const std::map<std::string, std::string>& replacements,
    const std::filesystem::path& workingDirectory)
{
    std::string text(reinterpret_cast<const char*>(contents->data()), contents->size());
    for (const auto& [key, configuredValue] : replacements)
    {
        ReplaceAll(&text, EncodeConsoleText(key), EncodeConsoleText(ResolveOutputValue(key, configuredValue, workingDirectory)));
    }
    contents->assign(reinterpret_cast<const std::byte*>(text.data()), reinterpret_cast<const std::byte*>(text.data() + text.size()));
}

void ContractStreamValues(
    std::vector<std::byte>* const contents,
    const std::map<std::string, std::string>& replacements,
    const std::filesystem::path& workingDirectory)
{
    std::string text(reinterpret_cast<const char*>(contents->data()), contents->size());
    for (const auto& [key, configuredValue] : replacements)
    {
        ReplaceAll(&text, EncodeConsoleText(ResolveOutputValue(key, configuredValue, workingDirectory)), EncodeConsoleText(key));
    }
    contents->assign(reinterpret_cast<const std::byte*>(text.data()), reinterpret_cast<const std::byte*>(text.data() + text.size()));
}

ProcessResult RunProcess(
    const std::filesystem::path& executable,
    const IntegrationTestCase& testCase,
    const std::filesystem::path& workingDirectory,
    const std::vector<std::byte>& standardInput)
{
    const std::filesystem::path standardInputPath = workingDirectory / ".makepri.stdin";
    const std::filesystem::path standardOutputPath = workingDirectory / ".makepri.stdout";
    const std::filesystem::path standardErrorPath = workingDirectory / ".makepri.stderr";
    WriteFileBytes(standardInputPath, standardInput);

    std::filesystem::current_path(workingDirectory);
    const std::string commandLine =
        MakeCommandLine(executable, testCase.arguments) + " < .makepri.stdin > .makepri.stdout 2> .makepri.stderr";
    ProcessResult result;
    const std::uint32_t systemResult = static_cast<std::uint32_t>(std::system(commandLine.c_str()));
    result.exitCode = g_integrationConfiguration.exitCodeBits == 32 ? systemResult : (systemResult >> 8) & ExitCodeMask();
    result.standardOutput = ReadFileBytes(standardOutputPath);
    result.standardError = ReadFileBytes(standardErrorPath);
    for (const std::filesystem::path& relativePath : testCase.outputFiles)
    {
        const std::filesystem::path outputPath = workingDirectory / relativePath;
        EXPECT_TRUE(std::filesystem::is_regular_file(outputPath)) << "Declared output was not created: " << relativePath.string();
        result.outputFiles.emplace(relativePath, ReadFileBytes(outputPath));
    }
    for (const std::filesystem::path& outputTextFile : testCase.outputTextFiles)
    {
        const std::filesystem::path outputPath = workingDirectory / outputTextFile;
        EXPECT_TRUE(std::filesystem::is_regular_file(outputPath)) << "Declared text output was not created: " << outputTextFile.string();
        result.outputTextFiles.emplace(outputTextFile, ReadFileText(outputPath));
    }
    return result;
}

std::vector<std::byte> ReadStandardInput(const IntegrationTestCase& testCase)
{
    const std::filesystem::path path = g_integrationConfiguration.inputRoot / testCase.name / "stdin.txt";
    if (!std::filesystem::is_regular_file(path))
    {
        return {};
    }
    return ReadFileBytes(path);
}

ProcessResult RunPrepared(
    const std::filesystem::path& executable,
    const IntegrationTestCase& testCase,
    const std::filesystem::path& workingDirectory,
    const std::vector<std::byte>& standardInput)
{
    std::filesystem::current_path(workingDirectory.parent_path());
    std::filesystem::remove_all(workingDirectory);
    std::filesystem::create_directories(workingDirectory);
    CopyInputs(testCase, workingDirectory);
    return RunProcess(executable, testCase, workingDirectory, standardInput);
}

void ExpectEqual(const ProcessResult& expected, const ProcessResult& actual, const std::string& expectedName, const std::string& actualName)
{
    EXPECT_EQ(actual.exitCode, expected.exitCode) << actualName << " versus " << expectedName;
    EXPECT_EQ(actual.standardOutput, expected.standardOutput) << actualName << " stdout versus " << expectedName;
    EXPECT_EQ(actual.standardError, expected.standardError) << actualName << " stderr versus " << expectedName;
    EXPECT_EQ(actual.outputFiles, expected.outputFiles) << actualName << " output files versus " << expectedName;
    EXPECT_EQ(actual.outputTextFiles, expected.outputTextFiles) << actualName << " text output files versus " << expectedName;
}

ProcessResult ReadSamples(const IntegrationTestCase& testCase, const std::filesystem::path& workingDirectory)
{
    const std::filesystem::path sampleDirectory = g_integrationConfiguration.outputRoot / testCase.name;
    ProcessResult result;
    const std::vector<std::byte> exitCodeBytes = ReadFileBytes(sampleDirectory / "exitcode.txt");
    std::string exitCodeText(reinterpret_cast<const char*>(exitCodeBytes.data()), exitCodeBytes.size());
    result.exitCode = static_cast<std::uint32_t>(std::stoul(exitCodeText)) & ExitCodeMask();
    result.standardOutput = ReadFileBytes(sampleDirectory / "stdout.txt");
    ConvertUtf16ToUtf8(&result.standardOutput);
    ExpandStreamKeys(&result.standardOutput, testCase.replacements, workingDirectory);
    result.standardError = ReadFileBytes(sampleDirectory / "stderr.txt");
    ConvertUtf16ToUtf8(&result.standardError);
    ExpandStreamKeys(&result.standardError, testCase.replacements, workingDirectory);
    for (const std::filesystem::path& relativePath : testCase.outputFiles)
    {
        result.outputFiles.emplace(relativePath, ReadFileBytes(sampleDirectory / relativePath));
    }
    for (const std::filesystem::path& outputTextFile : testCase.outputTextFiles)
    {
        std::string contents = ReadFileText(sampleDirectory / outputTextFile);
        ExpandOutputKeys(&contents, testCase.replacements, workingDirectory);
        result.outputTextFiles.emplace(outputTextFile, std::move(contents));
    }
    return result;
}

void WriteSamples(
    const IntegrationTestCase& testCase,
    const ProcessResult& result,
    const std::vector<std::byte>& standardInput,
    const std::filesystem::path& workingDirectory)
{
    const std::filesystem::path sampleDirectory = g_integrationConfiguration.outputRoot / testCase.name;
    std::filesystem::remove_all(sampleDirectory);
    std::filesystem::create_directories(sampleDirectory);
    const std::string exitCode = std::to_string(result.exitCode);
    WriteFileBytes(
        sampleDirectory / "exitcode.txt",
        std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(exitCode.data()), reinterpret_cast<const std::byte*>(exitCode.data() + exitCode.size())));
    WriteFileBytes(sampleDirectory / "stdin.txt", standardInput);
    std::vector<std::byte> standardOutput = result.standardOutput;
    ContractStreamValues(&standardOutput, testCase.replacements, workingDirectory);
    ConvertUtf8ToUtf16(&standardOutput);
    WriteFileBytes(sampleDirectory / "stdout.txt", standardOutput);
    std::vector<std::byte> standardError = result.standardError;
    ContractStreamValues(&standardError, testCase.replacements, workingDirectory);
    ConvertUtf8ToUtf16(&standardError);
    WriteFileBytes(sampleDirectory / "stderr.txt", standardError);
    for (const auto& [relativePath, contents] : result.outputFiles)
    {
        WriteFileBytes(sampleDirectory / relativePath, contents);
    }
    for (const std::filesystem::path& outputTextFile : testCase.outputTextFiles)
    {
        std::string contents = result.outputTextFiles.at(outputTextFile);
        ContractOutputValues(&contents, testCase.replacements, workingDirectory);
        WriteFileText(sampleDirectory / outputTextFile, contents);
    }
}

} // namespace

void VerifyIntegrationTest(const IntegrationTestCase& testCase)
{
    ASSERT_FALSE(g_integrationConfiguration.makePriUnderTest.empty());
    ASSERT_TRUE(std::filesystem::is_regular_file(g_integrationConfiguration.makePriUnderTest));
    ASSERT_FALSE(g_integrationConfiguration.inputRoot.empty());
    ASSERT_FALSE(g_integrationConfiguration.outputRoot.empty());
    if (!g_integrationConfiguration.officialMakePri.empty())
    {
        ASSERT_TRUE(std::filesystem::is_regular_file(g_integrationConfiguration.officialMakePri));
    }
    const std::vector<std::byte> standardInput = ReadStandardInput(testCase);
    const std::filesystem::path testDirectory = std::filesystem::current_path();
    const std::filesystem::path workingDirectory = testDirectory / "run";
    const ProcessResult actual = RunPrepared(g_integrationConfiguration.makePriUnderTest, testCase, workingDirectory, standardInput);

    if (!g_integrationConfiguration.officialMakePri.empty())
    {
        const ProcessResult official = RunPrepared(g_integrationConfiguration.officialMakePri, testCase, workingDirectory, standardInput);
        ExpectEqual(official, actual, "official MakePri", "MakePri under test");
        if (g_integrationConfiguration.updateOutputs)
        {
            WriteSamples(testCase, official, standardInput, workingDirectory);
        }
        else
        {
            ExpectEqual(ReadSamples(testCase, workingDirectory), official, "saved sample", "official MakePri");
        }
    }
    else
    {
        ASSERT_FALSE(g_integrationConfiguration.updateOutputs) << "The official executable is required to update integration samples";
        ExpectEqual(ReadSamples(testCase, workingDirectory), actual, "saved sample", "MakePri under test");
    }
}

} // namespace MakePri::Tests
