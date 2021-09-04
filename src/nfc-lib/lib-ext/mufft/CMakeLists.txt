set(CMAKE_C_STANDARD 99)
set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

project(mufft LANGUAGES C)

add_library(mufft STATIC src/main/cpp/fft.c src/main/cpp/kernel.c src/main/cpp/cpu.c)
add_library(mufft-avx STATIC src/main/cpp/x86/kernel.avx.c)
add_library(mufft-sse STATIC src/main/cpp/x86/kernel.sse.c)
add_library(mufft-sse3 STATIC src/main/cpp/x86/kernel.sse3.c)

target_compile_options(mufft PRIVATE -Wall -Wextra)
target_compile_options(mufft-sse PRIVATE -Wall -Wextra -msse -mno-sse3 -mno-avx)
target_compile_options(mufft-sse3 PRIVATE -Wall -Wextra -msse -msse3 -mno-avx)
target_compile_options(mufft-avx PRIVATE -Wall -Wextra -msse -msse3 -mavx)

target_compile_definitions(mufft PRIVATE MUFFT_HAVE_X86 MUFFT_HAVE_SSE MUFFT_HAVE_SSE3 MUFFT_HAVE_AVX)

target_include_directories(mufft PUBLIC ${PUBLIC_SOURCE_DIR})

target_link_libraries(mufft PRIVATE mufft-sse mufft-sse3 mufft-avx m)
