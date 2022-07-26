/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   RFE_Device.cpp
 * Author: ignas
 *
 * Created on September 4, 2019, 1:18 PM
 */

#include "RFE_Device.h"
#include "limeRFE_constants.h"
#include "API/lms7_device.h"

namespace
{
int FreqToBand(float freq)
{
    static constexpr struct {int band; float min; float max;} ranges[] =
    {
            {RFE_CID_HAM_0030, 0, 30e6},
			{RFE_CID_HAM_0070, 50e6, 70e6},
            {RFE_CID_HAM_0145, 140e6, 150e6},
			{RFE_CID_HAM_0220, 220e6, 225e6},
            {RFE_CID_HAM_0435, 400e6, 450e6},
			{RFE_CID_HAM_0920, 902e6, 928e6},
            {RFE_CID_HAM_1280, 1220e6, 1420e6},
            {RFE_CID_HAM_2400, 2.3e9, 2.5e9},
            {RFE_CID_HAM_3500, 3.3e9, 3.7e9},
            {RFE_CID_WB_1000, 1, 1e9},
            {RFE_CID_WB_4000, 100, 4e9}
    };

    for (auto& val : ranges)
        if (freq >= val.min && freq <= val.max)
            return val.band;
    return RFE_CID_WB_4000;
}

int TxPortCheck(int port, int band)
{
	if (port == RFE_PORT_3)
	{
		printf("tx port 3\n");
		if(band == RFE_CID_HAM_0070)
			return RFE_CID_HAM_0070;
		return RFE_CID_HAM_0030;
	}
	if ((band == RFE_CID_HAM_0030) ||
		(band == RFE_CID_HAM_0070))
		return RFE_CID_WB_1000;

    return band;
}

int RxPortCheck(int port, int band)
{
    if (port == RFE_PORT_3 && band != RFE_CID_HAM_0030 && band != RFE_CID_HAM_0070 && band != RFE_CID_HAM_0145 && band != RFE_CID_HAM_0220 && band != RFE_CID_HAM_0435)
        return RFE_CID_WB_1000;
    return band;
}
}

RFE_Device::RFE_Device(lms_device_t *dev, RFE_COM com) :
    sdrDevice(dev),
    com(com),
    rxChannel(0),
    txChannel(0),
    boardState{0},
    autoRx(true),
    autoTx(true)
{
    auto lmsDev = static_cast<lime::LMS7_Device*>(dev);
    if (lmsDev && lmsDev->GetConnection())
        lmsDev->SetLimeRFE(this);
    Cmd_GetConfig(dev, com, &boardState);
}

RFE_Device::~RFE_Device()
{
    auto lmsDev = static_cast<lime::LMS7_Device*>(sdrDevice);
    if (lmsDev && lmsDev->GetLimeRFE() == this)
        lmsDev->SetLimeRFE(nullptr);
}

void RFE_Device::OnCalibrate(int ch, bool en)
{
    static constexpr char cellbands[] = {
        RFE_CID_CELL_BAND01, RFE_CID_CELL_BAND02, RFE_CID_CELL_BAND03,
        RFE_CID_CELL_BAND07, RFE_CID_CELL_BAND38};

    if (ch == txChannel)
    {
        int mode = boardState.mode;
        if (mode != RFE_MODE_NONE)
        {
            for (auto band : cellbands)
                if (boardState.channelIDRX == band)
                {
                    if (!en)
                    {
                        boardState.channelIDRX = RFE_CID_HAM_0145;
                        boardState.channelIDTX = RFE_CID_HAM_0145;
                        boardState.mode = RFE_MODE_NONE;
						Cmd_ConfigureState(sdrDevice, com, boardState);
                        boardState.channelIDRX = band;
                        boardState.channelIDTX = band;
                        boardState.mode = mode;
                    }
                    else
                        Cmd_ConfigureState(sdrDevice, com, boardState);
                    return;
                }

            if (!en)
            {
                Cmd_Mode(sdrDevice, com, RFE_MODE_NONE);
                boardState.mode = mode;
            }
            else
				Cmd_Mode(sdrDevice, com, mode);
        }
    }
}

int RFE_Device::SetFrequency(bool dirTx, int ch, float freq)
{
    if ((dirTx && ch != txChannel) || (!dirTx && ch != rxChannel))
        return 0;

    if (!autoRx && !autoTx)
        return 0;

    if (dirTx && autoTx)
        boardState.channelIDTX = TxPortCheck(boardState.selPortTX,FreqToBand(freq));
    else if (!dirTx && autoRx)
        boardState.channelIDRX = RxPortCheck(boardState.selPortRX,FreqToBand(freq));

    return Cmd_ConfigureState(sdrDevice, com, boardState);
}

void RFE_Device::AutoFreq(rfe_boardState& state)
{

    if (state.channelIDRX == RFE_CID_AUTO)
    {
        autoRx = true;
        if (sdrDevice)
        {
            double freq = -1;
            LMS_GetLOFrequency(sdrDevice, LMS_CH_RX, rxChannel, &freq);
            if (freq > 0)
                state.channelIDRX = RxPortCheck(state.selPortRX,FreqToBand(freq));
        }
        else
            state.channelIDRX = RFE_CID_WB_4000;
    }
    else
        autoRx = false;

    if (state.channelIDTX == RFE_CID_AUTO)
    {
        autoTx = true;
        if (sdrDevice)
        {
            double freq = -1;
            LMS_GetLOFrequency(sdrDevice, LMS_CH_TX, txChannel, &freq);
            if (freq > 0)
                state.channelIDTX = TxPortCheck(state.selPortTX,FreqToBand(freq));
        }
        else
            state.channelIDTX = RFE_CID_WB_4000;
    }
    else
        autoTx = false;
}

void RFE_Device::UpdateState(const rfe_boardState& state)
{
    if (sdrDevice)
        boardState = state;
}

void RFE_Device::UpdateState(int mode)
{
    if (sdrDevice)
        boardState.mode = mode;
}

void RFE_Device::UpdateState()
{
    if (sdrDevice)
        Cmd_GetConfig(sdrDevice, com, &boardState);
}

void RFE_Device::SetChannels(int rx, int tx)
{
    rxChannel = rx;
    txChannel = tx;
}

rfe_boardState RFE_Device::GetState()
{
	rfe_boardState state = boardState;
	return state;
}
