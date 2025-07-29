
if (LIBRTLSDR_LIBRARY AND LIBRTLSDR_INCLUDE)
    set(LIBRTLSDR_FOUND TRUE)
else (LIBRTLSDR_LIBRARY AND LIBRTLSDR_INCLUDE)

    find_path(LIBRTLSDR_INCLUDE
            NAMES
                rtl-sdr.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/rtlsdr/include
            PATH_SUFFIXES
                librtlsdr
    )

    find_library(LIBRTLSDR_LIBRARY
            NAMES
                rtlsdr
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/rtlsdr/x86_64/lib
    )

    if (LIBRTLSDR_INCLUDE AND LIBRTLSDR_LIBRARY)
        set(LIBRTLSDR_FOUND TRUE)
    else ()
        set(LIBRTLSDR_LIBRARY "rtlsdr")
    endif ()

endif (LIBRTLSDR_LIBRARY AND LIBRTLSDR_INCLUDE)