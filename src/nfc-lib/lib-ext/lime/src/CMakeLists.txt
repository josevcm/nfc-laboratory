########################################################################
## public headers
########################################################################
set(LIME_SUITE_PUBLIC_HEADERS
    lime/LimeSuite.h
    VersionInfo.h
    Logger.h
    SystemResources.h
    LimeSuiteConfig.h
    ADF4002/ADF4002.h
    lms7002m_mcu/MCU_BD.h
    lms7002m_mcu/MCU_File.h
    ConnectionRegistry/IConnection.h
    ConnectionRegistry/ConnectionHandle.h
    ConnectionRegistry/ConnectionRegistry.h
    lms7002m/LMS7002M.h
    lms7002m/LMS7002M_RegistersMap.h
    lime/LMS7002M_parameters.h
    protocols/Streamer.h
    protocols/ADCUnits.h
    protocols/LMS64CCommands.h
    protocols/LMS64CProtocol.h
    protocols/LMSBoards.h
    protocols/dataTypes.h
    protocols/fifo.h
    Si5351C/Si5351C.h
    FPGA_common/FPGA_common.h
    API/lms7_device.h
#    limeRFE/limeRFE.h
)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_HEADERS "Enable install headers" ON "TRUE" OFF)
add_feature_info(LimeSuiteHeaders ENABLE_HEADERS "The lime suite headers")
if (ENABLE_HEADERS)
    install(
        FILES ${LIME_SUITE_PUBLIC_HEADERS}
        DESTINATION include/lime
    )
endif()

########################################################################
## lime suite library
########################################################################
set(LIME_SUITE_SOURCES
    Logger.cpp
    ADF4002/ADF4002.cpp
    lms7002m_mcu/MCU_BD.cpp
    lms7002m_mcu/MCU_File.cpp
    ConnectionRegistry/IConnection.cpp
    ConnectionRegistry/ConnectionHandle.cpp
    ConnectionRegistry/ConnectionRegistry.cpp
    lms7002m/LMS7002M_RegistersMap.cpp
    lms7002m/LMS7002M_parameters.cpp
    lms7002m/LMS7002M.cpp
    lms7002m/LMS7002M_RxTxCalibrations.cpp
    lms7002m/LMS7002M_BaseCalibrations.cpp
    lms7002m/mcu_dc_iq_calibration.cpp
    lms7002m/LMS7002M_filtersCalibration.cpp
    lms7002m/LMS7002M_gainCalibrations.cpp
    protocols/LMS64CProtocol.cpp
    protocols/Streamer.cpp
    protocols/ConnectionImages.cpp
    Si5351C/Si5351C.cpp
    ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c
    API/lms7_api.cpp
    API/lms7_device.cpp
    API/LmsGeneric.cpp
    API/qLimeSDR.cpp
    API/LimeSDR_mini.cpp
    API/LimeSDR.cpp
    API/LimeNET_micro.cpp
    API/LimeSDR_PCIE.cpp
    API/LimeSDR_Core.cpp
    FPGA_common/FPGA_common.cpp
    FPGA_common/FPGA_Mini.cpp
    FPGA_common/FPGA_Q.cpp
    windowFunction.cpp
    threadHelper/threadHelper.cpp
)

set(LIME_SUITE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    API
    lime
    GFIR
    protocols
    ConnectionRegistry
    lms7002m_mcu
    ADF4002
    Si5351C
    lms7002m
    FPGA_common
    lms7002m_mcu
    ${PROJECT_SOURCE_DIR}/external/cpp-feather-ini-parser
    ${PROJECT_SOURCE_DIR}/external/kissFFT/
#    limeRFE
    threadHelper
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp @ONLY)
list(APPEND LIME_SUITE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SystemResources.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/SystemResources.cpp @ONLY)
list(APPEND LIME_SUITE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/SystemResources.cpp)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
list(APPEND LIME_SUITE_LIBRARIES Threads::Threads)

