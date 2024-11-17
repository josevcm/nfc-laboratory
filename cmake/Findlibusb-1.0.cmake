
if (LIBUSB_LIBRARY AND LIBUSB_INCLUDE)
    set(LIBUSB_FOUND TRUE)
else (LIBUSB_LIBRARY AND LIBUSB_INCLUDE)

    find_path(LIBUSB_INCLUDE
            NAMES
                libusb.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_SOURCE_DIR}/dll/usb-1.0.26/include
            PATH_SUFFIXES
                libusb-1.0
    )

    find_library(LIBUSB_LIBRARY
            NAMES
                usb-1.0
            PATHS
                /usr/local/lib64
                /opt/local/lib64
                /usr/lib64
                /usr/local/lib
                /opt/local/lib
                /usr/lib
                ${CMAKE_SOURCE_DIR}/dll/usb-1.0.26/x86_64/lib
    )

    if (LIBUSB_INCLUDE AND LIBUSB_LIBRARY)
        set(LIBUSB_FOUND TRUE)
    endif (LIBUSB_INCLUDE AND LIBUSB_LIBRARY)

endif (LIBUSB_LIBRARY AND LIBUSB_INCLUDE)