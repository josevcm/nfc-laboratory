set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(posix STATIC
        src/main/c/select.c
)

target_include_directories(posix PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(posix PRIVATE ${PRIVATE_SOURCE_DIR})
