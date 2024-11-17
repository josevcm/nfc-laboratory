set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(test-dio
        src/main/cpp/main.cpp
)

target_include_directories(test-dio PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(test-dio PRIVATE ${AUTOGEN_BUILD_DIR}/include)

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(test-dio ${PLATFORM_LIBS} hw-logic hw-dev rt-lang)
