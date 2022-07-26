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

LMS7_LimeSDR_PCIE::LMS7_LimeSDR_PCIE(lime::IConnection* conn, LMS7_Device *obj) : LMS7_LimeSDR(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR_PCIE::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR_PCIE::Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const
{
    return LMS7_Device::Program(mode, data, len, callback);
}


}









