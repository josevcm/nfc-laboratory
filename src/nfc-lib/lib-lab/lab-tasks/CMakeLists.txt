set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(lab-tasks STATIC
        ${PRIVATE_SOURCE_DIR}/tasks/FourierProcessTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/LogicDecoderTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/LogicDeviceTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/RadioDecoderTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/RadioDeviceTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/SignalStreamTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/SignalStorageTask.cpp
        ${PRIVATE_SOURCE_DIR}/tasks/TraceStorageTask.cpp
        )

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    target_compile_options(lab-tasks PRIVATE "-msse2" -DUSE_SSE2)
endif ()

target_include_directories(lab-tasks PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(lab-tasks PRIVATE ${PRIVATE_SOURCE_DIR})

target_link_libraries(lab-tasks lab-logic lab-radio rt-lang hw-radio hw-logic nlohmann microtar z)