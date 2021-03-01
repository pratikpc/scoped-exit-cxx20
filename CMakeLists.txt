cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

project(Scoped-Exit VERSION 0.1.0)

enable_language(CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

add_subdirectory(header)
add_subdirectory(sample)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
