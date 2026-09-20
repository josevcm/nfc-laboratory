set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(test-clock
        src/main/cpp/main.cpp
)

target_include_directories(test-clock PRIVATE ${PRIVATE_SOURCE_DIR})

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(test-clock ${PLATFORM_LIBS} lab-logic rt-lang)
