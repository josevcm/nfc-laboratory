
if (LIBAIRSPY_LIBRARY AND LIBAIRSPY_INCLUDE)
    set(LIBAIRSPY_FOUND TRUE)
else (LIBAIRSPY_LIBRARY AND LIBAIRSPY_INCLUDE)

    find_path(LIBAIRSPY_INCLUDE
            NAMES
                airspy.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/airspy/include
            PATH_SUFFIXES
                libairspy
    )

    find_library(LIBAIRSPY_LIBRARY
            NAMES
                airspy
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/airspy/x86_64/lib
    )

    if (LIBAIRSPY_INCLUDE AND LIBAIRSPY_LIBRARY)
        set(LIBAIRSPY_FOUND TRUE)
    else ()
        set(LIBAIRSPY_LIBRARY "airspy")
    endif ()

endif (LIBAIRSPY_LIBRARY AND LIBAIRSPY_INCLUDE)