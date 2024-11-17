set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(nfc-rx
        src/main/cpp/main.cpp
        )

target_include_directories(nfc-rx PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(nfc-rx PRIVATE ${AUTOGEN_BUILD_DIR}/include)

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(nfc-rx ${PLATFORM_LIBS} lab-nfc lab-tasks hw-radio rt-lang nlohmann)
