set(CMAKE_C_STANDARD 99)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(hackrf STATIC ${PRIVATE_SOURCE_DIR}/hackrf.c)

target_compile_definitions(hackrf PRIVATE LIBRARY_VERSION="2026.01.3" LIBRARY_RELEASE="2026.01.3")

target_include_directories(hackrf PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(hackrf PRIVATE ${LIBUSB_INCLUDE_DIRS})

target_link_libraries(hackrf ${LIBUSB_LIBRARIES} pthread)
