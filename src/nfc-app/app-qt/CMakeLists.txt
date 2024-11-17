set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

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
        src/main/cpp/main.cpp
        src/main/cpp/main.rc
        src/main/cpp/QtApplication.cpp
        src/main/cpp/QtDecoder.cpp
        src/main/cpp/QtServer.cpp
        src/main/cpp/QtWindow.cpp
        src/main/cpp/events/ConsoleLogEvent.cpp
        src/main/cpp/events/DecoderControlEvent.cpp
        src/main/cpp/events/FourierStatusEvent.cpp
        src/main/cpp/events/LogicDecoderStatusEvent.cpp
        src/main/cpp/events/LogicDeviceStatusEvent.cpp
        src/main/cpp/events/RadioDecoderStatusEvent.cpp
        src/main/cpp/events/RadioDeviceStatusEvent.cpp
        src/main/cpp/events/SignalBufferEvent.cpp
        src/main/cpp/events/StorageControlEvent.cpp
        src/main/cpp/events/StorageStatusEvent.cpp
        src/main/cpp/events/StreamFrameEvent.cpp
        src/main/cpp/events/SystemShutdownEvent.cpp
        src/main/cpp/events/SystemStartupEvent.cpp
        src/main/cpp/dialogs/ConfigDialog.cpp
        src/main/cpp/dialogs/InspectDialog.cpp
        src/main/cpp/dialogs/LicenseDialog.cpp
        src/main/cpp/features/Caps.cpp
        src/main/cpp/format/DataFormat.cpp
        src/main/cpp/graph/AxisLabel.cpp
        src/main/cpp/graph/ChannelGraph.cpp
        src/main/cpp/graph/FrameData.cpp
        src/main/cpp/graph/FrameGraph.cpp
        src/main/cpp/graph/MarkerBracket.cpp
        src/main/cpp/graph/MarkerCursor.cpp
        src/main/cpp/graph/MarkerPeaks.cpp
        src/main/cpp/graph/MarkerRange.cpp
        src/main/cpp/graph/MarkerRibbon.cpp
        src/main/cpp/graph/MarkerValue.cpp
        src/main/cpp/graph/MarkerZoom.cpp
        src/main/cpp/graph/SelectionRect.cpp
        src/main/cpp/graph/TickerFrequency.cpp
        src/main/cpp/graph/TickerTime.cpp
        src/main/cpp/model/StreamFilter.cpp
        src/main/cpp/model/StreamModel.cpp
        src/main/cpp/model/ParserModel.cpp
        src/main/cpp/parser/ParserNfc.cpp
        src/main/cpp/parser/ParserNfcA.cpp
        src/main/cpp/parser/ParserNfcB.cpp
        src/main/cpp/parser/ParserNfcF.cpp
        src/main/cpp/parser/ParserNfcV.cpp
        src/main/cpp/parser/ParserISO7816.cpp
        src/main/cpp/protocol/ProtocolFrame.cpp
        src/main/cpp/protocol/ProtocolParser.cpp
        src/main/cpp/widgets/AbstractPlotWidget.cpp
        src/main/cpp/widgets/FramesWidget.cpp
        src/main/cpp/widgets/HexViewWidget.cpp
        src/main/cpp/widgets/FrequencyWidget.cpp
        src/main/cpp/widgets/IconDelegate.cpp
        src/main/cpp/widgets/ParserDelegate.cpp
        src/main/cpp/widgets/ParserWidget.cpp
        src/main/cpp/widgets/LogicWidget.cpp
        src/main/cpp/widgets/RadioWidget.cpp
        src/main/cpp/widgets/StreamDelegate.cpp
        src/main/cpp/widgets/StreamHeader.cpp
        src/main/cpp/widgets/StreamMenu.cpp
        src/main/cpp/widgets/StreamWidget.cpp
        src/main/cpp/styles/IconStyle.cpp
        src/main/cpp/styles/Theme.cpp
        src/main/cpp/3party/customplot/QCustomPlot.cpp
        src/main/assets/theme/dark/icons/icons.qrc
        src/main/assets/theme/dark/style/style.qrc
        src/main/assets/theme/light/icons/icons.qrc
        src/main/assets/theme/light/style/style.qrc
        src/main/assets/app/app.qrc
        src/main/cpp/parser/Parser.cpp
        src/main/cpp/parser/Parser.h
)

# Build for WIN32 / UNIX-LINUX
if (WIN32)

    set(PLATFORM_LIBS mingw32 psapi dwmapi)

    if(ENABLE_CONSOLE_OUTPUT)
        message(STATUS "Enable debug output to console.")
        add_executable(nfc-spy ${SOURCES})
        add_compile_definitions(ENABLE_CONSOLE_LOGGING)
    else()
        add_executable(nfc-spy WIN32 ${SOURCES})
    endif ()

elseif (UNIX)
    add_executable(nfc-spy ${SOURCES})
endif ()

add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

target_include_directories(nfc-spy PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-spy PRIVATE ${AUTOGEN_BUILD_DIR}/include)
target_include_directories(nfc-spy PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(nfc-spy
        ${PLATFORM_LIBS}
        lab-tasks
        lab-logic
        lab-radio
        hw-logic
        hw-radio
        rt-lang
        ed25519
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::PrintSupport)
