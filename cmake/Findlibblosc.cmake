
if (LIBBLOSC_LIBRARY AND LIBBLOSC_INCLUDE)
    set(LIBBLOSC_FOUND TRUE)
else (LIBBLOSC_LIBRARY AND LIBBLOSC_INCLUDE)

    find_path(LIBBLOSC_INCLUDE
            NAMES
                blosc.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
    )

    find_library(LIBBLOSC_LIBRARY
            NAMES
                blosc
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
    )

    if (LIBBLOSC_INCLUDE AND LIBBLOSC_LIBRARY)
        set(LIBBLOSC_FOUND TRUE)
    else ()
        set(LIBBLOSC_LIBRARY "blosc")
    endif ()

endif (LIBBLOSC_LIBRARY AND LIBBLOSC_INCLUDE)