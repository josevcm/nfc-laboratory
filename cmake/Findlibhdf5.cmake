
if (LIBHDF5_LIBRARY AND LIBHDF5_INCLUDE)
    set(LIBHDF5_FOUND TRUE)
else (LIBHDF5_LIBRARY AND LIBHDF5_INCLUDE)

    find_path(LIBHDF5_INCLUDE
            NAMES
                hdf5.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/hdf5/include
            PATH_SUFFIXES
                libhdf5
    )

    find_library(LIBHDF5_LIBRARY
            NAMES
                hdf5
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/hdf5/x86_64/lib
    )

    if (LIBHDF5_INCLUDE AND LIBHDF5_LIBRARY)
        set(LIBHDF5_FOUND TRUE)
    else ()
        set(LIBHDF5_LIBRARY "hdf5")
    endif ()

endif (LIBHDF5_LIBRARY AND LIBHDF5_INCLUDE)