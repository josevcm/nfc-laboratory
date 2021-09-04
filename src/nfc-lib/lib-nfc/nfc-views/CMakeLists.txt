set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(nfc-views STATIC
        src/main/cpp/views/FrequencyView.cpp
        src/main/cpp/views/FrequencyGrid.cpp
        src/main/cpp/views/FrequencyData.cpp
        src/main/cpp/views/FrequencyPeak.cpp
        src/main/cpp/views/QuadratureView.cpp
        src/main/cpp/views/QuadratureGrid.cpp
        src/main/cpp/views/QuadratureData.cpp
        src/main/cpp/shader/DefaultShader.cpp
        src/main/cpp/shader/QuadratureShader.cpp
        src/main/cpp/shader/EnvelopeShader.cpp
        src/main/cpp/shader/HeatmapShader.cpp
        src/main/cpp/shader/SignalSmoother.cpp
        src/main/cpp/shader/PeakShader.cpp
        )

target_include_directories(nfc-views PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(nfc-views PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(nfc-views nfc-tasks gl-engine sdr-io rt-lang)