set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_library(airspy STATIC
        src/main/cpp/airspy.c
        src/main/cpp/iqconverter_float.c
        src/main/cpp/iqconverter_int16.c)

target_compile_options(airspy PRIVATE "-msse2" -DUSE_SSE2)

target_include_directories(airspy PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(airspy PRIVATE ${USB_INCLUDE_DIR})

target_link_libraries(airspy ${USB_LIBRARY})