include(FeatureSummary)
include(CMakeDependentOption)

cmake_dependent_option(ENABLE_UTILITIES "Enable utility programs" OFF "ENABLE_LIBRARY" OFF)

if (NOT ENABLE_UTILITIES)
    return()
endif()

add_executable(pll_sweep pll_sweep.cpp)
set_target_properties(pll_sweep PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_link_libraries(pll_sweep LimeSuite)







