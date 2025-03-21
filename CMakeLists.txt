cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(COMPANY "Jose Vicente Campos")
set(PROGRAM_NAME "nfc-lab")
set(ICON_FILE_PATH "../assets/app/rc/nfc-lab.ico")
set(APP_TARGET_DIR "@HomeDir@/.${PROGRAM_NAME}")

if(NOT DEFINED BUILD_PROJECT_VERSION)
  set(BUILD_PROJECT_VERSION "0.0.0")
endif()

project(nfc-lab VERSION "${BUILD_PROJECT_VERSION}" LANGUAGES C CXX)

#-------------------------------------------------------------------------------
# build flags
#-------------------------------------------------------------------------------
if(WIN32)
  set(DEBUG_BIGOBJ_FLAG "-Wa,-mbig-obj")
else()
  set(DEBUG_BIGOBJ_FLAG "")
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -Og -fno-math-errno -falign-functions=32 -falign-loops=32 ${DEBUG_BIGOBJ_FLAG}" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og -fno-math-errno -falign-functions=32 -falign-loops=32 ${DEBUG_BIGOBJ_FLAG}" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-g1 -O3 -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-g1 -O3 -fno-math-errno -falign-functions=32 -falign-loops=32" CACHE INTERNAL "" FORCE)

if (UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

message(STATUS "Build for ${CMAKE_SYSTEM_PROCESSOR} in ${CMAKE_BUILD_TYPE} mode")

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  message(STATUS "Enabled SSE/SSE3 instruction set")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse3 -mno-avx")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse3 -mno-avx")
endif ()

#-------------------------------------------------------------------------------
# find dependency libusb-1.0
#-------------------------------------------------------------------------------
find_package(libusb-1.0)

if(NOT LIBUSB_FOUND)
  message(FATAL_ERROR  "Please install libusb-1.0!")
endif()

message(STATUS "usb-1.0:")
message(STATUS "  include:" ${LIBUSB_INCLUDE})
message(STATUS "  library:" ${LIBUSB_LIBRARY})

include_directories(${LIBUSB_INCLUDE})

#-------------------------------------------------------------------------------
# find dependency airspy
#-------------------------------------------------------------------------------
find_package(libairspy)

if(NOT LIBAIRSPY_FOUND)
  message(FATAL_ERROR  "Please install libairspy!")
endif()

message(STATUS "airspy:")
message(STATUS "  include:" ${LIBAIRSPY_INCLUDE})
message(STATUS "  library:" ${LIBAIRSPY_LIBRARY})

include_directories(${LIBAIRSPY_INCLUDE})

#-------------------------------------------------------------------------------
# find dependency rltsdr
#-------------------------------------------------------------------------------
find_package(librtlsdr)

if(NOT LIBUSB_FOUND)
  message(FATAL_ERROR  "Please install librtlsdr!")
endif()

message(STATUS "rtlsdr:")
message(STATUS "  include:" ${LIBRTLSDR_INCLUDE})
message(STATUS "  library:" ${LIBRTLSDR_LIBRARY})

include_directories(${LIBRTLSDR_INCLUDE})

#-------------------------------------------------------------------------------
# configure files
#-------------------------------------------------------------------------------

if (WIN32)

  # generate build scripts
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-runenv.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-runenv.ps1)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/scripts/build-installer.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/build-installer.ps1)

  # generate build scripts
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/config.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/config.xml)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/package.xml.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/package.xml)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/license.txt.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/license.txt)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/installer/script.js.in ${CMAKE_CURRENT_BINARY_DIR}/installer/config/script.js)

endif (WIN32)

#-------------------------------------------------------------------------------
# configure resources
#-------------------------------------------------------------------------------
add_subdirectory(src)

