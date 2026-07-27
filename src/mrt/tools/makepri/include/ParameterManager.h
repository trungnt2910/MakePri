#pragma once

#include <ParameterParser.h>
#include <DefStatus.h>
#include <IIndexOptions.h>
#include <SchemaReader.h>

#include <windows.h>

#include <cstdint>
#include <list>
#include <string>

namespace Microsoft::Resources
{

class OptionsBase
{
public:
    struct OptionSpec
    {
        const wchar_t* longName;
        const wchar_t* shortName;
        std::uint64_t mask;
        std::uint64_t positiveValue;
        std::uint64_t negativeValue;
    };

    struct StringOptionSpec
    {
        const wchar_t* longName;
        const wchar_t* shortName;
    };

    virtual ~OptionsBase();

    [[nodiscard]] virtual bool Initialize(const wchar_t* options, IDefStatusEx* status);
    [[nodiscard]] std::uint64_t GetFlags() const { return m_flags; }

protected:
    OptionsBase(
        std::uint64_t ignoredInitialFlags,
        std::uint32_t optionCount,
        const OptionSpec* options,
        std::uint32_t stringOptionCount,
        const StringOptionSpec* stringOptions);

    virtual bool VerifyStringOptions(IDefStatusEx* status);

    [[nodiscard]] bool ParseOptionsString(
        const wchar_t* options,
        std::uint32_t valueCount,
        IDefStatusEx* status,
        std::uint64_t* flags,
        wchar_t** values);

    std::uint64_t m_flags {};
    std::uint64_t m_unknown {};
    std::uint32_t m_optionCount {};
    const OptionSpec* m_options {};
    std::uint32_t m_stringOptionCount {};
    const StringOptionSpec* m_stringOptions {};
    wchar_t** m_stringValues {};
};

template<typename TFlags>
class TOptionsBase : public OptionsBase
{
public:
    [[nodiscard]] TFlags GetFlags() const { return static_cast<TFlags>(OptionsBase::GetFlags()); }

protected:
    TOptionsBase(
        std::uint64_t initialFlags,
        std::uint32_t optionCount,
        const OptionSpec* options,
        std::uint32_t stringOptionCount,
        const StringOptionSpec* stringOptions) :
        OptionsBase(initialFlags, optionCount, options, stringOptionCount, stringOptions)
    {}
};

class OutputOptions final : public OptionsBase
{
public:
    OutputOptions();
    ~OutputOptions() override = default;

private:
    static const OptionSpec s_options[14];
};

enum IndexOptionFlags : std::uint64_t;

class IndexOptions final : public TOptionsBase<IndexOptionFlags>, public Indexers::IIndexOptions
{
public:
    IndexOptions();
    ~IndexOptions() override;

    [[nodiscard]] bool GetShouldProcessHiddenFiles() const override;
    [[nodiscard]] bool GetShouldProcessLinkedFiles() const override;
    [[nodiscard]] bool GetShouldBuildAutoMergePri() const override;
    [[nodiscard]] bool GetShouldBuildDeploymentMergeablePri() const override;
    [[nodiscard]] bool GetShouldValidateDefaultQualifiers() const override;
    [[nodiscard]] bool GetShouldSuppressConsoleOutput() const override;
    [[nodiscard]] bool GetShouldSuppressEmbeddedData() const override;
    [[nodiscard]] bool GetShouldGenerateReverseMap() const override;
    [[nodiscard]] bool GetShouldUseOptimizedEncoding() const override;
    [[nodiscard]] bool GetShouldOmitSchemaFromResourcePacks() const override;
    [[nodiscard]] bool GetShouldDisableDeduplication() const override;
    [[nodiscard]] bool GetShouldUseGranularResourceSplitting() const override;
    [[nodiscard]] bool GetShouldSplitLanguageVariants() const override;
    [[nodiscard]] bool GetShouldCreateContentChecksum() const override;

    [[nodiscard]] const wchar_t* GetIndexerSchemaPath() const override;
    [[nodiscard]] const ISchemaCollection* GetIndexerSchemaCollection(IDefStatusEx* status) const override;
    bool SetIndexerSchemaPath(const wchar_t* path, IDefStatusEx* status);

protected:
    bool VerifyStringOptions(IDefStatusEx* status) override;

private:
    friend auto ::MakePriNewVersionedPack(Tools::MakePri::InputArgs* inputArgs, Tools::MakePri::UsageScenario scenario) -> HRESULT;

    [[nodiscard]] bool HasFlag(std::uint64_t flag) const;

    static const OptionSpec s_options[4];
    static const StringOptionSpec s_stringOptions[1];

    StringResult m_indexerSchemaPath;
    Indexers::PathSchemaCollection* m_indexerSchemaCollection {};
};

static_assert(sizeof(void*) != 4 || sizeof(IndexOptions) == 72);

} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{

class IFormatSpecificIndexer;

}

namespace Microsoft::Resources::Tools::MakePri
{

class FileOperations
{
public:
    FileOperations(const wchar_t* sourceFolder, const wchar_t* destinationFolder) :
        m_sourceFolder(sourceFolder), m_destinationFolder(destinationFolder)
    {}

    HRESULT MoveFiles(bool overwrite, IDefStatusEx* status);

