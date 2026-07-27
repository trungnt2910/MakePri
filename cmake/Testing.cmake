include(GoogleTest)

function(makepri_add_test_executable target)
    makepri_add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)
endfunction()

set(
    MAKEPRI_TEST_OFFICIAL_EXECUTABLE
    ""
    CACHE
        FILEPATH
        "Official MakePri executable used by integration tests"
)
option(
    MAKEPRI_UPDATE_INTEGRATION_OUTPUTS
    "Update integration samples from the official MakePri executable"
    OFF
)

function(makepri_add_integration_test_executable target)
    add_executable(
        ${target}
        "${PROJECT_SOURCE_DIR}/tests/integration/IntegrationMain.cpp"
        "${PROJECT_SOURCE_DIR}/tests/integration/IntegrationTest.cpp"
        ${ARGN}
    )
    if(NOT MAKEPRI_SANITIZER_PREVENT_STATIC_LINKING)
        target_link_options(${target} PRIVATE -static)
    endif()
    _makepri_target_add_warning_flags(${target})
    makepri_target_add_pdb(${target})
    makepri_target_add_sanitizers(${target})
    makepri_target_install_sanitizers(${target})
    target_link_libraries(${target} PRIVATE GTest::gtest)

    set(
        integration_arguments
        --makepri-under-test
        "$<TARGET_FILE:makepri>"
        --makepri-input-root
        "${PROJECT_SOURCE_DIR}/tests/data/input"
        --makepri-output-root
        "${PROJECT_SOURCE_DIR}/tests/data/output"
    )
    if(MAKEPRI_TEST_OFFICIAL_EXECUTABLE)
        list(APPEND integration_arguments --makepri-official "${MAKEPRI_TEST_OFFICIAL_EXECUTABLE}")
    endif()
    if(MAKEPRI_UPDATE_INTEGRATION_OUTPUTS)
        list(APPEND integration_arguments --makepri-update-outputs)
    endif()

    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST EXTRA_ARGS ${integration_arguments})
endfunction()
