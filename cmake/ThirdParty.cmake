include(FetchContent)

set(MAKEPRI_ORIGINAL_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
list(JOIN MAKEPRI_THIRD_PARTY_COMPILE_FLAGS " " makepri_third_party_compile_flags)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${makepri_third_party_compile_flags}")
set(MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")

# ========================================================
# libxml2
# ========================================================
if(NOT WIN32)
    FetchContent_Declare(
        libxml2
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
        GIT_TAG v2.15.3
        GIT_SHALLOW TRUE
    )
    set(BUILD_SHARED_LIBS OFF)
    set(HAVE_DECL_GETENTROPY 0 CACHE INTERNAL "" FORCE)
    set(LIBXML2_WITH_ICONV OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_ICU OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_LZMA OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_MODULES OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_THREADS OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_ZLIB OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libxml2)
endif()

# ========================================================
# Unicode Algorithms Implementation
# ========================================================
FetchContent_Declare(
    uni-algo
    URL https://github.com/uni-algo/uni-algo/archive/refs/tags/v1.2.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(UNI_ALGO_HEADER_ONLY ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(uni-algo)

# ========================================================
# Boost
# ========================================================
if(NOT WIN32)
    set(BOOST_INCLUDE_LIBRARIES dll interprocess)
    string(
        CONCAT
        MAKEPRI_BOOST_URL
        "https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/"
        "boost-1.91.0-1-cmake.tar.xz"
    )
    FetchContent_Declare(
        Boost
        URL "${MAKEPRI_BOOST_URL}"
        URL_HASH SHA256=cc5dc5006ecbdf0051f90979be31b4eee5987d9ae14ae9fb9c03cfa43fa3cdad
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(Boost)
    unset(MAKEPRI_BOOST_URL)
endif()

# ========================================================
# Windows Implementation Library
# ========================================================
if(WIN32)
    FetchContent_Declare(
        wil
        URL https://github.com/microsoft/wil/archive/refs/tags/v1.0.260126.7.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(WIL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(wil)
    target_include_directories(WIL SYSTEM INTERFACE "$<BUILD_INTERFACE:${wil_SOURCE_DIR}/include>")
endif()

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

set(BUILD_SHARED_LIBS "${MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS}")
unset(MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS)
set(CMAKE_CXX_FLAGS "${MAKEPRI_ORIGINAL_CXX_FLAGS}")
unset(MAKEPRI_ORIGINAL_CXX_FLAGS)
