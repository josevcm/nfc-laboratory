set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

file(GLOB LMS7002M_SOURCES "${PUBLIC_SOURCE_DIR}/LMS7002M_*.c")

add_library(lms7002m STATIC ${LMS7002M_SOURCES})

#target_compile_options(lms7002m PRIVATE "-msse2" -DUSE_SSE2)

target_include_directories(lms7002m PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(lms7002m PRIVATE ${PUBLIC_INCLUDE_DIR})

target_link_libraries(lms7002m posix ${LIBUSB_LIBRARY})