set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(crapto1 STATIC
        src/main/c/crapto1.c
        src/main/c/crypto1.c
        )

#target_compile_options(crapto1 PRIVATE "-msse2" -DUSE_SSE2)

target_include_directories(crapto1 PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(crapto1 PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(crapto1)