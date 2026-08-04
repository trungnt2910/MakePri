makepri_add_library(
    makepri_makepricore
    STATIC
    FileOperations.cpp
    IndexOptions.cpp
    Log.cpp
    MakePriInternal.cpp
    MrmXml.cpp
    OptionsBase.cpp
    OutputOptions.cpp
    ParameterManager.cpp
)
add_library(makepri_resources_tools_makepri ALIAS makepri_makepricore)

target_link_libraries(
    makepri_makepricore
    PRIVATE #
        # Indexers
        makepri_embedfilesindexer
        makepri_folderindexer
        makepri_prifileindexer
        makepri_priinfoindexer
        makepri_resfilesindexer
        makepri_resjsonindexer
        makepri_resxindexer #
        # Other MRM deps
        makepri_langslib
        makepri_mrmbc
        makepri_mrmidxbase
        makepri_mrmstatus
        makepri_mrmruntimedlp #
        # Win32
        rpcrt4
        shlwapi
)

target_include_directories(
    makepri_makepricore
    PRIVATE "${PROJECT_SOURCE_DIR}/src/mrt/tools/makepri/res"
)

makepri_add_library(makepri_parameterparser OBJECT ParameterParser.cpp)
target_include_directories(
    makepri_parameterparser
    PRIVATE "${PROJECT_SOURCE_DIR}/src/mrt/tools/makepri/res"
)
target_link_libraries(makepri_parameterparser)

makepri_add_unicode_executable(
    makepri
    CIndexerTool.cpp
    $<TARGET_OBJECTS:makepri_parameterparser>
    "${PROJECT_SOURCE_DIR}/src/mrt/tools/makepri/res/makepri.rc"
)
target_include_directories(makepri PRIVATE "${PROJECT_SOURCE_DIR}/src/mrt/tools/makepri/res")
target_link_libraries(makepri PRIVATE makepri_makepricore ole32)

install(TARGETS makepri)
