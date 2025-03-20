set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

find_package(Qt6 COMPONENTS Core Widgets SerialPort REQUIRED)

add_executable(si5451-tx src/main/cpp/main.cpp
        src/main/cpp/SerialSi5351.cpp)

target_include_directories(si5451-tx PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(si5451-tx PRIVATE ${AUTOGEN_BUILD_DIR}/include)

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(si5451-tx ${PLATFORM_LIBS} Qt6::Core Qt6::Widgets Qt6::SerialPort)

# Install targets
if (UNIX)
    install(TARGETS si5451-tx DESTINATION bin)
endif (UNIX)
