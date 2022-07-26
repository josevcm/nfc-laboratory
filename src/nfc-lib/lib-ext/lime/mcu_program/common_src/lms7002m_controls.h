#ifndef LMS7002M_CONTROLS_MCU_H
#define LMS7002M_CONTROLS_MCU_H

#define LMS7002M_Rx 0
#define LMS7002M_Tx 1

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void SaveChipState(bool wr);
extern void SetDefaults(uint16_t start, uint16_t end);
extern void SetDefaultsSX();
extern void ClockLogicResets();

extern void SetNCOFrequency(const bool tx, const float freq_Hz, uint8_t index);

extern float_type GetReferenceClk_TSP_MHz(bool tx);
extern float_type GetFrequencyCGEN();
extern uint8_t SetFrequencyCGEN(const float_type freq);
extern float_type GetFrequencySX(const bool Tx);
extern uint8_t SetFrequencySX(const bool Tx, const float_type freq);
extern void EnableChannelPowerControls();
extern void EnableMIMOBuffersIfNecessary();

enum VCO_ID
{
    VCO_CGEN = 0,
    VCO_SXR,
    VCO_SXT
};

extern uint8_t TuneVCO(bool SX); // 0-cgen, 1-SXR, 2-SXT
extern uint16_t pow2(const uint8_t power);

typedef struct
{
    const uint16_t* addr;
    const uint16_t* val;
    const uint16_t* mask;
    const uint8_t cnt;
    const uint16_t* wrOnlyAddr;
    const uint16_t* wrOnlyData;
    const uint8_t wrOnlyAddrCnt;
    const uint8_t wrOnlyDataCnt;
} RegisterBatch;

extern void WriteMaskedRegs(const RegisterBatch ROM* regs);
extern uint8_t GetValueOf_c_ctl_pga_rbb(uint8_t g_pga_rbb);

#ifdef __cplusplus
}
#endif

#endif //LMS7002M_CONTROLS_MCU_H
