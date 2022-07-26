#include <stdio.h>
#include "lms7002m_calibrations.h"
#include "lms7002m_controls.h"
#include "lms7002m_filters.h"
#include "spi.h"
#include "mcu.h"
#include "mcuHexBin.h"
#include "MCU_BD.h"

#include "LMSBoards.h"
#include "LMS7002M.h"
#include "IConnection.h"
#include "ConnectionRegistry.h"
#include <chrono>
#include <fstream>
#include <gnuPlotPipe.h>
#include <math.h>
#include "mcu_defines.h"
using namespace std;

lime::IConnection *serPort = nullptr;
lime::LMS7002M lmsControl;

//use the LMS7002M or calibrate directly from Host
static bool useMCU =1;
static bool tx = 1;
static bool filters = 0;
static float FBW = 5e6;
static bool extLoop = false;
extern float RefClk;
extern uint8_t extLoopbackPair;

uint8_t GetExtLoopPair(lime::LMS7002M &ctr, bool calibratingTx)
{
    uint8_t loopPair = 0;
    lime::IConnection* port = ctr.GetConnection();
    if(!port)
        return 0;

    auto devName = port->GetDeviceInfo().deviceName;
    uint8_t activeLNA = ctr.Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE);
    uint8_t activeBand = (ctr.Get_SPI_Reg_bits(LMS7_SEL_BAND2_TRF) << 1 | ctr.Get_SPI_Reg_bits(LMS7_SEL_BAND1_TRF))-1;

    if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR))
        loopPair = 1 << 2 | 0x1; // band2 -> LNAH
    else if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI))
        loopPair = activeBand << 2 | activeLNA;
    return loopPair;
}

int16_t ReadDCCorrector(bool tx, uint8_t channel)
{
    uint16_t addr = tx ? 0x05C3 : 0x05C7;
    addr += channel;
    uint16_t mask = tx ? 0x03FF : 0x003F;
    uint16_t value = 0;

    SPI_write(addr, 0);
    SPI_write(addr, 0x4000);
    value = SPI_read(addr);
    SPI_write(addr, value & ~0xC000);

    if(value & (mask+1))
        return (value & mask) * -1;
    else
        return (value & mask);
}

class BoardLoopbackStore
{
public:
    BoardLoopbackStore(lime::IConnection* port) : port(port)
    {
        if(port)
            port->ReadRegister(LoopbackCtrAddr, mLoopbackState);
    }
    ~BoardLoopbackStore()
    {
        if(port)
            port->WriteRegister(LoopbackCtrAddr, mLoopbackState);
    }
private:
    lime::IConnection* port;
    static const uint32_t LoopbackCtrAddr = 0x0017;
    int mLoopbackState;
};

int SetExtLoopback(lime::IConnection* port, uint8_t ch, bool enable)
{
    //enable external loopback switches
    const uint32_t LoopbackCtrAddr = 0x0017;
    uint32_t value = 0;
    int status;
    status = port->ReadRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return -1;
    auto devName = port->GetDeviceInfo().deviceName;

    if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR))
    {
        const uint16_t mask = 0x7;
        const uint8_t shiftCount = (ch==2 ? 4 : 0);
        value &= ~(mask << shiftCount);
        value |= enable << shiftCount;   //EN_Loopback
        value |= enable << (shiftCount+1); //EN_Attenuator
        value |= !enable << (shiftCount+2); //EN_Shunt
    }
    else if (devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI))
    {
        //EN_Shunt
        value &= ~(1 << 2);
        value |= !enable << 2;

        if(tx)
        {
            uint32_t wr = 0x0103 << 16;
            uint32_t path;
            port->ReadLMS7002MSPI(&wr, &path, 1);
            path >>= 10;
            path &= 0x3;
            if (path==1)
            {
                value &= ~(1<<13);
                value |= 1<<12;

                value &= ~(1<<8); //LNA_H
                value |= 1<<9;
            }
            else if (path==2)
            {
                value &= ~(1<<12);
                value |= 1<<13;

                value &= ~(1<<9); //LNA_W
                value |= 1<<8;
            }
            //value |= enable << 1; //EN_Attenuator
        }
        else
        {
            uint32_t wr = 0x010D << 16;
            uint32_t path;
            port->ReadLMS7002MSPI(&wr, &path, 1);
            path &= ~0x0180;
            path >>= 7;
            if (path==1)
            {
                value &= ~(1<<8);
                value |= 1<<9;
            }
            else if (path==3)
            {
                value &= ~(1<<9);
                value |= 1<<8;
            }
        }
    }
    status = port->WriteRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return -1;
    return status;
}

