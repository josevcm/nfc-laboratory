#ifndef LMS7002M_MCU_INTERFACE_H
#define LMS7002M_MCU_INTERFACE_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void MCU_RunProcedure(uint8_t id);
extern uint8_t MCU_WaitForStatus(uint16_t timeout_ms);
extern uint8_t MCU_UploadProgram(const uint8_t* binImage, const uint16_t len);

enum MCU_Parameter
{
    MCU_REF_CLK,
    MCU_BW,
    MCU_EXT_LOOPBACK_PAIR,
};
extern uint8_t MCU_SetParameter(MCU_Parameter param, float value);

#ifdef __cplusplus
}
#endif

#endif
