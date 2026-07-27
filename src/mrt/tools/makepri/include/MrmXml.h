#pragma once

#include <DefStatus.h>
#include <MrtMap.h>
#include <ParameterManager.h>
#include <ParameterParser.h>

#include <windows.h>
#include <msxml6.h>

namespace Microsoft::Resources
{

class IHierarchicalSchema;
class IHierarchicalSchemaVersionInfo;
class IDecisionInfo;
class MrmProfile;
class NamedResourceResult;
class PriFile;
class QualifierResult;
class ResourceCandidateResult;
class ResourceMapBase;
class ResourceMapSubtree;
class StringResult;
class AtomPoolGroup;
class UnifiedEnvironment;

class PriConfigurationXml
{
public:
    static bool GeneratePriConfigToFile(
        MrmPlatformVersionInternal platformVersion,
        const wchar_t* outputFile,
        const wchar_t* defaultQualifiers,
        IDefStatusEx* status);

private:
    static bool GeneratePriConfig(
        MrmPlatformVersionInternal platformVersion,
        const wchar_t* defaultQualifiers,
        IDefStatusEx* status,
        IXMLDOMDocument2** document);
    static HRESULT GetDefaultQualifierMap(
        const MrmProfile* profile,
        const UnifiedEnvironment* environment,
        IDefStatusEx* status,
        Runtime::MrtMap<const wchar_t*, const wchar_t*>* qualifiers);
};

class StandalonePriFileXml
{
public:
    static bool DumpPriFileToXmlFile(
        const wchar_t* outputFileName,
        PriFile* priFile,
        Tools::MakePri::PriDumpType dumpType,
        const OutputOptions& outputOptions,
        IDefStatus* status);

private:
    static bool DumpPriFile(
        IXMLDOMDocument2* document,
        PriFile* priFile,
        Tools::MakePri::PriDumpType dumpType,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpSummaryXMLPriFile(IXMLDOMDocument2* document, PriFile* priFile, const OutputOptions& outputOptions, IDefStatus* status);
    static bool DumpResourceSchemaToXml(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const IHierarchicalSchema* schema,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpRecursiveSchemaSubTree(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const IHierarchicalSchema* schema,
        int scopeIndex,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedXMLPriFile(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        PriFile* priFile,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedQualifierInfo(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedQualifier(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        QualifierResult* qualifier,
        AtomPoolGroup* atomPoolGroup,
        bool,
        bool,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedResourceMap(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const ResourceMapBase* resourceMap,
        bool primary,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedRecursiveScopeTree(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const ResourceMapSubtree* resourceMapSubtree,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpDetailedItem(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        NamedResourceResult* resource,
        StringResult* resourceName,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpBasicXMLPriFile(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        PriFile* priFile,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpBasicQualifierSummary(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const IDecisionInfo* decisionInfo,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpBasicResourceMap(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const ResourceMapBase* resourceMapBase,
        bool primary,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpBasicRecursiveResourceMapSubTree(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const ResourceMapSubtree* subtree,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool DumpBasicNamedResource(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        NamedResourceResult* resource,
        AtomPoolGroup* atomPoolGroup,
        const OutputOptions& outputOptions,
        IDefStatus* status);
    static bool GetNamedResourceIsEmpty(NamedResourceResult* resource, const OutputOptions& outputOptions, IDefStatus* status);
    static bool GetSubtreeIsEmpty(const ResourceMapSubtree* subtree, const OutputOptions& outputOptions, IDefStatus* status);
    static bool GetRawLocatorText(ResourceCandidateResult* candidate, IDefStatus* status, StringResult* result);
    static bool ConstructUri(const wchar_t* authority, const wchar_t* path, IDefStatus* status, StringResult* result);
    static bool DumpDetailedHierarchicalSchemaVersionInfo(
        IXMLDOMDocument2* document,
        IXMLDOMNode* parent,
        const IHierarchicalSchemaVersionInfo* versionInfo,
        bool currentVersion,
        const OutputOptions& outputOptions,
        IDefStatus* status);
};

} // namespace Microsoft::Resources
