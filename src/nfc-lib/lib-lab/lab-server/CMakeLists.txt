set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

find_package(Protobuf REQUIRED)

add_library(lab-server STATIC
        )

target_include_directories(lab-server PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(lab-server PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(lab-server lab-data rt-lang)