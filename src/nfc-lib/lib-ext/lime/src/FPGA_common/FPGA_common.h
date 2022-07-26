/**
@file FPGA_common.h
@author Lime Microsystems
@brief Common functions used to work with FPGA
*/

#ifndef FPGA_COMMON_H
#define FPGA_COMMON_H
#include "IConnection.h"
#include <stdint.h>
#include "dataTypes.h"
#include "Streamer.h"
#include <map>

namespace lime
{
     
class LIME_API FPGA
{
public:
    
    FPGA();
    virtual ~FPGA(){};
    void SetConnection(IConnection* conn);
    IConnection* GetConnection() const;
    int StartStreaming();
    int StopStreaming();
    int ResetTimestamp();
    virtual int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex);
    
    struct FPGA_PLL_clock
    {
        FPGA_PLL_clock()
        {
           findPhase = false;
           bypass = false;
           phaseShift_deg = 0;
           index = 0;
        }
        double outFrequency;
        double phaseShift_deg;
        uint8_t index;
        bool bypass;
        bool findPhase;
        double rd_actualFrequency;
    };

    virtual int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0);
    virtual int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0);
    double DetectRefClk(double fx3Clk = 100e6);

    static int FPGAPacketPayload2Samples(const uint8_t* buffer, int bufLen, bool mimo, bool compressed, complex16_t** samples);
    static int Samples2FPGAPacketPayload(const complex16_t* const* samples, int samplesCount, bool mimo, bool compressed, uint8_t* buffer);
    virtual void EnableValuesCache(bool enabled);
    virtual int WriteRegisters(const uint32_t *addrs, const uint32_t *data, unsigned cnt);
    virtual int ReadRegisters(const uint32_t *addrs, uint32_t *data, unsigned cnt);
    int WriteRegister(uint32_t addr, uint32_t val);
    int ReadRegister(uint32_t addr);
protected:
    int SetPllFrequency(uint8_t pllIndex, double inputFreq, FPGA_PLL_clock* outputs, uint8_t clockCount);
    int SetDirectClocking(int clockIndex);
    IConnection* connection;
private:
    virtual int ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms);
    int SetPllClock(int clockIndex, int nSteps, bool waitLock, uint16_t &reg23val);
    bool useCache;
    std::map<uint16_t, uint16_t> regsCache;
};

}
#endif // FPGA_COMMON_H
