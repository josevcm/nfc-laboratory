/*
 * File:   LimeSDR.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_PCIE_H
#define	LIMESDR_PCIE_H
#include "LimeSDR.h"

namespace lime
{

class LMS7_LimeSDR_PCIE : public LMS7_LimeSDR
{
public:
    LMS7_LimeSDR_PCIE(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    std::vector<std::string> GetProgramModes() const override;
    int Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const override;
private:
};

}

#endif	/* LIMESDR_PCIE_H */

