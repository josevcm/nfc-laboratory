set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtensor INTERFACE)

target_include_directories(xtensor INTERFACE ${PUBLIC_SOURCE_DIR})

target_link_libraries(xtensor INTERFACE xtl)


