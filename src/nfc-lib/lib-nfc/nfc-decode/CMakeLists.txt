set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(nfc-decode STATIC
        src/main/cpp/NfcFrame.cpp
        src/main/cpp/NfcDecoder.cpp
        src/main/cpp/tech/NfcA.cpp
        src/main/cpp/tech/NfcB.cpp
        src/main/cpp/tech/NfcF.cpp
        src/main/cpp/tech/NfcV.cpp
        )

target_include_directories(nfc-decode PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(nfc-decode PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(nfc-decode rt-lang sdr-io)