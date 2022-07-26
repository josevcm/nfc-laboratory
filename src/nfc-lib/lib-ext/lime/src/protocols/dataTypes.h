#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

#include <string.h>
#include <stdint.h>
#include <utility>

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
    uint64_t timestamp; //timestamp of the packet
    uint32_t last; //end index of samples
    uint32_t flags;
    complex16_t* samples;

    SamplesPacket(int size = 0):
        timestamp(0),
        last(0),
        flags(0),
        samples(size ? new complex16_t[size]: nullptr)  {};

    SamplesPacket (SamplesPacket&& pkt)
    {
        timestamp = pkt.timestamp;
        last = pkt.last;
        flags = pkt.flags;
        samples = pkt.samples;
        pkt.samples = nullptr;
    };

    SamplesPacket& operator=(SamplesPacket&& pkt)
    {
        timestamp = pkt.timestamp;
        last = pkt.last;
        flags = pkt.flags;
        std::swap(samples, pkt.samples);
        return *this;
    }
    ~SamplesPacket()
    {
        if (samples)
            delete [] samples;
    };

    SamplesPacket (const SamplesPacket&) = delete;
    SamplesPacket& operator=(const SamplesPacket&) = delete;
};

}// namespace lime

#endif
