function(_mrtcore_target_options target)
    target_compile_definitions(
        ${target}
        PUBLIC #
            # SAL
            # The raw definitions are not available in MinGW.
            # The public headers are polluted with this.
            __deref_opt_out_opt=SAL__deref_opt_out_opt
            __deref_out_bcount=SAL__deref_out_bcount
            __field_ecount=_Field_size_
            __in=SAL__in
            __in_bcount_opt=SAL__in_bcount_opt
            __in_ecount_opt=SAL__in_ecount_opt
            __out=SAL__out
            __out_ecount_opt=SAL__out_ecount_opt
        PRIVATE #
            # Built-in
            _LIB
            _UNICODE
            _USRDLL
            _WINDOWS
            UNICODE #
            # Missing in MinGW
            ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE=15141L
            ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE=15142L
            ERROR_PRI_MERGE_VERSION_MISMATCH=15148L
            InterlockedExchangeNoFence=_InterlockedExchange
            SIZE_T_MAX=SIZE_MAX
    )
    target_compile_options(
        ${target}
        PRIVATE
            -fms-extensions
            -w
            "SHELL:-include type_traits"
            "-Dmin(a,b)=(((a)<(b))?(a):(b))"
            "-Dmax(a,b)=(((a)>(b))?(a):(b))"
    )
    target_include_directories(
        ${target}
        SYSTEM
        PUBLIC "${CMAKE_SOURCE_DIR}/src/compat/win32/include"
    )
endfunction()

