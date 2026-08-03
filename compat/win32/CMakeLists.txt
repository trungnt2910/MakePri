add_library(
    compat_win32
    STATIC
    src/appmodel.cpp
    src/atomics.cpp
    src/automation.cpp
    src/com.cpp
    src/console.cpp
    src/crt.cpp
    src/environment.cpp
    src/errors.cpp
    src/files.cpp
    src/locale.cpp
    src/memory.cpp
    src/messages.cpp
    src/modules.cpp
    src/path.cpp
    src/process.cpp
    src/registry.cpp
    src/rpc.cpp
    src/resources.cpp
    src/security.cpp
    src/stdio.cpp
    src/streams.cpp
    src/synchronization.cpp
    src/system.cpp
    src/threading.cpp
    src/time.cpp
    src/unicode.cpp
    src/uri.cpp
    src/vm.cpp
    src/wchar.cpp
    src/wstring.cpp
    src/xml.cpp
)
target_compile_options(
    compat_win32
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -Wpedantic
        -fms-extensions
        -fshort-wchar
)
target_include_directories(compat_win32 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(compat_win32 SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
    compat_win32
    PRIVATE
        Boost::dll
        Boost::interprocess
        LibXml2::LibXml2
        uni-algo::uni-algo
)
# Existing targets retain their Windows import-library names. On non-Windows systems these
# interface targets route those dependencies to the compatible implementations above.
foreach(import_library IN ITEMS msxml6 ole32 oleaut32 rpcrt4 shlwapi urlmon)
    add_library(${import_library} INTERFACE)
    target_link_libraries(${import_library} INTERFACE "$<LINK_LIBRARY:WHOLE_ARCHIVE,compat_win32>")
endforeach()
