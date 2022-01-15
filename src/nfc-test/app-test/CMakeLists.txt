set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(nfc-test
        src/main/cpp/main.cpp
        )

target_include_directories(nfc-test PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-test PRIVATE ${AUTOGEN_BUILD_DIR}/include)

target_link_libraries(nfc-test
        nfc-decode
        sdr-io
        rt-lang
        nlohmann
        mingw32
        psapi
        )
