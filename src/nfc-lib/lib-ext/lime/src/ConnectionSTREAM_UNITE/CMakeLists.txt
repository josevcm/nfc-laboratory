########################################################################
## Support for Stream+UNITE (USB+COM) connection
########################################################################

#if(WIN32)
#    find_package(CyAPI)
#    set(USB_FOUND ${CYAPI_FOUND})
#    set(USB_INCLUDE_DIRS ${CYAPI_INCLUDE_DIRS})
#    set(USB_LIBRARIES ${CYAPI_LIBRARIES} SetupAPI)
#elseif(UNIX)
#    find_package(libusb-1.0)
#    set(USB_FOUND ${LIBUSB_1_FOUND})
#    set(USB_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS})
#    set(USB_LIBRARIES ${LIBUSB_1_LIBRARIES})
#endif()

#message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
#message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionSTREAM_UNITE)

set(CONNECTION_STREAM_UNITE_SOURCES
    ${THIS_SOURCE_DIR}/ConnectionSTREAM_UNITEEntry.cpp
    ${THIS_SOURCE_DIR}/ConnectionSTREAM_UNITE.cpp
)

#set(CONNECTION_STREAM_UNITE_LIBRARIES ${USB_LIBRARIES})

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_STREAM_UNITE "Enable STREAM+UNITE" ON "ENABLE_LIBRARY;ENABLE_EVB7COM;ENABLE_FX3" OFF)
add_feature_info(ConnectionSTREAM_UNITE ENABLE_STREAM_UNITE "STREAM+UNITE Connection support")
if (NOT ENABLE_STREAM_UNITE)
    return()
endif()

########################################################################
## Add to library
########################################################################
#target_include_directories(LimeSuite PUBLIC ${USB_INCLUDE_DIRS})
target_sources(LimeSuite PRIVATE ${CONNECTION_STREAM_UNITE_SOURCES})
#target_link_libraries(LimeSuite ${CONNECTION_STREAM_UNITE_LIBRARIES})
