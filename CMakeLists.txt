cmake_minimum_required(VERSION 3.17)

project(nfc-lab)

set(CMAKE_C_FLAGS_DEBUG "-g -march=native -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -march=native -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -march=native -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -march=native -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

set(USB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.20/include)
set(GLEW_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/dll/glew-2.1.0/include)
set(FT_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/dll/freetype-2.11.0/include)

find_library(USB_LIBRARY usb-1.0 HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/usb-1.0.20/x86_64-w64-mingw32/lib)
find_library(GLEW_LIBRARY glew32 HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/glew-2.1.0/x86_64-w64-mingw32/lib)
find_library(FT_LIBRARY freetype HINTS ${CMAKE_CURRENT_LIST_DIR}/dll/freetype-2.11.0/x86_64-w64-mingw32/lib)

message(STATUS "USB_LIBRARY: ${USB_LIBRARY}")
message(STATUS "GLEW_LIBRARY: ${GLEW_LIBRARY}")
message(STATUS "FT_LIBRARY: ${FT_LIBRARY}")

add_subdirectory(src)
