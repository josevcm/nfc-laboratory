#project directory inside LimeSuite/mcu_program
cmake_policy(SET CMP0003 NEW)
#C++11 is a required language feature for this project
set(CMAKE_CXX_STANDARD 11)

cmake_dependent_option(ENABLE_MCU_TESTBENCH "MCU functions testing project" OFF "ENABLE_LIBRARY" OFF)

if(ENABLE_MCU_TESTBENCH)

if(CMAKE_COMPILER_IS_GNUCXX)

    #enable C++11 on older versions of cmake
    if (CMAKE_VERSION VERSION_LESS "3.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11x")
    endif()

    #common warnings enabled
    add_compile_options(-Wall)
endif()

set(HOST_SOURCES
    host_src/mcu.c 
    host_src/main.cpp
    host_src/spi.cpp
    host_src/mcuHexBin.cpp
    common_src/lms7002m_calibrations.c
    common_src/lms7002m_controls.c
    common_src/lms7002m_filters.c
    common_src/lms7002m_agc.c
)

SET_SOURCE_FILES_PROPERTIES( 
	host_src/mcu.c
	common_src/lms7002m_calibrations.c
	common_src/lms7002m_controls.c
    common_src/lms7002m_agc.c
	PROPERTIES LANGUAGE CXX )


include_directories(../src)
include_directories(../src/lms7002m)
include_directories(../src/ConnectionRegistry)
include_directories(../src/ConnectionSTREAM)
include_directories(../src/protocols)
include_directories(common_src)

add_executable(MCU_TestBench ${HOST_SOURCES})
if(WIN32)
target_link_libraries(MCU_TestBench LimeSuite)
else()
target_link_libraries(MCU_TestBench LimeSuite)
endif()
if(WIN32)
    target_link_libraries(MCU_TestBench ${CYAPI_LIBRARIES} SetupAPI)
endif()

endif(ENABLE_MCU_TESTBENCH)
