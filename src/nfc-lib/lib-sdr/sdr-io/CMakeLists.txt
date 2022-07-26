set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(sdr-io STATIC
        src/main/cpp/AirspyDevice.cpp
        src/main/cpp/FourierTransform.cpp
        src/main/cpp/RealtekDevice.cpp
        src/main/cpp/RecordDevice.cpp
        src/main/cpp/DeviceFactory.cpp
        src/main/cpp/SignalBuffer.cpp)

target_include_directories(sdr-io PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(sdr-io PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(sdr-io rt-lang mufft airspy rtlsdr LimeSuite)