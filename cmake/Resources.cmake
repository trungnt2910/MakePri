if(MAKEPRI_ARCH STREQUAL "x86_64")
    set(MAKEPRI_RC_ARCH "AMD64")
elseif(MAKEPRI_ARCH STREQUAL "i686")
    set(MAKEPRI_RC_ARCH "IX86")
elseif(MAKEPRI_ARCH STREQUAL "aarch64")
    set(MAKEPRI_RC_ARCH "ARM64")
elseif(MAKEPRI_ARCH STREQUAL "armv7")
    set(MAKEPRI_RC_ARCH "ARM")
else()
    message(FATAL_ERROR "Unsupported resource compiler architecture: ${MAKEPRI_ARCH}")
endif()

if(NOT WIN32)
    string(REGEX MATCH "^[0-9]+" MAKEPRI_COMPILER_MAJOR "${CMAKE_CXX_COMPILER_VERSION}")
    find_program(MAKEPRI_LLVM_RC NAMES "llvm-rc-${MAKEPRI_COMPILER_MAJOR}" llvm-rc REQUIRED)
endif()

function(makepri_target_embed_resources target)
    set(resource_files ${ARGN})
    list(FILTER resource_files INCLUDE REGEX "\\.rc$")
    list(LENGTH resource_files resource_count)
    if(resource_count EQUAL 0)
        return()
    elseif(NOT resource_count EQUAL 1)
        message(FATAL_ERROR "${target} must contain exactly one Win32 resource script")
    endif()

    list(GET resource_files 0 resource_file)
    if(WIN32)
        target_sources(${target} PRIVATE "${resource_file}")
        return()
    endif()

    get_filename_component(resource_directory "${resource_file}" DIRECTORY)
    file(GLOB resource_dependencies CONFIGURE_DEPENDS "${resource_directory}/*")
    set(compiled_resource "${CMAKE_CURRENT_BINARY_DIR}/${target}.res")
    add_custom_command(
        OUTPUT "${compiled_resource}"
        COMMAND
            "${MAKEPRI_LLVM_RC}"
            /D_M_${MAKEPRI_RC_ARCH}
            "/FO${compiled_resource}"
            "/I${resource_directory}"
            "/I${MAKEPRI_COMPAT_SOURCE_DIR}/win32/include"
            "${resource_file}"
        DEPENDS ${resource_dependencies}
        VERBATIM
    )

    set(embedded_source "${CMAKE_CURRENT_BINARY_DIR}/${target}_resources.cpp")
    file(
        CONFIGURE
        OUTPUT "${embedded_source}"
        CONTENT
            [=[
            #include <cstddef>
            #include <span>

            #include "internal/resources.h"

            namespace
            {
            const unsigned char ResourceData[] = {
            #if defined(__clang__)
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wc23-extensions"
            #endif
            #embed "@compiled_resource@"
            #if defined(__clang__)
            #pragma clang diagnostic pop
            #endif
            };

            [[maybe_unused]] const bool ResourceRegistered = []
            {
                win32_compat::RegisterResourceData(std::span<const unsigned char>(ResourceData));
                return true;
            }();
            }
            ]=]
        @ONLY
    )
    add_custom_target(${target}_embedded_resource DEPENDS "${compiled_resource}")
    add_dependencies(compat_win32 ${target}_embedded_resource)
    target_sources(compat_win32 PRIVATE "${embedded_source}")
endfunction()
