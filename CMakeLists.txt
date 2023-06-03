cmake_minimum_required(VERSION 3.17)

project(nfc-lab VERSION 2.8.4)

set(CMAKE_C_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(SYSTEM_INSTALL_DIR "@ApplicationsDirX64@")

if (WIN32)
    find_path(LIBUSB_INCLUDE_DIR libusb.h HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.26/include)
    find_library(LIBUSB_LIBRARY usb-1.0 HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.26/x86_64-w64-mingw32/lib)
else (WIN32)
    find_path(LIBUSB_INCLUDE_DIR libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
    find_library(LIBUSB_LIBRARY usb-1.0 PATH_SUFFIXES "lib" "lib32" "lib64")
endif (WIN32)

message(STATUS "USB_INCLUDE_DIR: ${LIBUSB_INCLUDE_DIR}")
message(STATUS "USB_LIBRARY: ${LIBUSB_LIBRARY}")

if (WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-runenv.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-runenv.ps1)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-installer.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-installer.ps1)
    add_custom_target(build-runenv COMMAND powershell -File build-runenv.ps1 DEPENDS nfc-lab)
    add_custom_target(build-installer COMMAND powershell -File build-installer.ps1 DEPENDS nfc-lab)
endif (WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/config.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/config.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/package.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/package.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/license.txt.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/license.txt)

add_subdirectory(src)

