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

# synthetic captures generated at test time, require python 3
find_package(Python3 COMPONENTS Interpreter)

if (Python3_Interpreter_FOUND)
    set(SYNTHETIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/synthetic)

    set(NFCA_SYNTHETIC_SCENARIOS basic fsdi_4096 fsdi_rfu ats_short sblock wtx spurious)
    set(NFCB_SYNTHETIC_SCENARIOS basic fsd_4096 fsdi_rfu frame_size egt tr1_short wtx hltb slot_marker no_eos)
    set(NFCF_SYNTHETIC_SCENARIOS basic basic_424 max_length spurious unanswered)
    set(NFCV_SYNTHETIC_SCENARIOS basic code256 large_response rate_20m)

    foreach (tech nfca nfcb nfcf nfcv)
        add_test(NAME decoder-${tech}-synthetic-generate
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${tech}_synthetic.py ${SYNTHETIC_DIR})

        set_tests_properties(decoder-${tech}-synthetic-generate PROPERTIES FIXTURES_SETUP decoder-${tech}-synthetic ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

        string(TOUPPER ${tech} TECH)

        foreach (scenario ${${TECH}_SYNTHETIC_SCENARIOS})
            add_test(NAME decoder-${tech}-synthetic-${scenario}
                    COMMAND test-decoder ${SYNTHETIC_DIR}/${tech}_synthetic_${scenario}.wav)

            set_tests_properties(decoder-${tech}-synthetic-${scenario} PROPERTIES FIXTURES_REQUIRED decoder-${tech}-synthetic TIMEOUT 60)
        endforeach ()
    endforeach ()
else ()
    message(STATUS "python3 not found, synthetic decoder tests disabled")
endif ()