function(mrtcore_add_targets windows_app_sdk_source_dir)
    set(mrtcore_source_dir "${windows_app_sdk_source_dir}/dev/MRTCore/mrt")
    set(mrm_source_dir "${mrtcore_source_dir}/mrm")

    add_library(
        mrtcore_min
        STATIC
        "${mrm_source_dir}/mrmmin/Atom.cpp"
        "${mrm_source_dir}/mrmmin/AtomGroup.cpp"
        "${mrm_source_dir}/mrmmin/BaseFile.cpp"
        "${mrm_source_dir}/mrmmin/BaseProviders.cpp"
        "${mrm_source_dir}/mrmmin/BaseQualifierTypes.cpp"
        "${mrm_source_dir}/mrmmin/BlobResult.cpp"
        "${mrm_source_dir}/mrmmin/BlobResultImpl.cpp"
        "${mrm_source_dir}/mrmmin/Checksums.cpp"
        "${mrm_source_dir}/mrmmin/CoreEnvironment.cpp"
        "${mrm_source_dir}/mrmmin/CoreProfile.cpp"
        "${mrm_source_dir}/mrmmin/CoreQualifierTypes.cpp"
        "${mrm_source_dir}/mrmmin/DecisionInfo.cpp"
        "${mrm_source_dir}/mrmmin/DecisionInfoBuilder.cpp"
        "${mrm_source_dir}/mrmmin/DefObject.cpp"
        "${mrm_source_dir}/mrmmin/Environment.cpp"
        "${mrm_source_dir}/mrmmin/FileAtomPool.cpp"
        "${mrm_source_dir}/mrmmin/FileDataSection.cpp"
        "${mrm_source_dir}/mrmmin/FileFileList.cpp"
        "${mrm_source_dir}/mrmmin/HNames.cpp"
        "${mrm_source_dir}/mrmmin/HSchema.cpp"
        "${mrm_source_dir}/mrmmin/ManagedFiles.cpp"
        "${mrm_source_dir}/mrmmin/Managers.cpp"
        "${mrm_source_dir}/mrmmin/MrmFile.cpp"
        "${mrm_source_dir}/mrmmin/MrmTraceLogging.cpp"
        "${mrm_source_dir}/mrmmin/Platform.cpp"
        "${mrm_source_dir}/mrmmin/PriFile.cpp"
        "${mrm_source_dir}/mrmmin/PriFileManager.cpp"
        "${mrm_source_dir}/mrmmin/RemapInfo.cpp"
        "${mrm_source_dir}/mrmmin/Resolvers.cpp"
        "${mrm_source_dir}/mrmmin/ResourceLink.cpp"
        "${mrm_source_dir}/mrmmin/ResourceMap.cpp"
        "${mrm_source_dir}/mrmmin/ReverseMap.cpp"
        "${mrm_source_dir}/mrmmin/RtlProfile.cpp"
        "${mrm_source_dir}/mrmmin/SchemaCollection.cpp"
        "${mrm_source_dir}/mrmmin/StaticAtomPool.cpp"
        "${mrm_source_dir}/mrmmin/StringResult.cpp"
        "${mrm_source_dir}/mrmmin/StringResultImpl.cpp"
        "${mrm_source_dir}/mrmmin/UnifiedView.cpp"
        "${mrm_source_dir}/mrmmin/Util.cpp"
    )
    add_library(MRTCore::mrmmin ALIAS mrtcore_min)
    target_include_directories(
        mrtcore_min
        SYSTEM
        PUBLIC
            "${mrm_source_dir}/include"
            "${mrm_source_dir}"
            "${windows_app_sdk_source_dir}/dev/WindowsAppRuntime_Insights"
            "${windows_app_sdk_source_dir}/dev/common"
        PRIVATE "${mrm_source_dir}/mrmmin"
    )
    target_link_libraries(mrtcore_min PUBLIC WIL::WIL)
    _mrtcore_target_options(mrtcore_min)

    add_library(
        mrtcore_ex
        STATIC
        "${mrm_source_dir}/mrmex/AIDict.cpp"
        "${mrm_source_dir}/mrmex/DataItemOrchestrator.cpp"
        "${mrm_source_dir}/mrmex/DataSectionBuilder.cpp"
        "${mrm_source_dir}/mrmex/EnvironmentEx.cpp"
        "${mrm_source_dir}/mrmex/EnvironmentMappingBuilder.cpp"
        "${mrm_source_dir}/mrmex/FileAtomPoolBuilder.cpp"
        "${mrm_source_dir}/mrmex/FileBuilder.cpp"
        "${mrm_source_dir}/mrmex/FileListBuilder.cpp"
        "${mrm_source_dir}/mrmex/HNamesBuilder.cpp"
        "${mrm_source_dir}/mrmex/HSchemaBuilder.cpp"
        "${mrm_source_dir}/mrmex/InstanceReferences.cpp"
        "${mrm_source_dir}/mrmex/LinkBuilder.cpp"
        "${mrm_source_dir}/mrmex/MapBuilder.cpp"
        "${mrm_source_dir}/mrmex/PriMerge.cpp"
        "${mrm_source_dir}/mrmex/PriSectionBuilder.cpp"
        "${mrm_source_dir}/mrmex/References.cpp"
        "${mrm_source_dir}/mrmex/ResourcePackMerge.cpp"
        "${mrm_source_dir}/mrmex/ReverseMapBuilder.cpp"
        "${mrm_source_dir}/mrmex/SectionCopier.cpp"
        "${mrm_source_dir}/mrmex/SectionCopierFactory.cpp"
        "${mrm_source_dir}/mrmex/SectionCopiers.cpp"
        "${mrm_source_dir}/mrmex/StdAfx.cpp"
    )
    add_library(MRTCore::mrmex ALIAS mrtcore_ex)
    target_include_directories(
        mrtcore_ex
        SYSTEM
        PUBLIC
            "${mrm_source_dir}/include"
            "${windows_app_sdk_source_dir}/dev/WindowsAppRuntime_Insights"
            "${windows_app_sdk_source_dir}/dev/common"
        PRIVATE "${mrm_source_dir}/mrmex"
    )
    target_link_libraries(mrtcore_ex PUBLIC MRTCore::mrmmin WIL::WIL)
    _mrtcore_target_options(mrtcore_ex)
endfunction()
