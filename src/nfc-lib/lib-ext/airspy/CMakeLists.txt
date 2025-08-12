set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(airspy STATIC
        ${PRIVATE_SOURCE_DIR}/airspy.c
        ${PRIVATE_SOURCE_DIR}/iqconverter_float.c
        ${PRIVATE_SOURCE_DIR}/iqconverter_int16.c)

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    target_compile_options(airspy PRIVATE "-msse2" -DUSE_SSE2)
endif ()

target_include_directories(airspy PUBLIC ${PUBLIC_INCLUDE_DIR})

target_link_libraries(airspy ${LIBUSB_LIBRARY})
