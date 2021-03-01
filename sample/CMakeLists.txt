project (scoped-exit-cxx20-run)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} "run.cxx")

target_link_libraries(${PROJECT_NAME} PRIVATE libs::scoped-exit-cxx20)
