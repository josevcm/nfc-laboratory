
if (LIBXTENSOR_INCLUDE)
    set(LIBXTENSOR_FOUND TRUE)
else (LIBXTENSOR_INCLUDE)

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

    if (LIBXTENSOR_INCLUDE)
        set(LIBXTENSOR_FOUND TRUE)
    else ()
        set(LIBXTENSOR_LIBRARY "xtensor")
    endif ()

endif (LIBXTENSOR_INCLUDE)