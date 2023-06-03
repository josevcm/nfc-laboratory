set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(airspy STATIC
        src/main/c/airspy.c
        src/main/c/iqconverter_float.c
        src/main/c/iqconverter_int16.c)

target_compile_options(airspy PRIVATE "-msse2" -DUSE_SSE2)

target_include_directories(airspy PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(airspy PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(airspy ${LIBUSB_LIBRARY})