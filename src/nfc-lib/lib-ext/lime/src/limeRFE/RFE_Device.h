/*
 * File:   RFE_Device.h
 * Author: ignas
 *
 * Created on September 4, 2019, 1:18 PM
 */

#ifndef RFE_DEVICE_H
#define RFE_DEVICE_H

#include "limeRFE_constants.h"


class RFE_Device {
public:
    RFE_Device(lms_device_t *d, RFE_COM com);
    RFE_Device(const RFE_Device&) = delete;
    RFE_Device& operator=(const RFE_Device&) = delete;
    ~RFE_Device();
    void OnCalibrate(int ch, bool enable);
    int SetFrequency(bool dirTx, int ch, float freq);
    void AutoFreq(rfe_boardState& state);
    void UpdateState(const rfe_boardState& state);
    void UpdateState(int mode);
    void UpdateState();
    void SetChannels(int rx, int tx);
    lms_device_t *sdrDevice;
	struct RFE_COM com;
	rfe_boardState GetState();
private:
    int rxChannel;
    int txChannel;
    rfe_boardState boardState;
    bool autoRx;
    bool autoTx;
};

#endif /* RFE_DEVICE_H */

