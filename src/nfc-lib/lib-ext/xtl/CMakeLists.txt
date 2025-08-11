set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtl INTERFACE)

target_include_directories(xtl INTERFACE ${PUBLIC_SOURCE_DIR})

