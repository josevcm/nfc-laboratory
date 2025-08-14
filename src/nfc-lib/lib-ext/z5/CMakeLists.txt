set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(z5 INTERFACE)

target_include_directories(z5 INTERFACE ${PUBLIC_SOURCE_DIR})

target_link_libraries(z5 INTERFACE xtensor)


