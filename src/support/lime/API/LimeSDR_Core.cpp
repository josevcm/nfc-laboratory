/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR_Core.h"
#include "FPGA/FPGA_common.h"
#include "device_constants.h"

namespace lime
{

LMS7_CoreSDR::LMS7_CoreSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_LimeSDR(conn, obj)
{
}

std::vector<std::string> LMS7_CoreSDR::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_CoreSDR::SetPath(bool tx, unsigned chan, unsigned path)
{
    if (path >= GetPathNames(tx, chan).size())
        return -1;

    int ret = LMS7_Device::SetPath(tx, chan, path);

    if (tx==false)
    {
        uint16_t value = fpga->ReadRegister(0x17);
        int bit_offset = chan ? 4 : 0;
        value &= ~(0xF<<bit_offset);
        if (path==LMS_PATH_LNAH)
            value |= (1<<bit_offset);
        else if (path==LMS_PATH_LNAW)
            value |= (2<<bit_offset);
        else if (path==LMS_PATH_LNAL)
            value |= (4<<bit_offset);
        ret |= fpga->WriteRegister(0x17, value);
    }
    else
    {
        int bit_offset = chan ? 12 : 8;
        if (path==LMS_PATH_TX1)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(0xF<<bit_offset);
            ret |= fpga->WriteRegister(0x17, value | (1<<bit_offset));
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(0xF<<bit_offset);
            ret |= fpga->WriteRegister(0x17, value);
        }
    }

    return ret;
}

}









