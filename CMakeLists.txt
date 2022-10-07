cmake_minimum_required(VERSION 3.17)

project(nfc-lab VERSION 2.8.1)

set(CMAKE_C_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(USB_LIBRARY_PATH ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.26)
set(USB_INCLUDE_DIR ${USB_LIBRARY_PATH}/include)

set(SYSTEM_INSTALL_DIR "@ApplicationsDirX64@")

find_library(USB_LIBRARY usb-1.0 HINTS ${USB_LIBRARY_PATH}/x86_64-w64-mingw32/lib)

message(STATUS "USB_INCLUDE_DIR: ${USB_INCLUDE_DIR}")
message(STATUS "USB_LIBRARY: ${USB_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/config.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/config.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/package.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/package.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/license.txt.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/license.txt)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-installer.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-installer.ps1)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-runenv.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-runenv.ps1)

add_subdirectory(src)

add_custom_target(build-runenv
        COMMAND powershell -File build-runenv.ps1
        DEPENDS nfc-lab)

add_custom_target(build-installer
        COMMAND powershell -File build-installer.ps1
        DEPENDS nfc-lab)

