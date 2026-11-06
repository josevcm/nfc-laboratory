set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_executable(test-decoder
        src/main/cpp/main.cpp
)

target_include_directories(test-decoder PRIVATE ${PRIVATE_SOURCE_DIR})

if (WIN32)
    set(PLATFORM_LIBS mingw32 psapi)
endif (WIN32)

target_link_libraries(test-decoder ${PLATFORM_LIBS} lab-radio lab-data hw-dev rt-lang nlohmann)

#-------------------------------------------------------------------------------
# regression tests, run with: ctest --test-dir <build-dir> -R decoder
#-------------------------------------------------------------------------------

# reference captures with expected frames, each capture is a separate test
file(GLOB DECODER_TEST_CAPTURES ${CMAKE_SOURCE_DIR}/wav/test_*.wav)

foreach (capture ${DECODER_TEST_CAPTURES})
    get_filename_component(name ${capture} NAME_WE)
    add_test(NAME decoder-${name} COMMAND test-decoder ${capture})
endforeach ()

# synthetic NFC-A captures generated at test time, require python 3
find_package(Python3 COMPONENTS Interpreter)

if (Python3_Interpreter_FOUND)
    set(SYNTHETIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/synthetic)

    add_test(NAME decoder-synthetic-generate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/nfca_synthetic.py ${SYNTHETIC_DIR})

    set_tests_properties(decoder-synthetic-generate PROPERTIES FIXTURES_SETUP decoder-synthetic)

    foreach (scenario basic fsdi_4096 fsdi_rfu ats_short sblock wtx spurious)
        add_test(NAME decoder-nfca-synthetic-${scenario}
                COMMAND test-decoder ${SYNTHETIC_DIR}/nfca_synthetic_${scenario}.wav)

        set_tests_properties(decoder-nfca-synthetic-${scenario} PROPERTIES FIXTURES_REQUIRED decoder-synthetic TIMEOUT 60)
    endforeach ()
else ()
    message(STATUS "python3 not found, synthetic decoder tests disabled")
endif ()