uint8_t UploadMCUParameters(float ref, float bandwidth)
{
    uint8_t status;
    if((status = MCU_SetParameter(MCU_REF_CLK, ref)) != MCU_NO_ERROR)
        printf("Failed to set Reference Clk\n");
    if((status = MCU_SetParameter(MCU_BW, bandwidth)) != MCU_NO_ERROR)
        printf("Failed to set Bandwidth\n");
    return status;
}

void DCIQ()
{
    BoardLoopbackStore loopbackCache(lmsControl.GetConnection());

    int status;
    float freqStart = 2000e6;
    float freqEnd = freqStart;//1000e6;
    float freqStep = 10e6;

    float freq = freqStart;

    vector<float> vfreqs;
    vector<int> vdci, vdcq, vgi, vgq, vph;

    bool isSetBW = false;

    SetExtLoopback(lmsControl.GetConnection(), 0, extLoop);
    while(freq <= freqEnd)
    {

        vfreqs.push_back(freq);
        //status = SetFrequencySX(true, freq + (tx ? 0 : 1e6));
        //status = SetFrequencySX(false, freq + (tx ? -1e6 : 0));

        auto t1 = chrono::high_resolution_clock::now();
        if(useMCU) //using algorithm inside MCU
        {
            if(!isSetBW)
            {
                if(UploadMCUParameters(RefClk, FBW) != MCU_NO_ERROR)
                    return;
                isSetBW = true;

                if(extLoop)
                {
                    uint8_t loopPair = GetExtLoopPair(lmsControl, tx);
                    status = MCU_SetParameter(MCU_EXT_LOOPBACK_PAIR, loopPair);
                    if(status != 0)
                        printf("Failed to set external loopback pair\n");
                }
            }

            if(tx)
                MCU_RunProcedure(extLoop ? 17 : 1); //initiate Tx calibration
            else
                MCU_RunProcedure(extLoop ? 18 : 2); //initiate Rx calibration
            status = MCU_WaitForStatus(10000); //wait until MCU finishes
        }
        else //calibrating with PC
        {
            if(extLoop)
                extLoopbackPair = GetExtLoopPair(lmsControl, tx);

            if(tx)
                status = CalibrateTx(extLoop);
            else
                status = CalibrateRx(extLoop, false);
        }
        auto t2 = chrono::high_resolution_clock::now();
        long duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

        if(status != MCU_NO_ERROR)
            printf("MCU calibration FAILED : 0x%02X\n", status);

        int16_t dci, dcq, ph, gi, gq;
        dci = ReadDCCorrector(tx, 0);
        dcq = ReadDCCorrector(tx, 1);
        gi = lmsControl.Get_SPI_Reg_bits(tx ? LMS7param(GCORRI_TXTSP) : LMS7param(GCORRI_RXTSP), true);
        gq = lmsControl.Get_SPI_Reg_bits(tx ? LMS7param(GCORRQ_TXTSP) : LMS7param(GCORRQ_RXTSP), true);
        ph = lmsControl.Get_SPI_Reg_bits(tx ? LMS7param(IQCORR_TXTSP) : LMS7param(IQCORR_RXTSP), true);

        ph <<= 4;
        ph >>= 4;
        vdci.push_back(dci);
        vdcq.push_back(dcq);
        vgi.push_back(gi);
        vgq.push_back(gq);
        vph.push_back(ph);

        printf("F: %4.0f MHz, DCI:%3i DCQ:%3i GI:%4i GQ:%4i PH:%4i - %s | %li ms\n",
            freq/1e6, dci, dcq, gi, gq, ph,
            (status == MCU_NO_ERROR ? "OK" : "FAIL"),
            duration);
        freq += freqStep;
    }
    SetExtLoopback(lmsControl.GetConnection(), 0, false);
    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        lmsControl.LoadDC_REG_IQ(true, 0x7FFF, 0x7FFF);
    }
}