include(CheckAtomic)
if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB OR NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
    list(APPEND LIME_SUITE_LIBRARIES atomic)
endif()

include(FeatureSummary)
include(CMakeDependentOption)

option(ENABLE_LIBRARY "Enable build library" ON)
add_feature_info(LimeSuiteLibrary ENABLE_LIBRARY "The lime suite library")
if (ENABLE_LIBRARY)
    add_library(LimeSuite STATIC ${LIME_SUITE_SOURCES})
    target_link_libraries(LimeSuite ${LIME_SUITE_LIBRARIES})
    set_property(TARGET LimeSuite PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(LimeSuite PROPERTIES SOVERSION ${LIME_SUITE_SOVER})
    set_target_properties(LimeSuite PROPERTIES VERSION ${LIME_SUITE_LIBVER})
    set_target_properties(LimeSuite PROPERTIES DEFINE_SYMBOL "LIME_DLL_EXPORTS")
    target_include_directories(LimeSuite PUBLIC ${LIME_SUITE_INCLUDES})

#    if(BUILD_SHARED_LIBS)
#        target_compile_definitions(LimeSuite PUBLIC "LIME_DLL")
#    endif()

    install(TARGETS LimeSuite
        LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
        RUNTIME DESTINATION bin              # .dll file
    )

	if (MSVC)
		set_target_properties(LimeSuite PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	endif(MSVC)

    add_subdirectory(GFIR)
    set(LIME_SUITE_LIBS LimeSuite)
endif()

########################################################################
## Supported connections
########################################################################
#include(ConnectionEVB7COM/CMakeLists.txt)
#include(ConnectionFX3/CMakeLists.txt)
#include(ConnectionSTREAM_UNITE/CMakeLists.txt)
include(ConnectionFTDI/CMakeLists.txt)
#include(ConnectionXillybus/CMakeLists.txt)
#include(ConnectionSPI/CMakeLists.txt)

## Remote connection is not safe or efficient, it should not be in releases (github #263)
## it's only for debugging purposes when USB device cannot be used by multiple applications simultaneously.
#include(ConnectionRemote/CMakeLists.txt)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionRegistry/BuiltinConnections.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp
)

target_sources(LimeSuite PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp)

########################################################################
## wx widgets dependency
########################################################################
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets COMPONENTS richtext xml html xrc adv aui core base gl)
message(STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")
message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_GUI "Enable build GUI" OFF "ENABLE_LIBRARY;wxWidgets_FOUND" OFF)
add_feature_info(LimeSuiteGUI ENABLE_GUI "GUI Application for LimeSuite")
if (ENABLE_GUI)

    ########################################################################
    ## LimeSuite GUI app
    ########################################################################
    set(LMS7SUITE_GUI_SOURCES
            ADF4002/ADF4002_wxgui.cpp
            lms7002_wxgui/lms7002_pnlXBUF_view.cpp
            lms7002_wxgui/lms7002_pnlRFE_view.cpp
            lms7002_wxgui/lms7002_pnlCalibrations_view.cpp
            lms7002_wxgui/lms7002_pnlTRF_view.cpp
            lms7002_wxgui/lms7002_pnlCDS_view.cpp
            lms7002_wxgui/lms7002_pnlLimeLightPAD_view.cpp
            lms7002_wxgui/lms7002_pnlBIST_view.cpp
            lms7002_wxgui/dlgViewIRAM.cpp
            lms7002_wxgui/lms7002_gui_utilities.cpp
            lms7002_wxgui/lms7002_pnlSX_view.cpp
            lms7002_wxgui/lms7002_pnlTBB_view.cpp
            lms7002_wxgui/lms7002_dlgVCOfrequencies.cpp
            lms7002_wxgui/lms7002_pnlRBB_view.cpp
            lms7002_wxgui/lms7002_pnlLDO_view.cpp
            lms7002_wxgui/lms7002_pnlAFE_view.cpp
            lms7002_wxgui/lms7002_pnlGains_view.cpp
            lms7002_wxgui/CoefficientFileParser.cpp
            lms7002_wxgui/lms7002_pnlRxTSP_view.cpp
            lms7002_wxgui/lms7002_mainPanel.cpp
            lms7002_wxgui/dlgViewSFR.cpp
            lms7002_wxgui/lms7002_pnlCLKGEN_view.cpp
            lms7002_wxgui/lms7002_pnlMCU_BD_view.cpp
            lms7002_wxgui/lms7002_pnlBIAS_view.cpp
            lms7002_wxgui/lms7002_dlgGFIR_Coefficients.cpp
            lms7002_wxgui/lms7002_pnlTxTSP_view.cpp
            lms7002_wxgui/lms7002_pnlR3.cpp
            LMS_Programing/LMS_Programing_wxgui.cpp
            utilities_gui/dlgConnectionSettings.cpp
            utilities_gui/pnlMiniLog.cpp
            utilities_gui/pnlAPI.cpp
            utilities_gui/SPI_wxgui.cpp
            utilities_gui/dlgFullMessageLog.cpp
            utilities_gui/dlgDeviceInfo.cpp
            utilities_gui/utilities_gui.cpp
            lms7suiteAppFrame.cpp
            boards_wxgui/RFSpark_wxgui.cpp
            Si5351C/Si5351C_wxgui.cpp
            lms7suiteApp_gui.cpp
            lms7suiteApp.cpp
            FPGAcontrols_wxgui/FPGAcontrols_wxgui.cpp
            dlgAbout.cpp
            numericSlider/numericSlider.cpp
            fftviewer_wxgui/fftviewer_wxgui.cpp
            fftviewer_wxgui/fftviewer_frFFTviewer.cpp
            lms7suiteEvents/lms7suiteEvents.cpp
            boards_wxgui/pnlBoardControls.cpp
            boards_wxgui/pnlQSpark.cpp
            boards_wxgui/pnluLimeSDR.cpp
            boards_wxgui/pnlUltimateEVB.cpp
            boards_wxgui/pnlBuffers.cpp
            boards_wxgui/pnlCoreSDR.cpp
            boards_wxgui/pnlLimeNetMicro.cpp
            ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c
            windowFunction.cpp
            boards_wxgui/pnlLimeSDR.cpp
            boards_wxgui/pnlGPIO.cpp
            )

    include_directories(lms7002_wxgui)
    include_directories(LMS_Programing)
    include_directories(utilities_gui)
    include_directories(RFSpark)
    include_directories(FPGAcontrols_wxgui)
    include_directories(numericSlider)
    include_directories(fftviewer_wxgui)
    include_directories(lms7suiteEvents)
    include_directories(boards_wxgui)
