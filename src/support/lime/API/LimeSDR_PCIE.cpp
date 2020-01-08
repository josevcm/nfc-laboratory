/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR_PCIE.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeSDR_PCIE::LMS7_LimeSDR_PCIE(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR_PCIE::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR_PCIE::SetRate(double f_Hz, int oversample)
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

}









