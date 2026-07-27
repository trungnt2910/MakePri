makepri_add_test_executable(makepri_parameter_parser_tests Given_ParameterParser.cpp)
target_link_libraries(
    makepri_parameter_parser_tests
    PRIVATE
        makepri_makepricore
        makepri_parameterparser
)

makepri_add_integration_test_executable(
    makepri_integration_tests
    integration/Given_Command.cpp
    integration/Given_CreateConfig.cpp
    integration/Given_Dump.cpp
    integration/Given_Help.cpp
    integration/Given_InputEdgeCases.cpp
    integration/Given_New.cpp
    integration/Given_ResourcePack.cpp
    integration/Given_Versioned.cpp
)