void Filters()
{
    int status;
    auto t1 = chrono::high_resolution_clock::now();
    if(useMCU) //using algorithm inside MCU
    {
        if(UploadMCUParameters(RefClk, FBW) != MCU_NO_ERROR)
            return;
        MCU_RunProcedure(tx ? 6 : 5); //initiate calibration
        status = MCU_WaitForStatus(1000); //wait until MCU finishes
    }
    else //calibrating with PC
        status = tx ? TuneTxFilter(FBW) : TuneRxFilter(FBW);

    auto t2 = chrono::high_resolution_clock::now();
    long duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    if(tx)
    {
        int16_t rcal_lpflad = lmsControl.Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB));
        int16_t ccal_lpflad = lmsControl.Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
        int16_t lpfh = lmsControl.Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB));
        printf("Tx filter BW: %.0f MHz, RCAL_LPFLAD: %i, CCAL_LPFLAD: %i, RCAL_LPFH: %i | %li ms (%s)\n",
                FBW/1e6, rcal_lpflad, ccal_lpflad, lpfh, duration,
                status == MCU_NO_ERROR ? "OK" : "FAIL");
    }
    else
    {
        int cfb_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), true);
        int ccomp_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), true);
        int rcomp_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), true);
        int rcc_ctl_lpfl_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), true);
        int c_ctl_lpfl_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), true);
        int rcc_ctl_lpfh_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), true);
        printf("Rx filter BW: %0.f MHz, CFB_TIA: %i, CCOMP: %i, RCOMP: %i, "
            "RCTL_LPFL: %i, RCTL_LPFH: %i, C_CTL_LPFL: %i | %li ms (%s)\n",
            FBW/1e6, cfb_tia_rfe, ccomp_tia_rfe, rcomp_tia_rfe, rcc_ctl_lpfl_rbb,
            rcc_ctl_lpfh_rbb, c_ctl_lpfl_rbb, duration,
            status == MCU_NO_ERROR ? "OK" : "FAIL");
    }

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        lmsControl.LoadDC_REG_IQ(true, 0x7FFF, 0x7FFF);
    }
}

int main(int argc, char** argv)
{
    //connect to first available device
    auto cachedHandles = lime::ConnectionRegistry::findConnections();
    if(cachedHandles.size() > 0)
        serPort = lime::ConnectionRegistry::makeConnection(cachedHandles.at(0));
    if(serPort == nullptr)
        return 0;
    if (serPort != nullptr && !serPort->IsOpen())
    {
        delete serPort;
        printf("failed to open LMS7 control device");
    }
    else
    {
        auto info = serPort->GetDeviceInfo();
        printf("Running tests with %s FW:%s HW:%s\n", info.deviceName.c_str(), info.firmwareVersion.c_str(), info.hardwareVersion.c_str());
    }

    lmsControl.SetConnection(serPort);
    //change SPI switch to BB, just in case it was left for MCU
    lmsControl.SPI_write(0x0006, 0);

    //load initial chip config for testing
    string filename;
    /*if(tx)
        filename = "TxTest.ini";
    else
        filename = "RxTest.ini";*/
    filename = "TxDCTest.ini";
    //filename = "AGC_RSSI_test.ini";
    if(lmsControl.LoadConfig(filename.c_str()) != 0)
    {
        printf("Failed to load .ini file\n");
        lime::ConnectionRegistry::freeConnection(serPort);
        return -1;
    }
    RefClk = lmsControl.GetReferenceClk_SX(false);
    lmsControl.UploadAll();
    //calibrating A channel
    //lmsControl.SetActiveChannel(lime::LMS7002M::Channel::ChB);

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    }

    //lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    //float crestFactor = 1;
    //uint32_t wantedRSSI = 87330 / pow(10.0, (3+crestFactor)/20);
    //RunAGC(wantedRSSI);

    int status = 0;
    if(useMCU)
    {
        uint8_t mcuImage[8192*2];
        uint16_t imgSize = 0;
        status = MCU_HEX2BIN("calibrationsLMS7_MCU.hex", sizeof(mcuImage), mcuImage, &imgSize);
        if(status != 0)
            return status;

        fstream fout;
        fout.open("mcu_bin.txt", ios::out);
        fout << "static const uint8_t mcuImage[] = {";
        for(size_t i = 0; i < sizeof(mcuImage); ++i)
        {
            if(i % 16 == 0)
                fout << endl;
            char ctemp[40];
            sprintf(ctemp, "0x%02X,", mcuImage[i]);
            fout << ctemp;
        }
        fout << "}\n";
        fout.close();

        status = MCU_UploadProgram(mcuImage, sizeof(mcuImage));
        if(status != 0)
        {
            printf("MCU programming failed\n");
            return -1;
        }
    }

    if(filters)
        Filters();
    else
        DCIQ();
    lime::ConnectionRegistry::freeConnection(serPort);
    return 0;
}
