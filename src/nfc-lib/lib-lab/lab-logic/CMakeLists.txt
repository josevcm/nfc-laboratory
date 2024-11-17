set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(lab-logic STATIC
        src/main/cpp/IsoDecoder.cpp
        src/main/cpp/tech/Iso7816.cpp
        src/main/cpp/IsoTech.cpp
)

target_include_directories(lab-logic PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(lab-logic PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(lab-logic lab-data rt-lang hw-logic)