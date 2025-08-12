set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtensor-zarr INTERFACE)

target_include_directories(xtensor-zarr INTERFACE ${PUBLIC_SOURCE_DIR})

target_link_libraries(xtensor-zarr INTERFACE xtensor-io zarray blosc)


