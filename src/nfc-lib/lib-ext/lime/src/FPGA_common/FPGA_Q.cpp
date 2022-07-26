#include "FPGA_Q.h"
#include "IConnection.h"
#include "Logger.h"
#include "LMS64CProtocol.h"
#include <ciso646>
#include <vector>
#include <map>
#include <math.h>

namespace lime
{

FPGA_Q::FPGA_Q() : FPGA(){}
/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA_Q::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel)
{
    lime::FPGA::FPGA_PLL_clock clocks[2];

    if (channel == 2)
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        return SetPllFrequency(4, 30.72e6, clocks, 2);
    }

    const int pll_ind = (channel == 1) ? 2 : 0;

    clocks[0].index = 0;
    clocks[0].outFrequency = rxRate_Hz;
    clocks[1].index = 1;
    clocks[1].outFrequency = rxRate_Hz;
    clocks[1].phaseShift_deg = rxPhase;
    if (SetPllFrequency(pll_ind+1, rxRate_Hz, clocks, 2)!=0)
        return -1;

    clocks[0].index = 0;
    clocks[0].outFrequency = txRate_Hz;
    clocks[1].index = 1;
    clocks[1].outFrequency = txRate_Hz;
    clocks[1].phaseShift_deg = txPhase;
    if (SetPllFrequency(pll_ind, txRate_Hz, clocks, 2)!=0)
        return -1;

    return 0;
}

int FPGA_Q::WriteRegisters(const uint32_t *addrs, const uint32_t *data, unsigned cnt)
{
    return connection->WriteRegisters(addrs, data, cnt);
}
int FPGA_Q::ReadRegisters(const uint32_t *addrs, uint32_t *data, unsigned cnt)
{
    return connection->ReadRegisters(addrs, data, cnt);
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
/*
int FPGA_Q::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int channel)
{
    const int pll_ind = (channel == 1) ? 2 : 0;
    int status = 0;
    uint32_t reg20;
    const double rxPhC1[] = { 91.08, 89.46 };
    const double rxPhC2[] = { -1 / 6e6, 1.24e-6 };
    const double txPhC1[] = { 89.75, 89.61 };
    const double txPhC2[] = { -3.0e-7, 2.71e-7 };

    const std::vector<uint32_t> spiAddr = { 0x021, 0x022, 0x023, 0x024, 0x027, 0x02A,
                                            0x400, 0x40C, 0x40B, 0x400, 0x40B, 0x400};
    const int bakRegCnt = spiAddr.size() - 4;

    std::vector<uint32_t> dataWr;
    dataWr.push_back(uint32_t(0x002F) << 16);
    uint32_t chipVersion=0;
    connection->ReadLMS7002MSPI(dataWr.data(), &chipVersion, 1, channel);
    dataWr.clear();
    //auto info = GetDeviceInfo();
    bool phaseSearch = false;

    std::vector<uint32_t> dataRd;

    if (phaseSearch)
    {
        dataWr.resize(spiAddr.size());
        dataRd.resize(spiAddr.size());
        //backup registers
        dataWr[0] = (uint32_t(0x0020) << 16);
        connection->ReadLMS7002MSPI(dataWr.data(), &reg20, 1, channel);

        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);

        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (spiAddr[i] << 16);
        connection->ReadLMS7002MSPI(dataWr.data(),dataRd.data(), bakRegCnt, channel);
    }

    if(rxRate_Hz >= 5e6)
    {
        if (phaseSearch)
        {
            const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0086,
                                                    0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED};
            //Load test config
            const int setRegCnt = spiData.size();
            for (int i = 0; i < setRegCnt; ++i)
                dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
            connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
        }
        lime::FPGA::FPGA_PLL_clock clocks[2];
        clocks[0].index = 0;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = rxRate_Hz;
        if (chipVersion == 0x3841)
            clocks[1].phaseShift_deg = rxPhC1[1] + rxPhC2[1] * rxRate_Hz;
        else
            clocks[1].phaseShift_deg = rxPhC1[0] + rxPhC2[0] * rxRate_Hz;
        if (phaseSearch)
            clocks[1].findPhase = true;
        status = SetPllFrequency(pll_ind+1, rxRate_Hz, clocks, 2);
    }
    else
        status = SetDirectClocking(pll_ind+1);

    if(txRate_Hz >= 5e6)
    {
        if (phaseSearch)
        {
            const std::vector<uint32_t> spiData = {0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0484};
            connection->WriteRegister(0x000A, 0x0000);
            //Load test config
            const int setRegCnt = spiData.size();
            for (int i = 0; i < setRegCnt; ++i)
                dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
            connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
        }

        lime::FPGA::FPGA_PLL_clock clocks[2];
        clocks[0].index = 0;
        clocks[0].outFrequency = txRate_Hz;
        clocks[0].phaseShift_deg = 0;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        if (chipVersion == 0x3841)
            clocks[1].phaseShift_deg = txPhC1[1] + txPhC2[1] * txRate_Hz;
        else
            clocks[1].phaseShift_deg = txPhC1[0] + txPhC2[0] * txRate_Hz;

        if (phaseSearch)
        {
            clocks[1].findPhase = true;
            connection->WriteRegister(0x000A, 0x0200);
        }
        status = SetPllFrequency(pll_ind, txRate_Hz, clocks, 2);
    }
    else
        status = SetDirectClocking(pll_ind);

    if (phaseSearch)
    {
        //Restore registers
        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRd[i]; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), bakRegCnt, channel);
        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);
        connection->WriteRegister(0x000A, 0);
    }
    return status;
}


*/



} //namespace lime
