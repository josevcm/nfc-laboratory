set(connection_registry_src_files
    ConnectionHandle.cpp
    ConnectionRegistry.cpp
    IConnection.cpp
)

add_library(connectionRegistry STATIC ${connection_registry_src_files})
target_include_directories(connectionRegistry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET connectionRegistry PROPERTY POSITION_INDEPENDENT_CODE TRUE)
