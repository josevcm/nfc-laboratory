
if (LIBXTENSOR_LIBRARY AND LIBXTENSOR_INCLUDE)
    set(LIBXTENSOR_FOUND TRUE)
else (LIBXTENSOR_LIBRARY AND LIBXTENSOR_INCLUDE)

    find_path(LIBXTENSOR_INCLUDE
            NAMES
                xtensor.hpp
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
            PATH_SUFFIXES
                xtensor
    )

    find_library(LIBXTENSOR_LIBRARY
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

    if (LIBXTENSOR_INCLUDE AND LIBXTENSOR_LIBRARY)
        set(LIBXTENSOR_FOUND TRUE)
    else ()
        set(LIBXTENSOR_LIBRARY "xtensor")
    endif ()

endif (LIBXTENSOR_LIBRARY AND LIBXTENSOR_INCLUDE)