cmake_minimum_required(VERSION 3.16)

set(COMPANY "Jose Vicente Campos")
set(PROGRAM_NAME "nfc-laboratory")

if(NOT DEFINED BUILD_PROJECT_VERSION)
    set(BUILD_PROJECT_VERSION "0.0.0" CACHE STRING "Project version" FORCE)
endif()

project(nfc-lab VERSION "${BUILD_PROJECT_VERSION}" LANGUAGES C CXX)

set(CMAKE_C_FLAGS_DEBUG "-g -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(SYSTEM_INSTALL_DIR "@ApplicationsDirX64@")

message(STATUS "Build for ${CMAKE_SYSTEM_PROCESSOR}")

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    message(STATUS "Enabled SSE/SSE3 instruction set")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse3 -mno-avx")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse3 -mno-avx")
endif ()

# customize SO architecture options
if (WIN32)

    find_path(LIBUSB_INCLUDE_DIR libusb.h HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.26/include)
    find_library(LIBUSB_LIBRARY usb-1.0 HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.26/x86_64-w64-mingw32/lib)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-runenv.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-runenv.ps1)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-installer.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-installer.ps1)
    add_custom_target(build-runenv COMMAND powershell -File build-runenv.ps1 DEPENDS nfc-lab)
    add_custom_target(build-installer COMMAND powershell -File build-installer.ps1 DEPENDS nfc-lab)

elseif (UNIX)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

    find_path(LIBUSB_INCLUDE_DIR libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
    find_library(LIBUSB_LIBRARY usb-1.0 PATH_SUFFIXES "lib" "lib32" "lib64")

endif ()

message(STATUS "Detected libusb ${LIBUSB_INCLUDE_DIR} ${LIBUSB_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/config.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/config.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/package.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/package.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/license.txt.in ${CMAKE_CURRENT_BINARY_DIR}/installer/packages/org.josevcm.nfclab/meta/license.txt)

add_subdirectory(src)

