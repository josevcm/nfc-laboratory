#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

#include <string.h>
#include <stdint.h>

namespace lime{

struct FPGA_DataPacket
{
    uint8_t reserved[8];
    uint64_t counter;
    uint8_t data[4080];
};

struct complex16_t
{
    int16_t i;
    int16_t q;
};

const int samples12InPkt = 1360;
const int samples16InPkt = 1020; 

class SamplesPacket
{
public:
    static const int16_t maxSamplesInPacket = samples12InPkt; //total number of samples in all channels combined
    uint64_t timestamp; //timestamp of the packet
    uint16_t first; //index of first unused sample in samples
    uint16_t last; //end index of samples
    complex16_t samples[maxSamplesInPacket];
    uint32_t flags;

    SamplesPacket()
    {
        timestamp = 0;
        first = 0;
        last = 0;
        flags = 0;
    }
};

}// namespace lime

#endif
