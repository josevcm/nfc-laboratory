set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(sdr-test
        src/main/cpp/main.cpp
        )

target_include_directories(sdr-test PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(sdr-test PRIVATE ${AUTOGEN_BUILD_DIR}/include)

target_link_libraries(sdr-test
        sdr-io
        rt-lang
        mingw32
        psapi
        )
