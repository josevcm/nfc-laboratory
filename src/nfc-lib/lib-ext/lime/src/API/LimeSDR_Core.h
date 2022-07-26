/*
 * File:   LimeSDR.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_CORE_H
#define	LIMESDR_CORE_H
#include "LimeSDR.h"

namespace lime
{

class LMS7_CoreSDR : public LMS7_LimeSDR
{
public:
    LMS7_CoreSDR(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    int Init() override;
    std::vector<std::string> GetProgramModes() const override;
    int SetPath(bool tx, unsigned chan, unsigned path) override;
};

}

#endif	/* LIMESDR_CORE_H */

