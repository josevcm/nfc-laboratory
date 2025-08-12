set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

#set(BLOSC_INCLUDE_DIRS ${PUBLIC_INCLUDE_DIR} ${PRIVATE_SOURCE_DIR})

# parse the full version numbers from blosc.h
file(READ ${PUBLIC_INCLUDE_DIR}/blosc.h _blosc_h_contents)

string(REGEX REPLACE ".*#define[ \t]+BLOSC_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" BLOSC_VERSION_MAJOR ${_blosc_h_contents})
string(REGEX REPLACE ".*#define[ \t]+BLOSC_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" BLOSC_VERSION_MINOR ${_blosc_h_contents})
string(REGEX REPLACE ".*#define[ \t]+BLOSC_VERSION_RELEASE[ \t]+([0-9]+).*" "\\1" BLOSC_VERSION_PATCH ${_blosc_h_contents})
string(REGEX REPLACE ".*#define[ \t]+BLOSC_VERSION_STRING[ \t]+\"([-0-9A-Za-z.]+)\".*" "\\1" BLOSC_VERSION_STRING ${_blosc_h_contents})

# includes
#include_directories(${BLOSC_INCLUDE_DIRS})

# library sources
set(SOURCES
        ${PRIVATE_SOURCE_DIR}/blosc.c
        ${PRIVATE_SOURCE_DIR}/blosclz.c
        ${PRIVATE_SOURCE_DIR}/fastcopy.c
        ${PRIVATE_SOURCE_DIR}/shuffle.c
        ${PRIVATE_SOURCE_DIR}/shuffle-generic.c
        ${PRIVATE_SOURCE_DIR}/bitshuffle-generic.c)

# hide warnings about attributes
set_source_files_properties(
        ${PRIVATE_SOURCE_DIR}/shuffle.c
        ${PRIVATE_SOURCE_DIR}/shuffle-generic.c
        ${PRIVATE_SOURCE_DIR}/bitshuffle-generic.c
        PROPERTIES COMPILE_FLAGS -Wno-attributes)

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    set(COMPILER_SUPPORT_SSE2 TRUE)
    set(COMPILER_SUPPORT_AVX2 TRUE)
endif ()

if (COMPILER_SUPPORT_SSE2)

    set(SOURCES ${SOURCES}
            ${PRIVATE_SOURCE_DIR}/shuffle-sse2.c
            ${PRIVATE_SOURCE_DIR}/bitshuffle-sse2.c)

    set_source_files_properties(
            ${PRIVATE_SOURCE_DIR}/shuffle-sse2.c
            ${PRIVATE_SOURCE_DIR}/bitshuffle-sse2.c
            PROPERTIES COMPILE_FLAGS "-msse2 -Wno-attributes")

    set_property(
            SOURCE ${PRIVATE_SOURCE_DIR}/shuffle.c
            APPEND PROPERTY COMPILE_DEFINITIONS SHUFFLE_SSE2_ENABLED)

endif (COMPILER_SUPPORT_SSE2)

if (COMPILER_SUPPORT_AVX2)

    set(SOURCES ${SOURCES}
            ${PRIVATE_SOURCE_DIR}/shuffle-avx2.c
            ${PRIVATE_SOURCE_DIR}/bitshuffle-avx2.c)

    set_source_files_properties(
            ${PRIVATE_SOURCE_DIR}/shuffle-avx2.c
            ${PRIVATE_SOURCE_DIR}/bitshuffle-avx2.c
            PROPERTIES COMPILE_FLAGS "-mavx2 -Wno-attributes")

    set_property(
            SOURCE ${PRIVATE_SOURCE_DIR}/shuffle.c
            APPEND PROPERTY COMPILE_DEFINITIONS SHUFFLE_AVX2_ENABLED)

endif (COMPILER_SUPPORT_AVX2)

set(version_string ${BLOSC_VERSION_MAJOR}.${BLOSC_VERSION_MINOR}.${BLOSC_VERSION_PATCH})

add_library(blosc STATIC ${SOURCES})

target_include_directories(blosc PUBLIC ${PUBLIC_INCLUDE_DIR})

#target_link_libraries(blosc )

