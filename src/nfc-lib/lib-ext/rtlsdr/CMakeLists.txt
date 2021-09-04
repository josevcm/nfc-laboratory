set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_library(rtlsdr STATIC
        src/main/cpp/librtlsdr.c
        src/main/cpp/tuner_e4k.c
        src/main/cpp/tuner_fc0012.c
        src/main/cpp/tuner_fc0013.c
        src/main/cpp/tuner_fc2580.c
        src/main/cpp/tuner_r82xx.c)

target_include_directories(rtlsdr PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(rtlsdr PRIVATE ${USB_INCLUDE_DIR})

target_link_libraries(rtlsdr ${USB_LIBRARY})