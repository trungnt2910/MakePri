# Custom wrappers ensure that project targets consistently use MakePri headers, warning flags,
# and static compiler runtimes. Third-party targets are deliberately unaffected.

function(_makepri_target_add_warning_flags target)
    target_compile_options(
        ${target}
        PRIVATE
            -Wall
            -Werror
            -Wextra
            -Wpedantic
            -Wno-language-extension-token
            -fms-extensions
    )
endfunction()

function(makepri_add_executable target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE ${MAKEPRI_INCLUDE_DIRECTORIES})
    target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(${target} PRIVATE MRTCore::mrmex)
    makepri_target_add_compat(${target})
    if(NOT MAKEPRI_SANITIZER_PREVENT_STATIC_LINKING)
        target_link_options(${target} PRIVATE -static)
    endif()
    _makepri_target_add_warning_flags(${target})
    makepri_target_add_pdb(${target})
    makepri_target_add_sanitizers(${target})
    makepri_target_install_sanitizers(${target})
endfunction()

function(makepri_add_library target type)
    add_library(${target} ${type} ${ARGN})
    target_include_directories(${target} PUBLIC ${MAKEPRI_INCLUDE_DIRECTORIES})
    target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(${target} PRIVATE MRTCore::mrmex)
    _makepri_target_add_warning_flags(${target})
    makepri_target_add_compat(${target})
    makepri_target_add_sanitizers(${target})
endfunction()

function(makepri_add_unicode_executable target)
    set(core_sources ${ARGN})
    list(FILTER core_sources EXCLUDE REGEX "\\.rc$")
    set(core_target "${target}_core")
    makepri_add_library(${core_target} STATIC ${core_sources})

    makepri_add_executable(${target})
    target_include_directories(
        ${core_target}
        PRIVATE "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>"
    )
    target_compile_definitions(
        ${core_target}
        PRIVATE "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>"
    )
    target_link_libraries(${target} PRIVATE ${core_target})

    makepri_target_embed_resources(${target} ${ARGN})
    makepri_add_unicode_compat(${target})
endfunction()
