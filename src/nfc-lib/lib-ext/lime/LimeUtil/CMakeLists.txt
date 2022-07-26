########################################################################
# Lime Util -- command line utility, info and connections
########################################################################
cmake_dependent_option(ENABLE_LIME_UTIL "LimeSuite device utility" ON "ENABLE_LIBRARY" OFF)
add_feature_info(LimeUtilCommand ENABLE_LIME_UTIL "Command line device discovery utility")
if(ENABLE_LIME_UTIL)
    add_executable(LimeUtil
        LimeUtil.cpp
        LimeUtilTiming.cpp
        LimeUtilCalSweep.cpp)
    target_link_libraries(LimeUtil LimeSuite)
    install(TARGETS LimeUtil DESTINATION bin)
endif()
