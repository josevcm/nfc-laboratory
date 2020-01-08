#-------------------------------------------------
# Project created by QtCreator 2019-01-20T00:09:34
#-------------------------------------------------

QT += core gui widgets printsupport

TARGET = nfc-spy
TEMPLATE = app

# emit warnings for deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# general compiler options
QMAKE_CFLAGS   *= -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
QMAKE_CXXFLAGS *= -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable

# optimize more width -O3 to enable vectorization
QMAKE_CFLAGS_RELEASE   -= -O1 -O2
QMAKE_CFLAGS_RELEASE   *= -O3 -msse3 -ffast-math

QMAKE_CXXFLAGS_RELEASE -= -O1 -O2
QMAKE_CXXFLAGS_RELEASE *= -O3 -msse3 -ffast-math

CONFIG += c++11

SOURCES += \
    events/StreamFrameEvent.cpp \
    events/StreamStatusEvent.cpp \
    main.cpp \
    Application.cpp \
    Dispatcher.cpp \
    model/FrameModel.cpp \
    decoder/NfcCapture.cpp \
    decoder/NfcService.cpp \
    decoder/NfcDecoder.cpp \
    decoder/NfcFrame.cpp \
    protocol/ProtocolFrame.cpp \
    protocol/ProtocolParser.cpp \
    devices/AirspyDevice.cpp \
    devices/RealtekDevice.cpp \
    devices/RadioDevice.cpp \
    devices/SignalDevice.cpp \
    devices/RecordDevice.cpp \
    events/ConsoleLogEvent.cpp \
    events/GainControlEvent.cpp \
    events/DecoderControlEvent.cpp \
    events/StorageControlEvent.cpp \
    interface/MainWindow.cpp \
    interface/SetupDialog.cpp \
    interface/PlotMarker.cpp \
    interface/PlotView.cpp \
    storage/StorageReader.cpp \
    storage/StorageService.cpp \
    storage/StorageWriter.cpp \
    support/ElapsedLogger.cpp \
    support/Hex.cpp \
    support/airspy/airspy.c \
    support/airspy/iqconverter_float.c \
    support/airspy/iqconverter_int16.c \
    support/rtlsdr/librtlsdr.c \
    support/rtlsdr/tuner_e4k.c \
    support/rtlsdr/tuner_fc0012.c \
    support/rtlsdr/tuner_fc0013.c \
    support/rtlsdr/tuner_fc2580.c \
    support/rtlsdr/tuner_r82xx.c \
    support/posix/dlfcn.c \
    support/posix/glob.c  \
    support/plot/qcustomplot.cpp

HEADERS += \
    Application.h \
    Dispatcher.h \
    events/StreamFrameEvent.h \
    events/StreamStatusEvent.h \
    model/FrameModel.h \
    decoder/NfcCapture.h \
    decoder/NfcIterator.h \
    decoder/NfcService.h \
    decoder/NfcStream.h \
    protocol/ProtocolFrame.h \
    protocol/ProtocolParser.h \
    devices/AirspyDevice.h \
    devices/RealtekDevice.h \
    devices/RecordDevice.h \
    devices/RadioDevice.h \
    devices/SampleBuffer.h \
    devices/SignalDevice.h \
    decoder/NfcFrame.h \
    decoder/NfcDecoder.h \
    events/ConsoleLogEvent.h \
    events/GainControlEvent.h \
    events/DecoderControlEvent.h \
    events/StorageControlEvent.h \
    interface/MainWindow.h \
    interface/SetupDialog.h \
    interface/PlotMarker.h \
    interface/PlotView.h \
    storage/StorageReader.h \
    storage/StorageService.h \
    storage/StorageWriter.h \
    support/ElapsedLogger.h \
    support/Hex.h \
    support/TaskRunner.h \
    support/airspy/airspy.h \
    support/airspy/airspy_commands.h \
    support/airspy/filters.h \
    support/airspy/iqconverter_float.h \
    support/airspy/iqconverter_int16.h \
    support/rtlsdr/reg_field.h \
    support/rtlsdr/rtl-sdr.h \
    support/rtlsdr/rtl-sdr_export.h \
    support/rtlsdr/rtlsdr_i2c.h \
    support/rtlsdr/tuner_e4k.h \
    support/rtlsdr/tuner_fc0012.h \
    support/rtlsdr/tuner_fc0013.h \
    support/rtlsdr/tuner_fc2580.h \
    support/rtlsdr/tuner_r82xx.h \
    support/posix/dlfcn.h \
    support/posix/glob.h \
    support/plot/qcustomplot.h

FORMS += \
    interface/MainWindow.ui \
    interface/SetupDialog.ui

RESOURCES += \
    resources/icons/icons.qrc \
    resources/style/style.qrc

INCLUDEPATH += $$PWD/3party/ftd3xx-1.2.0.7/include
INCLUDEPATH += $$PWD/3party/libusb-1.0.20/include
INCLUDEPATH += $$PWD/support/airspy
INCLUDEPATH += $$PWD/support/rtlsdr
INCLUDEPATH += $$PWD/support/lime

DEPENDPATH += $$PWD/support/airspy
DEPENDPATH += $$PWD/support/rtlsdr
DEPENDPATH += $$PWD/support/lime

win32: LIBS += -lpsapi
win32: LIBS += $$PWD/3party/ftd3xx-1.2.0.7/win64/dll/FTD3XX.lib
win32: LIBS += $$PWD/3party/libusb-1.0.20/win64/dll/libusb-1.0.lib

unix: LIBS += -lftd3xx
unix: LIBS += -lusb-1.0

