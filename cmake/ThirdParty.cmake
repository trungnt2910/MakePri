include("${CMAKE_CURRENT_LIST_DIR}/GetCPM.cmake")

find_package(Git REQUIRED)

set(MAKEPRI_ORIGINAL_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
list(JOIN MAKEPRI_THIRD_PARTY_COMPILE_FLAGS " " makepri_third_party_compile_flags)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${makepri_third_party_compile_flags}")
set(MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS FALSE)

# ========================================================
# libxml2
# ========================================================
if(NOT WIN32)
    CPMAddPackage(
        NAME libxml2
        VERSION 2.15.3
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
        GIT_TAG v2.15.3
        OPTIONS
            "HAVE_DECL_GETENTROPY 0"
            "LIBXML2_WITH_ICONV FALSE"
            "LIBXML2_WITH_ICU FALSE"
            "LIBXML2_WITH_LZMA FALSE"
            "LIBXML2_WITH_MODULES FALSE"
            "LIBXML2_WITH_PROGRAMS FALSE"
            "LIBXML2_WITH_PYTHON FALSE"
            "LIBXML2_WITH_TESTS FALSE"
            "LIBXML2_WITH_THREADS FALSE"
            "LIBXML2_WITH_ZLIB FALSE"
    )
endif()

# ========================================================
# Boost
# ========================================================
if(NOT WIN32)
    string(
        CONCAT
        MAKEPRI_BOOST_URL
        "https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/"
        "boost-1.91.0-1-cmake.tar.xz"
    )
    CPMAddPackage(
        NAME Boost
        VERSION 1.91.0
        URL "${MAKEPRI_BOOST_URL}"
        URL_HASH SHA256=cc5dc5006ecbdf0051f90979be31b4eee5987d9ae14ae9fb9c03cfa43fa3cdad
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        EXCLUDE_FROM_ALL YES
        OPTIONS "BOOST_INCLUDE_LIBRARIES dll\\\;interprocess"
    )
    unset(MAKEPRI_BOOST_URL)
endif()

# ========================================================
# Unicode Algorithms Implementation
# ========================================================
CPMAddPackage(
    NAME uni-algo
    VERSION 1.2.0
    URL https://github.com/uni-algo/uni-algo/archive/refs/tags/v1.2.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS "UNI_ALGO_HEADER_ONLY TRUE"
)

# ========================================================
# Windows Implementation Library
# ========================================================
if(WIN32)
    CPMAddPackage(
        NAME wil
        VERSION 1.0.260126.7
        URL https://github.com/microsoft/wil/archive/refs/tags/v1.0.260126.7.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        EXCLUDE_FROM_ALL YES
        OPTIONS "WIL_BUILD_TESTS FALSE"
    )
    target_include_directories(WIL SYSTEM INTERFACE "$<BUILD_INTERFACE:${wil_SOURCE_DIR}/include>")
endif()

# ========================================================
# Windows App SDK
# ========================================================
CPMAddPackage(
    NAME windows_app_sdk
    GIT_REPOSITORY https://github.com/microsoft/WindowsAppSDK.git
    GIT_TAG v2.3.1
    GIT_SHALLOW TRUE
    PATCH_COMMAND
        "${GIT_EXECUTABLE}"
        apply
        "${CMAKE_SOURCE_DIR}/patches/MRTCore/6663.patch"
        "${CMAKE_SOURCE_DIR}/patches/MRTCore/6664.patch"
    SOURCE_SUBDIR makepri-no-add-subdirectory
)
include("${CMAKE_CURRENT_LIST_DIR}/MRTCore.cmake")
mrtcore_add_targets("${windows_app_sdk_SOURCE_DIR}")

# ========================================================
# GoogleTest
# ========================================================
if(BUILD_TESTING)
    CPMAddPackage(
        NAME googletest
        VERSION 1.14.0
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        EXCLUDE_FROM_ALL YES
    )
    target_compile_options(gtest PRIVATE -Wno-character-conversion)
endif()

set(BUILD_SHARED_LIBS "${MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS}")
unset(MAKEPRI_ORIGINAL_BUILD_SHARED_LIBS)
set(CMAKE_CXX_FLAGS "${MAKEPRI_ORIGINAL_CXX_FLAGS}")
unset(MAKEPRI_ORIGINAL_CXX_FLAGS)
