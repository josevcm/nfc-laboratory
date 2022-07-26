/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR.h"
#include "device_constants.h"
#include "FPGA_common.h"

namespace lime
{

LMS7_LimeSDR::LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR::GetProgramModes() const
{
    return {program_mode::autoUpdate,
            program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR::SetRate(double f_Hz, int oversample)
{
    bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);

    for (unsigned i = 0; i < GetNumChannels(false);i++)
    {
        if (rx_channels[i].cF_offset_nco != 0.0 || tx_channels[i].cF_offset_nco != 0.0)
        {
            bypass = false;
            break;
        }
    }

    lime::LMS7002M* lms = lms_list[0];

    if (!bypass)
        return LMS7_Device::SetRate(f_Hz, oversample);

    if ((lms->SetFrequencyCGEN(f_Hz*4) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
       || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), 7, 7) != 0))
       return -1;

    return SetFPGAInterfaceFreq(7, 7);
}

int LMS7_LimeSDR::Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const
{
    int ret = LMS7_Device::Program(mode, data, len, callback);

    if ((mode == program_mode::fx3Flash) || (mode == program_mode::fpgaFlash))
        connection->ProgramWrite(nullptr, 0, 0, 1, nullptr);
    return ret;
}

int LMS7_LimeSDR::EnableChannel(bool dir_tx, unsigned chan, bool enabled)
{
    int ret = LMS7_Device::EnableChannel(dir_tx, chan, enabled);
    if (dir_tx) //always enable DAC1, otherwise sample rates <2.5MHz do not work
        lms_list[0]->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
    return ret;
}

int LMS7_LimeSDR::Calibrate(bool dir_tx, unsigned chan, double bw, unsigned flags)
{
    //switch RF path (may improve things in some configurations)
    uint16_t value = fpga->ReadRegister(0x17);
    fpga->WriteRegister(0x17, (value & (~0x77)) | 0x11);
    int ret = LMS7_Device::Calibrate(dir_tx, chan, bw, flags);
    fpga->WriteRegister(0x17, value);
    return ret;
}

}









