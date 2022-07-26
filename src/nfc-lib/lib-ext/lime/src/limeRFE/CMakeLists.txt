########################################################################
## Support for LimeRFE
########################################################################
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIMERFE_GUI_SOURCES
    ${THIS_SOURCE_DIR}/limeRFE_gui.cpp
    ${THIS_SOURCE_DIR}/limeRFE_wxgui.cpp
    ${THIS_SOURCE_DIR}/limeRFE_cmd.cpp
    ${THIS_SOURCE_DIR}/RFE_Device.cpp
)
#milans 200309
#if(MSVC)
#    list(APPEND LIMERFE_GUI_SOURCES ${THIS_SOURCE_DIR}/external/termiWin/termiWin.c)
#endif()

set(LIMERFE_SOURCES
    ${THIS_SOURCE_DIR}/limeRFE_cmd.cpp
    ${THIS_SOURCE_DIR}/limeRFE_api.cpp
    ${THIS_SOURCE_DIR}/RFE_Device.cpp
)

#milans 200309
#if(MSVC)
#    list(APPEND LIMERFE_SOURCES ${THIS_SOURCE_DIR}/external/termiWin/termiWin.c)
#endif()

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMERFE "Enable LimeRFE support" ON "ENABLE_LIBRARY" OFF)
add_feature_info(LimeRFE ENABLE_LIMERFE "LimeRFE support")

if(ENABLE_LIMERFE AND ENABLE_GUI)
    target_compile_definitions(LimeSuiteGUI PUBLIC "LIMERFE")
endif()

if (NOT ENABLE_LIMERFE)
    return()
endif()

########################################################################
## Add to library
########################################################################
if(ENABLE_GUI)
    target_sources(LimeSuiteGUI PRIVATE ${LIMERFE_GUI_SOURCES})
endif()
target_sources(LimeSuite PRIVATE ${LIMERFE_SOURCES})
