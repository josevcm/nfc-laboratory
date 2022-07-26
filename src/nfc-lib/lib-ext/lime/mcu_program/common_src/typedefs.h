#ifndef LMS7002M_TYPEDEFS_H
#define LMS7002M_TYPEDEFS_H

typedef float float_type;

#ifdef __C51__
    typedef unsigned long uint32_t;
    typedef long int32_t;
    typedef unsigned short uint16_t;
    typedef short int16_t;
    typedef signed char int8_t;
    typedef unsigned char uint8_t;
    typedef bit bool;
    #define ROM code
    #define false 0
    #define true 1
    #define ROM code
#else //for convenience when compiling for PC
    #include <stdint.h>
    #include <stdbool.h>
    //replace with empty space
    #define ROM
    #define xdata
    #define bdata
#endif

#endif
