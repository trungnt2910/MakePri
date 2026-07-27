include(FetchContent)

set(MAKEPRI_ORIGINAL_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
list(JOIN MAKEPRI_THIRD_PARTY_COMPILE_FLAGS " " makepri_third_party_compile_flags)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${makepri_third_party_compile_flags}")

# ========================================================
# Windows Implementation Library
# ========================================================
FetchContent_Declare(
    wil
    URL https://github.com/microsoft/wil/archive/refs/tags/v1.0.260126.7.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(WIL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(wil)
target_include_directories(WIL SYSTEM INTERFACE "$<BUILD_INTERFACE:${wil_SOURCE_DIR}/include>")

# ========================================================
# Windows App SDK
# ========================================================
FetchContent_Declare(
    windows_app_sdk #
    # TODO: Switch to an official tag again once these PRs are merged:
    # https://github.com/microsoft/WindowsAppSDK/pull/6663
    # https://github.com/microsoft/WindowsAppSDK/pull/6664
    GIT_REPOSITORY https://github.com/trungnt2910/WindowsAppSDK.git
    GIT_TAG 55a9198c1d61cc6a0253131d791d3ecdec498164
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR makepri-no-add-subdirectory
)
FetchContent_MakeAvailable(windows_app_sdk)
include("${CMAKE_CURRENT_LIST_DIR}/MRTCore.cmake")
mrtcore_add_targets("${windows_app_sdk_SOURCE_DIR}")

# ========================================================
# GoogleTest
# ========================================================
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_MakeAvailable(googletest)
    target_compile_options(gtest PRIVATE -Wno-character-conversion)
endif()

set(CMAKE_CXX_FLAGS "${MAKEPRI_ORIGINAL_CXX_FLAGS}")
unset(MAKEPRI_ORIGINAL_CXX_FLAGS)