    static HRESULT s_DeleteFolderAndContents(const wchar_t* path, IDefStatusEx* status = nullptr);
    static HRESULT s_CreateUniqueTempFolder(IDefStatusEx* status, StringResult* result);

private:
    HRESULT _AskOverwritePermission(const std::list<std::wstring>& files);
    HRESULT _GetListOfFilesToOverwrite(const std::list<std::wstring>& sourceFiles, IDefStatusEx* status, std::list<std::wstring>& files);
    HRESULT _GetListOfSourceFiles(IDefStatusEx* status, std::list<std::wstring>& files);
    HRESULT _MoveAllFiles(const std::list<std::wstring>& files, IDefStatusEx* status);
    HRESULT _MoveFile(const wchar_t* source, const wchar_t* destination, IDefStatusEx* status);

    const wchar_t* m_sourceFolder;
    const wchar_t* m_destinationFolder;
};

class InputParams
{
public:
    ~InputParams();

protected:
    explicit InputParams(InputArgs inputArgs) : m_inputArgs(inputArgs), m_mappingFileFormat(inputArgs.mappingFileFormat) {}

    InputArgs m_inputArgs;
    MappingFileFormat m_mappingFileFormat;
    OutputOptions m_outputOptions;
    IndexOptions m_indexOptions;
};

static_assert(sizeof(void*) != 4 || sizeof(InputParams) == 216);

class PathHandler
{
public:
    enum class PathFlags : std::uint8_t
    {
        None = 0,
        AllowDirectory = 1,
        Overwrite = 2,
    };

    PathHandler() = default;
    ~PathHandler();

    HRESULT InitializeForInputFolder(const wchar_t* path);
    HRESULT InitializeForInputFile(const wchar_t* path, const wchar_t* extension);
    HRESULT InitializeForInputFile(const wchar_t* path, std::uint32_t extensionCount, const wchar_t* const* extensions);
    HRESULT InitializeForOutputPath(const wchar_t* path, const wchar_t* extension, const wchar_t* defaultFileName, PathFlags flags);

    [[nodiscard]] const wchar_t* GetFileNameNoExt();
    [[nodiscard]] const wchar_t* GetFullPath() const { return m_fullPath.c_str(); }
    [[nodiscard]] const wchar_t* GetFileName() const { return m_fileName.c_str(); }
    [[nodiscard]] const wchar_t* GetFolderPath() const { return m_folderPath.c_str(); }
    [[nodiscard]] bool IsInitialized() const { return m_initialized; }

    static HRESULT CombinePath(const wchar_t* parent, const wchar_t* child, std::wstring& result);

private:
    enum class FileState
    {
        DoesNotExist,
        Directory,
        File,
    };

    HRESULT _VerifyParentFolderExists(std::wstring& path);
    void _GetFileExtension(std::wstring& path, std::wstring& extension);
    HRESULT _GetFileState(const wchar_t* path, FileState& state);
    void _SeparateParentFolderAndFile(std::wstring& path);
    HRESULT _HandleOverwrite(PathFlags flags, const wchar_t* path);
    HRESULT _VerifyPathString(std::wstring& path);
    void _AdjustSlashes(std::wstring& path);

    bool m_hadTrailingSlash {};
    bool m_initialized {};
    std::wstring m_fullPath;
    std::wstring m_fileName;
    std::wstring m_fileNameNoExt;
    std::wstring m_folderPath;
};

class ParameterManager : private InputParams
{
public:
    explicit ParameterManager(InputArgs inputArgs);
    ~ParameterManager() = default;

    [[nodiscard]] const wchar_t* GetIndexFile() const;
    [[nodiscard]] const wchar_t* GetOutputFolder() const;
    [[nodiscard]] const wchar_t* GetOutputFile() const
    {
        const wchar_t* const path = m_outputPath.GetFullPath();
        return path[0] == L'\0' ? nullptr : path;
    }

    [[nodiscard]] const wchar_t* GetSchemaFile() const
    {
        const wchar_t* const path = m_schemaFilePath.GetFullPath();
        return path[0] == L'\0' ? nullptr : path;
    }

    [[nodiscard]] const wchar_t* GetExternalSchemaFile() const
    {
        const wchar_t* const path = m_externalSchemaPath.GetFullPath();
        return path[0] == L'\0' ? nullptr : path;
    }

    [[nodiscard]] const OutputOptions& GetOutputOptions() const { return m_outputOptions; }

    [[nodiscard]] const wchar_t* GetConfigXmlFile() const
    {
        const wchar_t* const path = m_configXmlPath.GetFullPath();
        return path[0] == L'\0' ? nullptr : path;
    }

    [[nodiscard]] const InputArgs& GetInputArgs() const { return m_inputArgs; }

    [[nodiscard]] HRESULT VerifyParams();

private:
    friend auto ::MakePriNewVersionedPack(InputArgs* inputArgs, UsageScenario scenario) -> HRESULT;

    [[nodiscard]] HRESULT InitializeInputIndexFile();
    [[nodiscard]] HRESULT GetIndexNameFromAppxManifest();

    OutputOptions m_secondaryOutputOptions;
    IndexOptions m_secondaryIndexOptions;
    std::list<std::wstring> m_strings;
    std::list<Indexers::IFormatSpecificIndexer*> m_indexers;
    std::list<Indexers::IFormatSpecificIndexer*> m_formatSpecificIndexers;
    std::wstring m_indexName;
    PathHandler m_manifestPath;
    PathHandler m_inputIndexFile;
    PathHandler m_configXmlPath;
    PathHandler m_projectRootPath;
    PathHandler m_extensionDllPath;
    PathHandler m_externalSchemaPath;
    PathHandler m_unknownPath;
    PathHandler m_indexLogPath;
    PathHandler m_outputPath;
    PathHandler m_schemaFilePath;
};

} // namespace Microsoft::Resources::Tools::MakePri
