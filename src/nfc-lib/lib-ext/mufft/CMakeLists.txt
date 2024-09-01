cmake_minimum_required(VERSION 3.5)  # Specify minimum CMake version required

set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

project(mufft LANGUAGES C)

add_library(mufft STATIC src/main/c/fft.c src/main/c/kernel.c src/main/c/cpu.c)

# Add conditional compilation for x86 architecture specific libraries
if (CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64|X86|x86|i386|i686")
    add_library(mufft-avx STATIC src/main/c/x86/kernel.avx.c)
    add_library(mufft-sse STATIC src/main/c/x86/kernel.sse.c)
    add_library(mufft-sse3 STATIC src/main/c/x86/kernel.sse3.c)
    
    target_compile_options(mufft PRIVATE -Wall -Wextra -DMUFFT_HAVE_X86)
    target_compile_options(mufft-sse PRIVATE -Wall -Wextra -msse -mno-sse3 -mno-avx -DMUFFT_HAVE_SSE)
    target_compile_options(mufft-sse3 PRIVATE -Wall -Wextra -msse -msse3 -mno-avx -DMUFFT_HAVE_SSE3)
    target_compile_options(mufft-avx PRIVATE -Wall -Wextra -msse -msse3 -mavx -DMUFFT_HAVE_AVX)

    target_link_libraries(mufft PRIVATE mufft-sse mufft-sse3 mufft-avx m)
endif()

target_include_directories(mufft PUBLIC ${PUBLIC_SOURCE_DIR})
