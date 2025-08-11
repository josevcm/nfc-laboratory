
if (LIBMIRISDR_LIBRARY AND LIBMIRISDR_INCLUDE)
    set(LIBMIRISDR_FOUND TRUE)
else (LIBMIRISDR_LIBRARY AND LIBMIRISDR_INCLUDE)

    find_path(LIBMIRISDR_INCLUDE
            NAMES
                mirisdr.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/mirisdr/include
            PATH_SUFFIXES
            libmirisdr
    )

    find_library(LIBMIRISDR_LIBRARY
            NAMES
                mirisdr
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/mirisdr/x86_64/lib
    )

    if (LIBMIRISDR_INCLUDE AND LIBMIRISDR_LIBRARY)
        set(LIBMIRISDR_FOUND TRUE)
    else ()
        set(LIBMIRISDR_LIBRARY "mirisdr")
    endif ()

endif (LIBMIRISDR_LIBRARY AND LIBMIRISDR_INCLUDE)