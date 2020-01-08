/**
@file FPGA_common.h
@author Lime Microsystems
@brief Common functions used to work with FPGA
*/

#ifndef FPGA_MINI_H
#define FPGA_MINI_H
#include "FPGA_common.h"


namespace lime
{
    
class FPGA_Mini : public FPGA
{
public:  
    FPGA_Mini();
    virtual ~FPGA_Mini(){};
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0)override;
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0)override;
    int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex) override;
private:
    int ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)override;
};


}
#endif // FPGA_COMMON_H
