set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(hw-radio STATIC
        src/main/cpp/AirspyDevice.cpp
        src/main/cpp/HydraDevice.cpp
        src/main/cpp/MiriDevice.cpp
        src/main/cpp/RealtekDevice.cpp)

target_include_directories(hw-radio PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(hw-radio PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(hw-radio rt-lang hw-dev mufft mirisdr hydrasdr ${LIBAIRSPY_LIBRARY} ${LIBRTLSDR_LIBRARY})
