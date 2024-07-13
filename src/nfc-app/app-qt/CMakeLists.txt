set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

find_package(Qt5 COMPONENTS Core Widgets PrintSupport REQUIRED)

configure_file(${PRIVATE_SOURCE_DIR}/QtConfig.h.in ${PRIVATE_SOURCE_DIR}/QtConfig.h)

set(SOURCES
        src/main/cpp/main.cpp
        src/main/cpp/main.rc
        src/main/cpp/QtApplication.cpp
        src/main/cpp/QtDecoder.cpp
        src/main/cpp/QtMemory.cpp
        src/main/cpp/QtWindow.cpp
        src/main/cpp/events/DecoderControlEvent.cpp
        src/main/cpp/events/DecoderStatusEvent.cpp
        src/main/cpp/events/ReceiverStatusEvent.cpp
        src/main/cpp/events/ConsoleLogEvent.cpp
        src/main/cpp/events/StorageControlEvent.cpp
        src/main/cpp/events/StorageStatusEvent.cpp
        src/main/cpp/events/StreamFrameEvent.cpp
        src/main/cpp/events/SignalBufferEvent.cpp
        src/main/cpp/events/SystemStartupEvent.cpp
        src/main/cpp/events/SystemShutdownEvent.cpp
        src/main/cpp/dialogs/ConfigDialog.cpp
        src/main/cpp/dialogs/InspectDialog.cpp
        src/main/cpp/graph/QCPAxisTickerFrequency.cpp
        src/main/cpp/graph/QCPAxisRangeMarker.cpp
        src/main/cpp/graph/QCPAxisCursorMarker.cpp
        src/main/cpp/graph/QCPGraphMarkerList.cpp
        src/main/cpp/graph/QCPGraphValueMarker.cpp
        src/main/cpp/model/StreamFilter.cpp
        src/main/cpp/model/StreamModel.cpp
        src/main/cpp/model/ParserModel.cpp
        src/main/cpp/parser/ParserNfc.cpp
        src/main/cpp/parser/ParserNfcA.cpp
        src/main/cpp/parser/ParserNfcB.cpp
        src/main/cpp/parser/ParserNfcF.cpp
        src/main/cpp/parser/ParserNfcV.cpp
        src/main/cpp/protocol/ProtocolFrame.cpp
        src/main/cpp/protocol/ProtocolParser.cpp
        src/main/cpp/widgets/HexViewWidget.cpp
        src/main/cpp/widgets/FramesWidget.cpp
        src/main/cpp/widgets/FourierWidget.cpp
        src/main/cpp/widgets/SignalWidget.cpp
        src/main/cpp/styles/StreamStyle.cpp
        src/main/cpp/styles/ParserStyle.cpp
        src/main/assets/icons/icons.qrc
        src/main/assets/theme/dark/style.qrc
        src/main/cpp/3party/customplot/QCustomPlot.cpp
        )

if(${BUILD_PROJECT_VERSION} EQUAL "0.0.0")
    add_executable(nfc-lab ${SOURCES})
else()
    add_executable(nfc-lab WIN32 ${SOURCES})
endif()

target_include_directories(nfc-lab PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-lab PRIVATE ${AUTOGEN_BUILD_DIR}/include)
target_include_directories(nfc-lab PRIVATE ${LIBUSB_INCLUDE_DIR})

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(nfc-lab
        ${PLATFORM_LIBS}
        nfc-decode
        nfc-tasks
        sdr-io
        rt-lang
        crapto1
        Qt5::Core
        Qt5::Widgets
        Qt5::PrintSupport
        )
