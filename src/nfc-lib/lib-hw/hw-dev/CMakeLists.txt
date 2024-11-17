set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(hw-dev STATIC
        src/main/cpp/hw/DeviceFactory.cpp
        src/main/cpp/hw/RecordDevice.cpp
        src/main/cpp/hw/SignalBuffer.cpp
        src/main/cpp/usb/Usb.cpp)

target_include_directories(hw-dev PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(hw-dev PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(hw-dev rt-lang ${LIBUSB_LIBRARY})
