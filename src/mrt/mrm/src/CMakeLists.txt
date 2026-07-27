makepri_add_library(
    makepri_mrmclientdl
    STATIC
    ClientProfileBase.cpp
    DownlevelProfiles.cpp
    LegacyApps.cpp
)

makepri_add_library(makepri_mrmbc STATIC MrmProfile.cpp QualifierTypes.cpp)

add_library(makepri_resources_mrm INTERFACE)
target_link_libraries(makepri_resources_mrm INTERFACE makepri_mrmclientdl makepri_mrmbc)

target_link_libraries(
    makepri_mrmclientdl
    PRIVATE
        makepri_mrmbc
        MRTCore::mrmex
        MRTCore::mrmmin
        WIL::WIL
)

target_link_libraries(
    makepri_mrmbc
    PRIVATE
        makepri_mrmclientdl
        MRTCore::mrmex
        MRTCore::mrmmin
        WIL::WIL
)
