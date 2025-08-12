set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(microtar STATIC
        ${PRIVATE_SOURCE_DIR}/microtar.c
)

target_include_directories(microtar PUBLIC ${PUBLIC_INCLUDE_DIR})
