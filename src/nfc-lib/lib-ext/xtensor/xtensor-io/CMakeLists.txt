set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtensor-io INTERFACE)

target_include_directories(xtensor-io INTERFACE ${PUBLIC_SOURCE_DIR})

target_link_libraries(xtensor-io INTERFACE xtensor)


