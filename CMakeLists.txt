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
set(CMAKE_AR gcc-ar)
set(CMAKE_RANLIB gcc-ranlib)

set(CMAKE_C_FLAGS_DEBUG "-g -Og -fno-omit-frame-pointer" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og -fno-omit-frame-pointer" CACHE INTERNAL "" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-O3 -fno-math-errno -falign-functions=32 -falign-loops=32 -march=native -fopenmp" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-math-errno -falign-functions=32 -falign-loops=32 -march=native -fopenmp" CACHE INTERNAL "" FORCE)

if (UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
else(WIN32)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wa,-mbig-obj")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
endif()

message(STATUS "Build for ${CMAKE_SYSTEM_PROCESSOR} in ${CMAKE_BUILD_TYPE} mode")

# customize CPU architecture options
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  message(STATUS "Enabled SSE/SSE3 instruction set")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse3 -mno-avx")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse3 -mno-avx")
endif ()

#-------------------------------------------------------------------------------
# find dependency libusb-1.0 MANDATORY
#-------------------------------------------------------------------------------
find_package(libusb-1.0)

if(NOT LIBUSB_FOUND)
  message(FATAL_ERROR  "Please install libusb-1.0!")
endif()

message(STATUS "libusb-1.0: found and using dynamic link mode")
message(STATUS "  include: " ${LIBUSB_INCLUDE})
message(STATUS "  library: " ${LIBUSB_LIBRARY})

include_directories(${LIBUSB_INCLUDE})

#-------------------------------------------------------------------------------
# find dependency airspy OPTIONAL
#-------------------------------------------------------------------------------
find_package(libairspy)

if(LIBAIRSPY_FOUND)
  message(STATUS "libairspy: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBAIRSPY_INCLUDE})
  message(STATUS "  library: " ${LIBAIRSPY_LIBRARY})
  include_directories(${LIBAIRSPY_INCLUDE})
else ()
  message(STATUS "libairspy: not found, using local bundle in static mode")
endif()

#-------------------------------------------------------------------------------
# find dependency libblosc OPTIONAL
#-------------------------------------------------------------------------------
find_package(libblosc)

if(LIBBLOSC_FOUND)
  message(STATUS "blosc: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBBLOSC_INCLUDE})
  message(STATUS "  library: " ${LIBBLOSC_LIBRARY})
  include_directories(${LIBBLOSC_INCLUDE})
else ()
  message(STATUS "libblosc: not found, using local bundle in static mode")
endif()

#-------------------------------------------------------------------------------
# find dependency hydrasdr OPTIONAL
#-------------------------------------------------------------------------------
find_package(libhydrasdr)

if(LIBHYDRASDR_FOUND)
  message(STATUS "hydrasdr: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBHYDRASDR_INCLUDE})
  message(STATUS "  library: " ${LIBHYDRASDR_LIBRARY})
  include_directories(${LIBHYDRASDR_INCLUDE})
else ()
  message(STATUS "libhydrasdr: not found, using local bundle in static mode")
endif()

#-------------------------------------------------------------------------------
# find dependency rltsdr OPTIONAL
#-------------------------------------------------------------------------------
find_package(librtlsdr)

if(LIBRTLSDR_FOUND)
  message(STATUS "librtlsdr: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBRTLSDR_INCLUDE})
  message(STATUS "  library: " ${LIBRTLSDR_LIBRARY})
  include_directories(${LIBRTLSDR_INCLUDE})
else ()
  message(STATUS "librtlsdr not found, using local bundle in static mode")
endif()

#-------------------------------------------------------------------------------
# find dependency mirisdr OPTIONAL
#-------------------------------------------------------------------------------
find_package(libmirisdr)

if(LIBMIRISDR_FOUND)
  message(STATUS "libmirisdr: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBMIRISDR_INCLUDE})
  message(STATUS "  library: " ${LIBMIRISDR_LIBRARY})
  include_directories(${LIBMIRISDR_INCLUDE})
else ()
  message(STATUS "libmirisdr not found, using local bundle in static mode")
endif()

#-------------------------------------------------------------------------------
# find dependency xtensor OPTIONAL
#-------------------------------------------------------------------------------
find_package(libxtensor)

if(LIBXTENSOR_FOUND)
  message(STATUS "libxtensor: found and using dynamic link mode")
  message(STATUS "  include: " ${LIBXTENSOR_INCLUDE})
  include_directories(${LIBXTENSOR_INCLUDE})
else ()
  message(STATUS "libxtensor not found, using local bundle in static mode")
endif()

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

