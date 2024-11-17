set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(microtar STATIC
        src/main/c/microtar.c
)

target_include_directories(microtar PUBLIC ${PUBLIC_SOURCE_DIR})
