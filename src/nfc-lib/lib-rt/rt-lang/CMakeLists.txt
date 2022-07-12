set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(rt-lang STATIC
        src/main/cpp/Executor.cpp
        src/main/cpp/Map.cpp
        src/main/cpp/Logger.cpp
        src/main/cpp/Worker.cpp
        src/main/cpp/Format.cpp
        src/main/cpp/FileSystem.cpp
        )

#target_compile_definitions(rt-lang PRIVATE LOG_OUTPUT=0) # NONE
#target_compile_definitions(rt-lang PRIVATE LOG_OUTPUT=1) # buffered STDOUT
#target_compile_definitions(rt-lang PRIVATE LOG_OUTPUT=2) # unbuffered STDERR
#target_compile_definitions(rt-lang PRIVATE LOG_OUTPUT=3) # buffered FILE

target_include_directories(rt-lang PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(rt-lang PRIVATE ${PRIVATE_SOURCE_DIR})