#    include_directories(limeRFE)

    include(${wxWidgets_USE_FILE})
    add_subdirectory(oglGraph)

    set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

    add_executable(LimeSuiteGUI ${LMS7SUITE_GUI_SOURCES} resources/resource.rc)
    set_target_properties(LimeSuiteGUI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
    target_compile_definitions(LimeSuiteGUI PRIVATE WIN32_LEAN_AND_MEAN)

    target_link_libraries(LimeSuiteGUI LimeSuite oglGraph ${wxWidgets_LIBRARIES})

    if (MSVC)
        set_property(TARGET LimeSuiteGUI APPEND PROPERTY LINK_FLAGS /SUBSYSTEM:WINDOWS)
    endif(MSVC)

    install(TARGETS LimeSuiteGUI DESTINATION bin)

endif()

########################################################################
# Build pkg config file
########################################################################
if (ENABLE_HEADERS AND ENABLE_LIBRARY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/LimeSuite.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/LimeSuite.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/LimeSuite.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
)
endif (ENABLE_HEADERS AND ENABLE_LIBRARY)

#########################################################################
# utilities
#########################################################################
add_subdirectory(utilities)

#########################################################################
# examples
#########################################################################
add_subdirectory(examples)

#########################################################################
# LimeRFE
#########################################################################
#add_subdirectory(limeRFE)
