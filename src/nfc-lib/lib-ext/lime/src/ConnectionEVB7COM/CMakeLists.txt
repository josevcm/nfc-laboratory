########################################################################
## Support for EVB7 over COM serial connection
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionEVB7COM)

set(CONNECTION_EVB7_COM_SOURCES
    ${THIS_SOURCE_DIR}/ConnectionEVB7COMEntry.cpp
    ${THIS_SOURCE_DIR}/ConnectionEVB7COM.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_EVB7COM "Enable EVB+COM" OFF "ENABLE_LIBRARY" OFF)
add_feature_info(ConnectionEVB7COM ENABLE_EVB7COM "EVB+COM Connection support")
if (NOT ENABLE_EVB7COM)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_sources(LimeSuite PRIVATE ${CONNECTION_EVB7_COM_SOURCES})
