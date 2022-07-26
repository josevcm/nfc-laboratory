/*
 * File:   LimeSDR.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_H
#define	LIMESDR_H
#include "LmsGeneric.h"

namespace lime
{

class LMS7_LimeSDR : public LMS7_Generic
{
public:
    LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    int SetRate(double f_Hz, int oversample) override;
    std::vector<std::string> GetProgramModes() const override;
    int Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const override;
    int EnableChannel(bool dir_tx, unsigned chan, bool enabled) override;
    int Calibrate(bool dir_tx, unsigned chan, double bw, unsigned flags) override;
};

}

#endif	/* LIMESDR_H */

