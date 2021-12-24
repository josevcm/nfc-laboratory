set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_executable(airspy-rx
        src/main/c/airspy_rx.c)

target_include_directories(airspy-rx PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(airspy-rx PRIVATE ${USB_INCLUDE_DIR})

target_link_libraries(airspy-rx airspy ${USB_LIBRARY})