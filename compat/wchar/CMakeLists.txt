# This directory owns the native-width executable boundary and linker wrappers. It deliberately
# exports no headers or transitive compile settings into the short-wchar target graph.
add_library(compat_wchar STATIC src/crt.cpp src/files.cpp src/strings.cpp)
target_compile_options(compat_wchar PRIVATE -Wall -Werror -Wextra -Wpedantic)
target_link_libraries(compat_wchar PRIVATE uni-algo::uni-algo)
