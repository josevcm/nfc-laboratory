set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PRIVATE_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/assets)

if (PROJECT_VERSION_MAJOR GREATER 0)
    set(ENABLE_CONSOLE_OUTPUT OFF)
else ()
    set(ENABLE_CONSOLE_OUTPUT ON)
endif ()

find_package(Qt6 COMPONENTS Core Widgets Network PrintSupport REQUIRED)

string(TIMESTAMP YEAR "%Y")

configure_file(${PRIVATE_SOURCE_DIR}/main.rc.in ${PRIVATE_SOURCE_DIR}/main.rc)
configure_file(${PRIVATE_SOURCE_DIR}/QtConfig.h.in ${PRIVATE_SOURCE_DIR}/QtConfig.h)

set(SOURCES
        ${PRIVATE_SOURCE_DIR}/main.cpp
        ${PRIVATE_SOURCE_DIR}/main.rc
        ${PRIVATE_SOURCE_DIR}/QtApplication.cpp
        ${PRIVATE_SOURCE_DIR}/QtControl.cpp
        ${PRIVATE_SOURCE_DIR}/QtStorage.cpp
        ${PRIVATE_SOURCE_DIR}/QtWindow.cpp
        ${PRIVATE_SOURCE_DIR}/events/ConsoleLogEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/DecoderControlEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/FourierStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/LogicDecoderStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/LogicDeviceStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/RadioDecoderStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/RadioDeviceStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/SignalBufferEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/StorageControlEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/StorageStatusEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/StreamFrameEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/SystemShutdownEvent.cpp
        ${PRIVATE_SOURCE_DIR}/events/SystemStartupEvent.cpp
        ${PRIVATE_SOURCE_DIR}/dialogs/ConfigDialog.cpp
        ${PRIVATE_SOURCE_DIR}/dialogs/InspectDialog.cpp
        ${PRIVATE_SOURCE_DIR}/features/Caps.cpp
        ${PRIVATE_SOURCE_DIR}/format/DataFormat.cpp
        ${PRIVATE_SOURCE_DIR}/graph/AxisLabel.cpp
        ${PRIVATE_SOURCE_DIR}/graph/ChannelGraph.cpp
        ${PRIVATE_SOURCE_DIR}/graph/FrameData.cpp
        ${PRIVATE_SOURCE_DIR}/graph/FrameGraph.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerBracket.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerCursor.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerPeaks.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerRange.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerRibbon.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerValue.cpp
        ${PRIVATE_SOURCE_DIR}/graph/MarkerZoom.cpp
        ${PRIVATE_SOURCE_DIR}/graph/SelectionRect.cpp
        ${PRIVATE_SOURCE_DIR}/graph/TickerFrequency.cpp
        ${PRIVATE_SOURCE_DIR}/graph/TickerTime.cpp
        ${PRIVATE_SOURCE_DIR}/model/StreamFilter.cpp
        ${PRIVATE_SOURCE_DIR}/model/StreamModel.cpp
        ${PRIVATE_SOURCE_DIR}/model/ParserModel.cpp
        ${PRIVATE_SOURCE_DIR}/parser/Parser.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserNfc.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserNfcA.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserNfcB.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserNfcF.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserNfcV.cpp
        ${PRIVATE_SOURCE_DIR}/parser/ParserISO7816.cpp
        ${PRIVATE_SOURCE_DIR}/protocol/ProtocolFrame.cpp
        ${PRIVATE_SOURCE_DIR}/protocol/ProtocolParser.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/AbstractPlotWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/FramesWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/HexViewWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/FrequencyWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/IconDelegate.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/ParserDelegate.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/ParserWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/LogicWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/RadioWidget.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/StreamDelegate.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/StreamHeader.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/StreamMenu.cpp
        ${PRIVATE_SOURCE_DIR}/widgets/StreamWidget.cpp
        ${PRIVATE_SOURCE_DIR}/styles/IconStyle.cpp
        ${PRIVATE_SOURCE_DIR}/styles/Theme.cpp
        ${PRIVATE_SOURCE_DIR}/3party/customplot/QCustomPlot.cpp
        ${PRIVATE_ASSETS_DIR}/theme/dark/icons/icons.qrc
        ${PRIVATE_ASSETS_DIR}/theme/dark/style/style.qrc
        ${PRIVATE_ASSETS_DIR}/theme/light/icons/icons.qrc
        ${PRIVATE_ASSETS_DIR}/theme/light/style/style.qrc
        ${PRIVATE_ASSETS_DIR}/app/app.qrc
)

# Build for WIN32 / UNIX-LINUX
if (WIN32)

    set(PLATFORM_LIBS mingw32 psapi dwmapi)

    if (ENABLE_CONSOLE_OUTPUT)
        message(STATUS "Enable debug output to console.")
        add_executable(nfc-lab ${SOURCES})
        add_compile_definitions(ENABLE_CONSOLE_LOGGING)
    else ()
        add_executable(nfc-lab WIN32 ${SOURCES})
    endif ()

elseif (UNIX)
    add_executable(nfc-lab ${SOURCES})
endif ()

add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

target_include_directories(nfc-lab PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-lab PRIVATE ${AUTOGEN_BUILD_DIR}/include)
target_include_directories(nfc-lab PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(nfc-lab
        ${PLATFORM_LIBS}
        lab-tasks
        lab-logic
        lab-radio
        hw-logic
        hw-radio
        rt-lang
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::PrintSupport
)

# Install targets
if (UNIX)
    # Option to use Flatpak naming convention
    option(FLATPAK_BUILD "Use Flatpak naming convention for desktop files and icons" OFF)

    install(TARGETS nfc-lab DESTINATION bin)

    if(FLATPAK_BUILD)
        # Flatpak build: Use reverse-DNS naming
        set(ICON_NAME "io.github.josevcm.nfc-laboratory")
        set(DESKTOP_FILE_NAME "io.github.josevcm.nfc-laboratory.desktop")
        set(ICON_PNG_NAME "io.github.josevcm.nfc-laboratory.png")
        set(ICON_SVG_NAME "io.github.josevcm.nfc-laboratory.svg")
    else()
        # Normal build: Use original naming
        set(ICON_NAME "nfc-lab")
        set(DESKTOP_FILE_NAME "nfc-lab.desktop")
        set(ICON_PNG_NAME "nfc-lab.png")
        set(ICON_SVG_NAME "nfc-lab.svg")
    endif()

    # Configure desktop file with correct icon name
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/assets/app/rc/nfc-lab.desktop
        ${CMAKE_CURRENT_BINARY_DIR}/nfc-lab.desktop
        @ONLY
    )

    # Install configured desktop file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nfc-lab.desktop
            DESTINATION share/applications
            RENAME ${DESKTOP_FILE_NAME})

    # Install icons
    install(FILES src/main/assets/app/rc/nfc-lab.png
            DESTINATION share/icons/hicolor/512x512/apps
            RENAME ${ICON_PNG_NAME})
    install(FILES src/main/assets/app/rc/nfc-lab.svg
            DESTINATION share/icons/hicolor/scalable/apps
            RENAME ${ICON_SVG_NAME})

    # Install metainfo (only for Flatpak)
    if(FLATPAK_BUILD)
        install(FILES ${CMAKE_SOURCE_DIR}/dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml
                DESTINATION share/metainfo)
    endif()

endif (UNIX)
