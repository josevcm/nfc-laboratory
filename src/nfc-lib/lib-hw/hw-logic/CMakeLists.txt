set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(hw-logic STATIC
        src/main/cpp/DSLogicDevice.cpp
)

target_include_directories(hw-logic PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(hw-logic PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(hw-logic rt-lang hw-dev)
