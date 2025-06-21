set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(rt-lang STATIC
        src/main/cpp/Executor.cpp
        src/main/cpp/FileSystem.cpp
        src/main/cpp/Format.cpp
        src/main/cpp/Library.cpp
        src/main/cpp/Map.cpp
        src/main/cpp/Package.cpp
        src/main/cpp/Worker.cpp
        src/main/cpp/Tokenizer.cpp
        src/main/cpp/Logger.cpp
)

target_include_directories(rt-lang PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(rt-lang PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(rt-lang microtar z)
