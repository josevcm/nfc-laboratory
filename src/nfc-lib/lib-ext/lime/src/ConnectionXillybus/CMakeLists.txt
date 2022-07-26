########################################################################
## Support for PCI-E Xillybus connection
########################################################################
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionXillybus)

set(CONNECTION_XILLYBUS_SOURCES
    ${THIS_SOURCE_DIR}/ConnectionXillybusEntry.cpp
    ${THIS_SOURCE_DIR}/ConnectionXillybus.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_PCIE_XILLYBUS "Enable PCIE Xillybus" ON "ENABLE_LIBRARY" OFF)
add_feature_info(ConnectionXillybus ENABLE_PCIE_XILLYBUS "PCIE Xillybus Connection support")
if (NOT ENABLE_PCIE_XILLYBUS)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_sources(LimeSuite PRIVATE ${CONNECTION_XILLYBUS_SOURCES})

