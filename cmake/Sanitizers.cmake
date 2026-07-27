set(MAKEPRI_ASAN_ARCH "${MAKEPRI_ARCH}")
if(MAKEPRI_ASAN_ARCH STREQUAL "i686")
    set(MAKEPRI_ASAN_ARCH "i386")
endif()

set(MAKEPRI_ASAN_ARCH_SUPPORTED TRUE)
if(MAKEPRI_ASAN_ARCH MATCHES "armv7|aarch64")
    set(MAKEPRI_ASAN_ARCH_SUPPORTED FALSE)
endif()

set(MAKEPRI_SANITIZER_DLLS "")
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND MAKEPRI_ASAN_ARCH_SUPPORTED)
    list(
        APPEND
        MAKEPRI_SANITIZER_DLLS
        "libc++.dll"
        "libclang_rt.asan_dynamic-${MAKEPRI_ASAN_ARCH}.dll"
        "libunwind.dll"
    )
endif()

set(MAKEPRI_SANITIZER_PREVENT_STATIC_LINKING FALSE)
set(MAKEPRI_SANITIZER_FLAGS "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MAKEPRI_ASAN_ARCH_SUPPORTED)
        list(APPEND MAKEPRI_SANITIZER_FLAGS -fsanitize=address)
        if(NOT WIN32)
            set(MAKEPRI_SANITIZER_PREVENT_STATIC_LINKING TRUE)
        endif()
    endif()
    list(APPEND MAKEPRI_SANITIZER_FLAGS -fsanitize=undefined)
endif()

function(makepri_target_add_sanitizers target)
    if(MAKEPRI_SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${MAKEPRI_SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${MAKEPRI_SANITIZER_FLAGS})
    endif()
endfunction()

function(makepri_target_install_sanitizers target)
    foreach(dll_name IN LISTS MAKEPRI_SANITIZER_DLLS)
        execute_process(
            COMMAND
                "${CMAKE_CXX_COMPILER}"
                "--print-file-name=${MAKEPRI_RUNTIME_TARGET}/bin/${dll_name}"
            OUTPUT_VARIABLE dll_path
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(EXISTS "${dll_path}")
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND
                    "${CMAKE_COMMAND}"
                    -E
                    copy_if_different
                    "${dll_path}"
                    "$<TARGET_FILE_DIR:${target}>/${dll_name}"
            )
            set_property(
                TARGET ${target}
                APPEND
                PROPERTY
                    ADDITIONAL_CLEAN_FILES
                    "$<TARGET_FILE_DIR:${target}>/${dll_name}"
            )
            install(FILES "${dll_path}" DESTINATION bin)
        endif()
    endforeach()
endfunction()

list(APPEND MAKEPRI_THIRD_PARTY_COMPILE_FLAGS ${MAKEPRI_SANITIZER_FLAGS})
