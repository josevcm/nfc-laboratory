/**
@file	ADF4002.h
@author	Lime Microsystems
@brief	Header for ADF_module.cpp
*/

#ifndef ADF_MODULE_H
#define ADF_MODULE_H

#include "LimeSuiteConfig.h"

namespace lime{

class LIME_API ADF4002
{
public:
    ADF4002();
    ~ADF4002();

    void SetFrefFvco(double Fref, double Fvco, int &rcount, int &ncount);
    void SetReferenceCounterLatch(int Ldp, int Abw, int refCounter);
    void SetNCounterLatch(int CPgain, int NCounter);
    void SetFunctionLatch(int currentSetting1, int currentSetting2, int timerCounter, int fastLock, int muxoutControl);
    void SetFunctionLatchRgr(int pdPol, int pd1, int pd2, int counterReset, int cpState);
    void SetInitializationLatch(int currentSetting1, int currentSetting2, int timerCounter, int fastLock, int muxoutControl);
    void SetInitializationLatchRgr(int pdPol, int pd1, int pd2, int counterReset, int cpState);

    void GetConfig(unsigned char data[12]);
    void SetDefaults();

    void MakeData();
    void CalculateRN();

    int txtRCnt;
    int cmbABW;
    int cmbLDP;
    int txtNCnt;
    int cmbCPG;
    int rgrCR_i;
    int rgrPD1_i;
    int rgrPD2_i;
    int cmbMOC_i;
    int rgrPDP_i;
    int rgrCPS_i;
    int cmbTC_i;
    int cmbCS1_i;
    int cmbCS2_i;
    int rgrCR_f;
    int rgrPD1_f;
    int rgrPD2_f;
    int cmbMOC_f;
    int rgrPDP_f;
    int rgrCPS_f;
    int cmbTC_f;
    int cmbCS1_f;
    int cmbCS2_f;
    int cmbFL_f;
    int cmbFL_i;
    double txtFref;
    double txtFvco;

    double lblFcomp;
    double lblFvco;

protected:
    unsigned char m_registers[12];
};

}
#endif // ADF_MODULE_H
