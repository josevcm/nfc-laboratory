set(CMAKE_C_STANDARD 11)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_library(nlohmann INTERFACE)

target_include_directories(nlohmann INTERFACE ${PUBLIC_SOURCE_DIR})

