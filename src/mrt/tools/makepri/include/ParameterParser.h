#pragma once

#include <cstddef>
#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Collections.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/Checksums.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/platform/base.h>

#include <windows.h>

namespace Microsoft::Resources::Tools::MakePri
{

enum class UsageScenario : std::int32_t
{
    None = 0,
    New = 1,
    Versioned = 2,
    ResourcePack = 3,
    Dump = 4,
    CreateConfig = 5,
};

enum class PriDumpType : std::int32_t
{
    Basic = 0,
    Detailed = 1,
    Schema = 2,
    Summary = 3,
};

enum class MappingFileFormat : std::int32_t
{
    NoMapping = 0,
    AppX = 1,
};

// This field order is the InputArgs layout copied by ParameterManager's constructor. The
// original x86 structure is 88 bytes. Pointer alignment naturally changes its size on 64-bit
// targets, which is intentional.
struct InputArgs
{
    bool help {};
    bool overwrite {};
    bool verbose {};
    bool autoMerge {};
    bool diagnosticLogging {};
    bool reverseMap {};
    std::byte reserved[2] {};
    const wchar_t* projectRoot {};
    const wchar_t* outputFile {};
    const wchar_t* configXml {};
    const wchar_t* indexLog {};
    const wchar_t* indexName {};
    const wchar_t* manifest {};
    std::int32_t versionMajor {};
    const wchar_t* indexFile {};
    PriDumpType dumpType {};
    const wchar_t* defaultQualifiers {};
    MappingFileFormat mappingFileFormat {};
    UsageScenario scenario {};
    const wchar_t* schemaFile {};
    const wchar_t* extensionDll {};
    const wchar_t* externalSchema {};
    MrmPlatformVersionInternal platformVersion {};
    const wchar_t* outputOptions {};
    const wchar_t* indexOptions {};
    bool outputOptionsError {};
    bool indexOptionsError {};
    std::byte reservedBeforeConflict[2] {};
    std::uint32_t contentChecksumValue {};
};

static_assert(sizeof(void*) != 4 || sizeof(InputArgs) == 88);
static_assert(offsetof(InputArgs, projectRoot) == 8);

class ParameterParser
{
public:
    explicit ParameterParser(InputArgs& inputArgs);

    [[nodiscard]] HRESULT Parse(int argc, wchar_t* const argv[]);

private:
    [[nodiscard]] HRESULT ParseOptions(int argc, wchar_t* const argv[]);
    [[nodiscard]] HRESULT SetTrueOnce(bool& value);
    [[nodiscard]] HRESULT AssignScenarioOnce(UsageScenario scenario);
    [[nodiscard]] HRESULT AssignDumpTypeOnce(const wchar_t* value);
    [[nodiscard]] HRESULT AssignPlatformVersionOnce(const wchar_t* value, MrmPlatformVersionInternal* platformVersion);

    InputArgs& m_inputArgs;
};

[[nodiscard]] bool StringsEqual(const wchar_t* left, const wchar_t* right);

} // namespace Microsoft::Resources::Tools::MakePri

extern const wchar_t* g_temporaryFolder;

BOOL WINAPI MakepriCtrlHandler(DWORD controlType);
HRESULT MakePriCreateConfigInternal(Microsoft::Resources::Tools::MakePri::InputArgs* inputArgs);
HRESULT MakePriDumpInternal(Microsoft::Resources::Tools::MakePri::InputArgs* inputArgs, const wchar_t** outputFile);
HRESULT MakePriNewVersionedPack(
    Microsoft::Resources::Tools::MakePri::InputArgs* inputArgs,
    Microsoft::Resources::Tools::MakePri::UsageScenario scenario);
