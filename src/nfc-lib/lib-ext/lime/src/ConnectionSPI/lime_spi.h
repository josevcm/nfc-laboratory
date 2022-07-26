//Lime Microsystems Limited, 
//LimeNET-micro spi stream driver module
//Copyright 2018 Norbertas Kremeris, LIME MICROSYSTEMS LTD.


#ifndef LIME_SPI_H
#define LIME_SPI_H

#include <linux/ioctl.h>

#define MAGIC_NUM 117
#define IOCTL_PROGSTOP _IO(MAGIC_NUM, 1)
#define IOCTL_PROGSTART _IO(MAGIC_NUM, 0)


//file operations on chardevice
#define DEVICE_NAME "lime_spi"
#define CLASS_NAME "lime"

#endif

