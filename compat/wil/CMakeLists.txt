add_library(WIL INTERFACE)
add_library(WIL::WIL ALIAS WIL)
target_include_directories(
    WIL
    SYSTEM
    INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
