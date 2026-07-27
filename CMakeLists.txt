cmake_minimum_required(VERSION 3.28)
project(makepri)
set(CMAKE_CXX_STANDARD 23)

set(
    MAKEPRI_INCLUDE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/src/compat/win32/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/embedfiles/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/folder/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/prifile/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/priinfo/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/resfiles/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/resjson/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/fsi/resx/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/ibc/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/langs/common/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/mrm/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/mrm/mrmstatus/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/runtime/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/tools/jsonparser/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/tools/makepri/include"
    "${PROJECT_SOURCE_DIR}/src/mrt/tools/utilities/include"
)

include(cmake/Architecture.cmake)

include(cmake/ThirdParty.cmake)

include(cmake/PDB.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Win32.cmake)

include(cmake/Targets.cmake)

add_subdirectory(src)

if(BUILD_TESTING)
    include(cmake/Testing.cmake)
    add_subdirectory(tests)
endif()
