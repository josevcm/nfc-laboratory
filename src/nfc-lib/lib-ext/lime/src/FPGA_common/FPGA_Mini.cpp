#include "FPGA_Mini.h"
#include "IConnection.h"
#include "Logger.h"
#include "LMS64CProtocol.h"
#include <ciso646>
#include <vector>
#include <map>
#include <math.h>

namespace lime
{

FPGA_Mini::FPGA_Mini() : FPGA(){}
/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA_Mini::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel)
{
    int status = 0;

    FPGA_PLL_clock clocks[4];
    if ((txRate_Hz >= 5e6) && (rxRate_Hz >= 5e6))
    {
        clocks[0].bypass = false;
        clocks[0].index = 0;
        clocks[0].outFrequency = txRate_Hz;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].bypass = false;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        clocks[1].findPhase = false;
        clocks[1].phaseShift_deg = txPhase;
        clocks[2].bypass = false;
        clocks[2].index = 2;
        clocks[2].outFrequency = rxRate_Hz;
        clocks[2].phaseShift_deg = 0;
        clocks[2].findPhase = false;
        clocks[3].bypass = false;
        clocks[3].index = 3;
        clocks[3].outFrequency = rxRate_Hz;
        clocks[3].findPhase = false;
        clocks[3].phaseShift_deg = rxPhase;

        status = SetPllFrequency(0, rxRate_Hz, clocks, 4);
    }
    else
    {
        status = SetDirectClocking(0);
        if (status == 0)
            status = SetDirectClocking(1);
    }
    return status;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA_Mini::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int channel)
{
    uint32_t reg20;
    const double rxPhC1 = 89.46;
    const double rxPhC2 = 1.24e-6;
    const double txPhC1 = 89.61;
    const double txPhC2 = 2.71e-7;

    const std::vector<uint32_t> spiAddr = { 0x0021, 0x0022, 0x0023, 0x0024, 0x0027, 0x002A,
                                            0x0400, 0x040C, 0x040B, 0x0400, 0x040B, 0x0400 };
    const int bakRegCnt = spiAddr.size() - 4;

    bool phaseSearch = false;
    if (rxRate_Hz >= 5e6 && txRate_Hz >= 5e6)
        phaseSearch = true;

    if (!phaseSearch)
        return SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, 0);

    std::vector<uint32_t> dataRd;
    std::vector<uint32_t> dataWr;
    dataWr.resize(spiAddr.size());
    dataRd.resize(spiAddr.size());

    //backup registers
    dataWr[0] = (uint32_t(0x0020) << 16);
    connection->ReadLMS7002MSPI(dataWr.data(), &reg20, 1, 0);

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, 0);

    for (int i = 0; i < bakRegCnt; ++i)
        dataWr[i] = (spiAddr[i] << 16);
    connection->ReadLMS7002MSPI(dataWr.data(),dataRd.data(), bakRegCnt, 0);

    {   //Config Rx
        const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4,
                 0xE4E4, 0x0086, 0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED };
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, 0);
    }

    bool phaseSearchSuccess = false;
    lime::FPGA::FPGA_PLL_clock clocks[4];

    for (int i = 0; i < 10; i++)    //attempt phase search 10 times
    {
        clocks[0].index = 3;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[0].phaseShift_deg = rxPhC1 + rxPhC2 * rxRate_Hz;
        clocks[0].findPhase = true;
        clocks[1] = clocks[0];
        clocks[2] = clocks[0];
        clocks[3] = clocks[0];
        if (SetPllFrequency(0, rxRate_Hz, clocks, 4)==0)
        {
            phaseSearchSuccess = true;
	    break;
        }
    }

    if (phaseSearchSuccess)
    {
        //Config TX
        phaseSearchSuccess = false;
        const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0484 };
        WriteRegister(0x000A, 0x0000);
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
                connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, 0);

        for (int i = 0; i < 10; i++)    //attempt phase search 10 times
        {
            clocks[0].index = 1;
            clocks[0].outFrequency = txRate_Hz;
            clocks[0].phaseShift_deg = txPhC1 + txPhC2 * txRate_Hz;
            clocks[0].findPhase = true;
            clocks[1] = clocks[0];
            clocks[2] = clocks[0];
            clocks[3] = clocks[0];
            WriteRegister(0x000A, 0x0200);
            if (SetPllFrequency(0, txRate_Hz, clocks, 4)==0)
            {
                phaseSearchSuccess = true;
                break;
            }
        }
        if (!phaseSearchSuccess)
            lime::error("LML TX phase search FAIL");
    }
    else
        lime::error("LML RX phase search FAIL");


    //Restore registers
    for (int i = 0; i < bakRegCnt; ++i)
        dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRd[i]; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), bakRegCnt, channel);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);
    WriteRegister(0x000A, 0);

    if (!phaseSearchSuccess)
    {
        SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, 0);
        return -1;
    }

    return 0;
}

int FPGA_Mini::UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex)
{
   return ReportError("UploadWFM not supported on LimeSDR-Mini");
}


int FPGA_Mini::ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)
{
    int totalBytesReceived = 0;
    StopStreaming();
#ifdef __unix__
    connection->ResetStreamBuffers();
#endif
    WriteRegister(0x0008, 0x0100 | 0x2);
    WriteRegister(0x0007, 1);

    StartStreaming();

    int handle = connection->BeginDataReading(buffer, length, 0);
    if (connection->WaitForReading(handle, timeout_ms))
        totalBytesReceived = connection->FinishDataReading(buffer, length, handle);

    connection->AbortReading(0);
    StopStreaming();

    return totalBytesReceived;
}



} //namespace lime
