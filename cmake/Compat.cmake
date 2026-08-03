# Compatibility targets remain isolated from the project target helpers. This module owns the
# non-Windows implementations and injects platform compatibility into targets created elsewhere.

set(MAKEPRI_COMPAT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/compat")
add_subdirectory("${MAKEPRI_COMPAT_SOURCE_DIR}")

function(_makepri_finalize_compat_link_group target)
    get_target_property(libraries ${target} LINK_LIBRARIES)
    if(libraries)
        set(end_marker "${target}_makepri_link_group_end")
        add_library(${end_marker} INTERFACE)
        target_link_libraries(${end_marker} INTERFACE "-Wl,--end-group")
        set_property(
            TARGET ${target}
            PROPERTY
                LINK_LIBRARIES
                "-Wl,--start-group;${libraries};${end_marker}"
        )
    endif()
endfunction()

function(_makepri_target_add_wchar_wrappers target)
    set(
        wrapped_functions
        fclose
        fwprintf
        fgetwc
        wcscasecmp
        wcschr
        wcscmp
        wcscspn
        wcslen
        wcsncmp
        wcsnlen
        wcsrchr
        wcsstr
        wmemcmp
        wmemchr
        wmemcpy
        wmemmove
        wmemset
        wprintf
        vswprintf
    )
    foreach(wrapped_function IN LISTS wrapped_functions)
        target_link_options(${target} PRIVATE "LINKER:--wrap=${wrapped_function}")
    endforeach()
endfunction()

function(makepri_target_add_compat target)
    target_compile_options(${target} PRIVATE -fshort-wchar)
    target_include_directories(
        ${target}
        SYSTEM
        BEFORE
        PRIVATE "${MAKEPRI_COMPAT_SOURCE_DIR}/win32/include"
    )
    if(WIN32)
        return()
    endif()

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        target_link_libraries(${target} PRIVATE compat_wchar)
        _makepri_target_add_wchar_wrappers(${target})
        cmake_language(
            EVAL
            CODE
            "cmake_language(DEFER CALL _makepri_finalize_compat_link_group ${target})"
        )
    endif()
endfunction()

function(makepri_add_unicode_compat target)
    if(WIN32)
        target_link_options(${target} PRIVATE -municode)
        return()
    endif()

    target_sources(${target} PRIVATE "${MAKEPRI_COMPAT_SOURCE_DIR}/wchar/src/main.cpp")
    target_link_libraries(${target} PRIVATE uni-algo::uni-algo)
endfunction()
