set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(lab-data STATIC
        src/main/cpp/Crc.cpp
        src/main/cpp/RawFrame.cpp
        src/main/cpp/StreamTree.cpp
)

target_include_directories(lab-data PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(lab-data PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(lab-data rt-lang hw-radio hw-logic)