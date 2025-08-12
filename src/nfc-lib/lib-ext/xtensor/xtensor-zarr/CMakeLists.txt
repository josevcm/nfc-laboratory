set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtensor-zarr STATIC
        ${PRIVATE_SOURCE_DIR}/xtensor-zarr.cpp
)

target_include_directories(xtensor-zarr PUBLIC ${PUBLIC_INCLUDE_DIR})

target_link_libraries(xtensor-zarr xtensor-io zarray blosc)


