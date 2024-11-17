set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(dslogic STATIC
        src/main/c/version.c
        src/main/c/strutil.c
        src/main/c/std.c
        src/main/c/session_driver.c
        src/main/c/session.c
        src/main/c/log.c
        src/main/c/hwdriver.c
        src/main/c/error.c
        src/main/c/backend.c
        src/main/c/output/output.c
        src/main/c/input/input.c
        src/main/c/hardware/demo/demo.c
        src/main/c/input/in_binary.c
        src/main/c/input/in_vcd.c
        src/main/c/input/in_wav.c
        src/main/c/output/csv.c
        src/main/c/output/gnuplot.c
        src/main/c/output/srzip.c
        src/main/c/output/vcd.c
        src/main/c/hardware/DSL/dslogic.c
        src/main/c/hardware/common/usb.c
        src/main/c/hardware/common/ezusb.c
        src/main/c/trigger.c
        src/main/c/dsdevice.c
        src/main/c/hardware/DSL/dscope.c
        src/main/c/hardware/DSL/command.c
        src/main/c/hardware/DSL/dsl.c
        src/main/c/lib_main.c
        src/main/c/common/minizip/zip.c
        src/main/c/common/minizip/unzip.c
        src/main/c/common/minizip/ioapi.c
        src/main/c/common/log/xlog.c)

target_include_directories(dslogic PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(dslogic PUBLIC ${PUBLIC_SOURCE_DIR}/common)

target_link_libraries(dslogic ${LIBUSB_LIBRARY} ${LIBG20_LIBRARY})