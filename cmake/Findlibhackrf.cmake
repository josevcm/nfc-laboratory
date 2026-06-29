
if (LIBHACKRF_LIBRARY AND LIBHACKRF_INCLUDE)
    set(LIBHACKRF_FOUND TRUE)
else (LIBHACKRF_LIBRARY AND LIBHACKRF_INCLUDE)

    find_path(LIBHACKRF_INCLUDE
            NAMES
                hackrf.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/hackrf/include
            PATH_SUFFIXES
                libhackrf
    )

    find_library(LIBHACKRF_LIBRARY
            NAMES
                hackrf
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/hackrf/x86_64/lib
    )

    if (LIBHACKRF_INCLUDE AND LIBHACKRF_LIBRARY)
        set(LIBHACKRF_FOUND TRUE)
    else ()
        set(LIBHACKRF_LIBRARY "hackrf")
    endif ()

endif (LIBHACKRF_LIBRARY AND LIBHACKRF_INCLUDE)
