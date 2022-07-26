/*
 * File:   LimeSDR_mini.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeNET_micro.h"
#include "Logger.h"
#include "FPGA_Mini.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeNET_micro::LMS7_LimeNET_micro(lime::IConnection* conn, LMS7_Device *obj):
    LMS7_LimeSDR_mini(conn, obj)
{
    if (lms_list[0]->GetReferenceClk_SX(false) < 0)
    {
        lime::info("Reference clock set to 30.72 MHz");
        lms_list[0]->SetReferenceClk_SX(false, 30.72e6);
    }
}

int LMS7_LimeNET_micro::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x2100}, {0x008C, 0x267B}, {0x00A1, 0x656A},
        {0x00A6, 0x0009}, {0x00A7, 0x8A8A}, {0x00A9, 0x8000}, {0x00AC, 0x2000},
        {0x0105, 0x0011}, {0x0108, 0x218C}, {0x0109, 0x6100}, {0x010A, 0x1F4C},
        {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000}, {0x010F, 0x3142},
        {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E}, {0x0113, 0x03C2},
        {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xE6C0}, {0x0121, 0x3650},
        {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B}, {0x020B, 0x4000},
        {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006}, {0x040B, 0x1020},
        {0x040C, 0x00FB}
    };

    lime::LMS7002M* lms = lms_list[0];
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

    if(lms->CalibrateTxGain(0,nullptr) != 0)
        return -1;

    lms->EnableChannel(true, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    lms->SPI_write(0x0123, 0x000F);  //SXT
    lms->SPI_write(0x0120, 0xE6C0);  //SXT
    lms->SPI_write(0x011C, 0x8941);  //SXT
    lms->EnableChannel(false, false);
    lms->EnableChannel(true, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    bool auto_path[2] = {auto_tx_path,auto_rx_path};
    auto_tx_path = false;
    auto_rx_path = false;
    
    if(SetFrequency(true, 0, GetFrequency(true, 0))!=0)
        return -1;
    if(SetFrequency(false, 0, GetFrequency(false, 0))!=0)
        return -1;

    auto_tx_path = auto_path[0];
    auto_rx_path = auto_path[1];
    
    if (SetRate(1e6, 16)!=0)
        return -1;

    return 0;
}

std::vector<std::string> LMS7_LimeNET_micro::GetPathNames(bool dir_tx, unsigned chan) const
{
    if (dir_tx)
        return {"NONE", "BAND1", "BAND2", "Auto"};
    else
	return {"NONE", "LNAH", "LNAL", "LNAW_NC", "Auto"};
}

int LMS7_LimeNET_micro::SetRFSwitch(bool isTx, unsigned path)
{
    int reg3 = fpga->ReadRegister(3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;
    if (isTx==false)
    {
        if (path==LMS_PATH_LNAW)
        {
            lime::warning("LNAW has no connection to RF ports");
        }
        else if (path==LMS_PATH_LNAL)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x0702);
                fpga->WriteRegister(0x17, value | 0x0502);
            }
            else
            {
                value &= ~(3<<8);
                fpga->WriteRegister(0x17, value | (1<<8));
            }
        }
        else if (path==LMS_PATH_LNAH)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x0702);
                fpga->WriteRegister(0x17, value | 0x0602);
            }
            else
            {
                value &= ~(3<<8);
                if (bom_ver == 0)
                    fpga->WriteRegister(0x17, value | (1<<8));
                else
                    fpga->WriteRegister(0x17, value | (2<<8));
            }
        }
    }
    else
    {
        if (path==LMS_PATH_TX1)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x7001);
                fpga->WriteRegister(0x17, value | 0x5000);
            }
            else
            {
                value &= ~(3<<12);
                fpga->WriteRegister(0x17, value | (1<<12));
            }
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {

                value &= ~(0x7001);
                fpga->WriteRegister(0x17, value | 0x6000);
            }
            else
            {
                value &= ~(3<<12);
                if (bom_ver == 0)
                    fpga->WriteRegister(0x17, value | (1<<12));
                else
                    fpga->WriteRegister(0x17, value | (2<<12));
            }
        }
    }
    return 0;
}

int LMS7_LimeNET_micro::AutoRFPath(bool isTx, double f_Hz)
{
    int reg3 = fpga->ReadRegister(3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;
    if (hw_ver < 3 && bom_ver == 0)
        return 0;
    if ((!isTx) && (f_Hz < 1.7e9))
    {
        int ret = 0;
        if (GetPath(false, 0)!= LMS_PATH_LNAL)
        {
            lime::info("Selected RX path: LNAL");
            ret = SetPath(false, 0, LMS_PATH_LNAL);
        }
        auto_rx_path = true;
        return ret;
    }
    return LMS7_LimeSDR_mini::AutoRFPath(isTx, f_Hz);
}

int LMS7_LimeNET_micro::SetClockFreq(unsigned clk_id, double freq, int channel)
{
    return LMS7_Device::SetClockFreq(clk_id, freq, channel);
}

}//namespace lime


