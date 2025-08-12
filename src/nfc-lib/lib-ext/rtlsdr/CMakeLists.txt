set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(rtlsdr STATIC
        ${PRIVATE_SOURCE_DIR}/librtlsdr.c
        ${PRIVATE_SOURCE_DIR}/tuner_e4k.c
        ${PRIVATE_SOURCE_DIR}/tuner_fc0012.c
        ${PRIVATE_SOURCE_DIR}/tuner_fc0013.c
        ${PRIVATE_SOURCE_DIR}/tuner_fc2580.c
        ${PRIVATE_SOURCE_DIR}/tuner_r82xx.c
)

target_include_directories(rtlsdr PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(rtlsdr PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(rtlsdr ${LIBUSB_LIBRARY})