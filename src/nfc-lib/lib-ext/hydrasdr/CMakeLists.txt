set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(hydrasdr STATIC
        src/main/c/hydrasdr.c
        src/main/c/iqconverter_float.c
        src/main/c/iqconverter_int16.c)

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    target_compile_options(hydrasdr PRIVATE "-msse2" -DUSE_SSE2)
endif ()

target_include_directories(hydrasdr PUBLIC ${PUBLIC_SOURCE_DIR})

target_link_libraries(hydrasdr ${LIBUSB_LIBRARY})
