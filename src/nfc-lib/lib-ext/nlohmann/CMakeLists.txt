set(CMAKE_C_STANDARD 11)

set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(nlohmann INTERFACE)

target_include_directories(nlohmann INTERFACE ${PUBLIC_INCLUDE_DIR})

