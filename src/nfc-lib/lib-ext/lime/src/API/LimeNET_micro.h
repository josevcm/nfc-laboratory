#ifndef LIMENET_MICRO_H
#define	LIMENET_MICRO_H
#include "LimeSDR_mini.h"

namespace lime {

class LMS7_LimeNET_micro : public LMS7_LimeSDR_mini
{
public:
    LMS7_LimeNET_micro(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    std::vector<std::string> GetPathNames(bool dir_tx, unsigned chan) const override;
    int Init() override;
private:
    int SetClockFreq(unsigned clk_id, double freq, int channel) override;
    int SetRFSwitch(bool tx, unsigned path) override;
    int AutoRFPath(bool isTx, double f_Hz) override;
};

}

#endif	/* LIMENET_MICRO_H */

