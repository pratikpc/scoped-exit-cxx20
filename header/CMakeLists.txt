project (scoped-exit-cxx20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} INTERFACE)
add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
INTERFACE
.
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)


if (MSVC)
target_compile_options(${PROJECT_NAME} INTERFACE /Zc:__cplusplus)
endif()
