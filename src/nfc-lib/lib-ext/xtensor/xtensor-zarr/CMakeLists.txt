set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(xtensor-zarr STATIC
        ${PRIVATE_SOURCE_DIR}/xtensor-zarr-blosc.cpp
        ${PRIVATE_SOURCE_DIR}/xtensor-zarr-gzip.cpp
        ${PRIVATE_SOURCE_DIR}/xtensor-zarr-zlib.cpp
        ${PRIVATE_SOURCE_DIR}/xtensor-zarr.cpp
)

set_target_properties(xtensor-zarr PROPERTIES COMPILE_FLAGS "-O3 -fno-math-errno -falign-functions=32 -falign-loops=32 -march=native -fopenmp")

target_compile_definitions(xtensor-zarr PUBLIC XTENSOR_ZARR_STATIC_LIB)

target_include_directories(xtensor-zarr PUBLIC ${PUBLIC_INCLUDE_DIR})

target_link_libraries(xtensor-zarr xtensor-io zarray blosc)


