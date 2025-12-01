set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(hw-radio STATIC
        src/main/cpp/airspy/AirspyDevice.cpp
        src/main/cpp/hydrasdr/HydraDevice.cpp
        src/main/cpp/msi2500/MiriDevice.cpp
        src/main/cpp/rtlsdr/RealtekDevice.cpp)

target_include_directories(hw-radio PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(hw-radio PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(hw-radio rt-lang hw-dev mufft ${LIBAIRSPY_LIBRARY} ${LIBHYDRASDR_LIBRARY} ${LIBRTLSDR_LIBRARY} ${LIBMIRISDR_LIBRARY})
