#pragma once

namespace Microsoft::Resources
{
class IDefStatusEx;
class ISchemaCollection;
} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{
class IIndexOptions
{
public:
    [[nodiscard]] virtual bool GetShouldProcessHiddenFiles() const = 0;
    [[nodiscard]] virtual bool GetShouldProcessLinkedFiles() const = 0;
    [[nodiscard]] virtual bool GetShouldBuildAutoMergePri() const = 0;
    [[nodiscard]] virtual bool GetShouldBuildDeploymentMergeablePri() const = 0;
    [[nodiscard]] virtual bool GetShouldValidateDefaultQualifiers() const = 0;
    [[nodiscard]] virtual bool GetShouldSuppressConsoleOutput() const = 0;
    [[nodiscard]] virtual bool GetShouldSuppressEmbeddedData() const = 0;
    [[nodiscard]] virtual bool GetShouldGenerateReverseMap() const = 0;
    [[nodiscard]] virtual bool GetShouldUseOptimizedEncoding() const = 0;
    [[nodiscard]] virtual bool GetShouldOmitSchemaFromResourcePacks() const = 0;
    [[nodiscard]] virtual bool GetShouldUseGranularResourceSplitting() const = 0;
    [[nodiscard]] virtual bool GetShouldSplitLanguageVariants() const = 0;
    [[nodiscard]] virtual bool GetShouldCreateContentChecksum() const = 0;
    [[nodiscard]] virtual const wchar_t* GetIndexerSchemaPath() const = 0;
    [[nodiscard]] virtual const ISchemaCollection* GetIndexerSchemaCollection(IDefStatusEx* status) const = 0;
    [[nodiscard]] virtual bool GetShouldDisableDeduplication() const = 0;
};
} // namespace Microsoft::Resources::Indexers
