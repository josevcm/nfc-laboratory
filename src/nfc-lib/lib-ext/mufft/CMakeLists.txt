set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

project(mufft LANGUAGES C)

add_library(mufft STATIC
        ${PRIVATE_SOURCE_DIR}/fft.c
        ${PRIVATE_SOURCE_DIR}/kernel.c
        ${PRIVATE_SOURCE_DIR}/cpu.c
)

# Add conditional compilation for x86 architecture specific libraries
if (CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64|X86|x86|i386|i686")
    add_library(mufft-avx STATIC ${PRIVATE_SOURCE_DIR}/x86/kernel.avx.c)
    add_library(mufft-sse STATIC ${PRIVATE_SOURCE_DIR}/x86/kernel.sse.c)
    add_library(mufft-sse3 STATIC ${PRIVATE_SOURCE_DIR}/x86/kernel.sse3.c)

    target_compile_options(mufft PRIVATE -Wall -Wextra -DMUFFT_HAVE_X86)
    target_compile_options(mufft-sse PRIVATE -Wall -Wextra -msse -mno-sse3 -mno-avx -DMUFFT_HAVE_SSE)
    target_compile_options(mufft-sse3 PRIVATE -Wall -Wextra -msse -msse3 -mno-avx -DMUFFT_HAVE_SSE3)
    target_compile_options(mufft-avx PRIVATE -Wall -Wextra -msse -msse3 -mavx -DMUFFT_HAVE_AVX)

    target_include_directories(mufft-sse PUBLIC ${PUBLIC_INCLUDE_DIR})
    target_include_directories(mufft-sse3 PUBLIC ${PUBLIC_INCLUDE_DIR})
    target_include_directories(mufft-avx PUBLIC ${PUBLIC_INCLUDE_DIR})

    target_link_libraries(mufft PRIVATE mufft-sse mufft-sse3 mufft-avx m)
endif ()

target_include_directories(mufft PUBLIC ${PUBLIC_INCLUDE_DIR})
