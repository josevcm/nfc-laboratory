########################################################################
## Support for EVB7 over COM serial connection
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionRemote)

set(CONNECTION_REMOTE_SOURCES
    ${THIS_SOURCE_DIR}/ConnectionRemoteEntry.cpp
    ${THIS_SOURCE_DIR}/ConnectionRemote.cpp
    ${THIS_SOURCE_DIR}/LMS64CProtocol_remote.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_REMOTE "Enable Remote connection" OFF "ENABLE_LIBRARY" OFF)
add_feature_info(ConnectionRemote ENABLE_REMOTE "DO NOT ENABLE. Feature is for internal development debugging only")
if (NOT ENABLE_REMOTE)
    return()
endif()
message(WARNING "ENABLE_REMOTE is only for internal debugging purposes, do not use for releases")

target_compile_definitions(LimeSuite PRIVATE -DREMOTE_CONTROL)
########################################################################
## Add to library
########################################################################
target_sources(LimeSuite PRIVATE ${CONNECTION_REMOTE_SOURCES})

if(WIN32)
target_link_libraries(LimeSuite Ws2_32.lib)
endif()
