makepri_add_library(
    makepri_folderindexer
    STATIC
    CFIXmlConfig.cpp
    CFolderIndexer.cpp
    CFolderInfo.cpp
)
add_library(makepri_resources_fsi_folder ALIAS makepri_folderindexer)

target_link_libraries(makepri_folderindexer PRIVATE WIL::WIL)
