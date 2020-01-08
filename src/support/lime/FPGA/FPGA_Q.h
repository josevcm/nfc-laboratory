/**
@file FPGA_common.h
@author Lime Microsystems
@brief Common functions used to work with FPGA
*/

#ifndef FPGA_Q_H
#define FPGA_Q_H
#include "FPGA_common.h"


namespace lime
{
    
class FPGA_Q : public FPGA
{
public:  
    FPGA_Q();
    virtual ~FPGA_Q(){};
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0) override;
    int WriteRegisters(const uint32_t *addrs, const uint32_t *data, unsigned cnt) override;
    int ReadRegisters(const uint32_t *addrs, uint32_t *data, unsigned cnt) override;
    //int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0)override;
};


}
#endif // FPGA_Q_H
