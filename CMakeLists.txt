cmake_minimum_required(VERSION 3.17)

project(nfc-lab)

set(CMAKE_C_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -msse -msse3 -mno-avx -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(USB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.20/include)

find_library(USB_LIBRARY usb-1.0 HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.20/x86_64-w64-mingw32/lib)

message(STATUS "USB_LIBRARY: ${USB_LIBRARY}")

add_subdirectory(src)
