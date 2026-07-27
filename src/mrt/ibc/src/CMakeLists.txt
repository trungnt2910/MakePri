makepri_add_library(
    makepri_mrmidxbase
    STATIC
    CBootStrapIndexer.cpp
    CContentChecksumData.cpp
    CFsiFactory.cpp
    CHIndexerBase.cpp
    CIndexPass.cpp
    CItemInstanceSink.cpp
    CPackageInfo.cpp
    CQualifierApplicator.cpp
    CSchemaReader.cpp
    CUtilities.cpp
    CXmlHelper.cpp
    LogSink.cpp
)
add_library(makepri_resources_ibc ALIAS makepri_mrmidxbase)

target_link_libraries(makepri_mrmidxbase PRIVATE WIL::WIL makepri_mrmutils)
