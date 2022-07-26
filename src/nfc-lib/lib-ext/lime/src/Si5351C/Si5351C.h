/**
@file	Si5351C.h
@brief	Header for Si5351C.cpp
@author	Lime Microsystems
*/

#ifndef SI5351C_MODULE
#define SI5351C_MODULE

#include <stdio.h>
#include <string>
#include "LimeSuiteConfig.h"
//---------------------------------------------------------------------------
namespace lime{

enum eSi_CLOCK_INPUT
{
    Si_CLKIN,
    Si_XTAL,
    Si_CMOS
};

struct Si5351_Channel
{
    Si5351_Channel() : outputDivider(1), outputFreqHz(1), multisynthDivider(1), pllSource(0),
        phaseOffset(0), powered(true), inverted(false), int_mode(false) {};
    int outputDivider;
    unsigned long outputFreqHz;
    float multisynthDivider;
    int pllSource;
    float phaseOffset;
    bool powered;
    bool inverted;
    bool int_mode;
};

struct Si5351_PLL
{
    Si5351_PLL() : inputFreqHz(0), VCO_Hz(0), feedbackDivider(0), CLKIN_DIV(1), CLK_SRC(1) {}
    unsigned long inputFreqHz;
    float VCO_Hz;
    float feedbackDivider;
    int CLKIN_DIV;
    int CLK_SRC; //0-XTAL, 1-CLKIN
};

class IConnection;
class LIME_API Si5351C
{
public:
    enum Status
    {
        SUCCESS,
        FAILED,
    };

    struct StatusBits
    {
        StatusBits() : sys_init(0), sys_init_stky(0), lol_b(0), lol_b_stky(0), lol_a(0), lol_a_stky(0), los(0), los_stky(0)
        {

        }
        int sys_init;
        int sys_init_stky;
        int lol_b;
        int lol_b_stky;
        int lol_a;
        int lol_a_stky;
        int los;
        int los_stky;
    };

    StatusBits GetStatusBits();
    Status ClearStatus();

	Si5351C();
	~Si5351C();
	void Initialize(IConnection *mng);
	bool LoadRegValuesFromFile(std::string FName);

    void SetPLL(unsigned char id, unsigned long CLKIN_Hz, int CLK_SRC);
    void SetClock(unsigned char id, unsigned long fOut_Hz, bool enabled = true, bool inverted = false);

    Status UploadConfiguration();
    Status ConfigureClocks();
	void Reset();

private:
    void FindVCO(Si5351_Channel *clocks, Si5351_PLL *plls, const unsigned long Fmin, const unsigned long Fmax);
    IConnection *device;
    int addrSi5351;

    Si5351_PLL PLL[2];
    Si5351_Channel CLK[8];

	static const unsigned char m_defaultConfiguration[];
	unsigned char m_newConfiguration[255];


};

}
#endif // SI5351C_MODULE
