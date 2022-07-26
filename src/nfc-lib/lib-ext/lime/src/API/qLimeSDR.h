/*
 * File:   qLimeSDR.h
 * Author: Ignas J
 *
 * Created on April 27, 2018
 */

#ifndef QLIMESDR_H
#define	QLIMESDR_H
#include "lms7_device.h"

namespace lime

{

class LMS7_qLimeSDR : public LMS7_Device
{
public:
    LMS7_qLimeSDR(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    unsigned GetNumChannels(const bool tx = false) const override;
    int EnableChannel(bool dir_tx, unsigned chan, bool enabled) override;
    double GetRate(bool tx, unsigned chan, double *rf_rate_Hz = NULL) const override;
    int SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample = 0) override;
private:
    double dacRate;
    double adcRate;
};

}

#endif	/* QLIMESDR_H */

