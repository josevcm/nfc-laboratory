#ifndef LMS7002M_FILTERS_TUNING_H
#define LMS7002M_FILTERS_TUNING_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t TuneTxFilterFixed(const float_type fixedBandwidth);
extern uint8_t TuneTxFilter(const float_type bandwidth);
extern uint8_t TuneRxFilter(const float_type rx_lpf_freq_RF);

extern uint8_t CalibrateRxFilter(const float_type rx_lpf_freq_RF);

#ifdef __cplusplus
}
#endif


#endif
