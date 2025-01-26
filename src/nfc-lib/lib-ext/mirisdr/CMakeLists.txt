set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(mirisdr STATIC src/main/c/libmirisdr.c)
#add_library(getopt STATIC src/main/c/getopt/getopt.c)

target_include_directories(mirisdr PUBLIC ${PUBLIC_SOURCE_DIR})

target_compile_options(mirisdr PRIVATE -Dmirisdr_STATIC)
target_compile_options(mirisdr PRIVATE -DSOVERSION="${MAJOR_VERSION}")
target_compile_options(mirisdr PRIVATE -DVERSION="${LIBVER}")

target_link_libraries(mirisdr ${LIBUSB_LIBRARY})

########################################################################
# Build utility
########################################################################

#add_executable(miri_sdr src/main/c/miri_sdr.c)
#add_executable(miri_fm src/main/c/miri_fm.c)

#target_link_libraries(miri_sdr mirisdr getopt ${LIBUSB_LIBRARY})
#target_link_libraries(miri_fm mirisdr getopt ${LIBUSB_LIBRARY})

