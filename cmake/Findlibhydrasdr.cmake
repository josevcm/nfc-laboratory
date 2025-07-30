
if (LIBHYDRASDR_LIBRARY AND LIBHYDRASDR_INCLUDE)
    set(LIBHYDRASDR_FOUND TRUE)
else (LIBHYDRASDR_LIBRARY AND LIBHYDRASDR_INCLUDE)

    find_path(LIBHYDRASDR_INCLUDE
            NAMES
                hydrasdr.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
            PATH_SUFFIXES
                libairspy
    )

    find_library(LIBHYDRASDR_LIBRARY
            NAMES
                hydrasdr
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
    )

    if (LIBHYDRASDR_INCLUDE AND LIBHYDRASDR_LIBRARY)
        set(LIBHYDRASDR_FOUND TRUE)
    else ()
        set(LIBHYDRASDR_LIBRARY "hydrasdr")
    endif ()

endif (LIBHYDRASDR_LIBRARY AND LIBHYDRASDR_INCLUDE)