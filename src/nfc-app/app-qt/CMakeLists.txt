set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

find_package(Qt5 COMPONENTS Core Widgets PrintSupport OpenGL REQUIRED)

#add_executable(nfc-lab WIN32
add_executable(nfc-lab
        src/main/cpp/main.cpp
        src/main/cpp/main.rc
        src/main/cpp/QtApplication.cpp
        src/main/cpp/QtDecoder.cpp
        src/main/cpp/QtMemory.cpp
        src/main/cpp/QtResources.cpp
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
        src/main/cpp/graph/QCPAxisTickerFrequency.cpp
        src/main/cpp/graph/QCPAxisRangeMarker.cpp
        src/main/cpp/graph/QCPAxisCursorMarker.cpp
        src/main/cpp/graph/QCPGraphValueMarker.cpp
        src/main/cpp/model/StreamModel.cpp
        src/main/cpp/model/ParserModel.cpp
        src/main/cpp/parser/ParserNfc.cpp
        src/main/cpp/parser/ParserNfcA.cpp
        src/main/cpp/parser/ParserNfcB.cpp
        src/main/cpp/parser/ParserNfcF.cpp
        src/main/cpp/parser/ParserNfcV.cpp
        src/main/cpp/protocol/ProtocolFrame.cpp
        src/main/cpp/protocol/ProtocolParser.cpp
        src/main/cpp/support/QCustomPlot.cpp
        src/main/cpp/widgets/FramesWidget.cpp
        src/main/cpp/widgets/FourierWidget.cpp
        src/main/cpp/widgets/QuadratureWidget.cpp
        src/main/cpp/widgets/FrequencyWidget.cpp
        src/main/cpp/widgets/SignalWidget.cpp
        src/main/cpp/styles/StreamStyle.cpp
        src/main/cpp/styles/ParserStyle.cpp
        src/main/assets/icons/icons.qrc
        src/main/assets/shader/shaders.qrc
        src/main/assets/texture/texture.qrc
        src/main/assets/theme/dark/style.qrc
        )

#target_compile_options(nfc-lab PRIVATE "-fopt-info-vec-optimized")

#target_compile_definitions(nfc-lab PRIVATE QCUSTOMPLOT_USE_OPENGL)

target_include_directories(nfc-lab PRIVATE ${FT_INCLUDE_DIR})
target_include_directories(nfc-lab PRIVATE ${GLEW_INCLUDE_DIR})
target_include_directories(nfc-lab PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-lab PRIVATE ${AUTOGEN_BUILD_DIR}/include)

target_link_libraries(nfc-lab
        nfc-decode
        nfc-tasks
        nfc-views
        gl-engine
        sdr-io
        rt-lang
        mingw32
        opengl32
        psapi
        Qt5::Core
        Qt5::Widgets
        Qt5::PrintSupport
        Qt5::OpenGL
        ${FT_LIBRARY}
        ${GLEW_LIBRARY})
