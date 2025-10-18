set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(APP_QT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/nfc-app/app-qt/src/main/cpp)

add_executable(test-parser
        src/main/cpp/main.cpp
        ${APP_QT_SOURCE_DIR}/parser/Parser.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserNfc.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserNfcA.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserNfcB.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserNfcF.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserNfcV.cpp
        ${APP_QT_SOURCE_DIR}/parser/ParserISO7816.cpp
        ${APP_QT_SOURCE_DIR}/protocol/ProtocolFrame.cpp
        ${APP_QT_SOURCE_DIR}/protocol/ProtocolParser.cpp
)

target_include_directories(test-parser PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(test-parser PRIVATE ${AUTOGEN_BUILD_DIR}/include)

# Include Qt and parser directories
target_include_directories(test-parser PRIVATE ${CMAKE_SOURCE_DIR}/src/nfc-app/app-qt/src/main/cpp)

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Link against required libraries
target_link_libraries(test-parser
    ${PLATFORM_LIBS}
    lab-radio
    lab-logic
    hw-radio
    rt-lang
    nlohmann
    Qt6::Core
)
