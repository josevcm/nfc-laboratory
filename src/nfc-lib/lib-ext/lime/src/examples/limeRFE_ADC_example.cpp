/**
@file   limeRFE_ADC_example.cpp
@author Lime Microsystems (www.limemicro.com)
@brief  LimeRFE ADC read example
*/
#include <lime/LimeSuite.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <cmath>

#include "limeRFE.h"

int openPort(char* portName);
void closePort(int fd);

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Error: Wrong number of parameters\n");
		printf("Usage: limeRFE_ADC_test <LimeRFE COM port>\n");
		printf("Example: limeRFE_ADC_test COM3\n");
		exit(1);
	}

	//Open port
	rfe_dev_t* rfe = RFE_Open(argv[1], nullptr);

	if (rfe == nullptr) {
		std::cout << "Open device failed" << std::endl;
		return -1;
	}
	else {
		std::cout << "Port opened" << std::endl;
	}

	//This example uses RFE_ConfigureState API function. It is possible to use RFE_Configure API function, as illustrated in other LimeRFE examples.

	//Configure LimeRFE to use channel HAM 2m channel in TX mode.
	//RX and TX port is J3.
	//Notch is off. Attenuation is 0.
	//SWR is enabled. SWR source is external (signals from the external coupler are provided, FWD to J18, and REV to J17).

	rfe_boardState state;
	state.channelIDRX = RFE_CID_HAM_0145;
	state.channelIDTX = RFE_CID_HAM_0145;
	state.selPortRX = RFE_PORT_1;
	state.selPortTX = RFE_PORT_1;
	state.mode = RFE_MODE_TX;
	state.notchOnOff = RFE_NOTCH_OFF;
	state.attValue = 0;
	state.enableSWR = RFE_SWR_ENABLE;
	state.sourceSWR = RFE_SWR_SRC_EXT;

	RFE_ConfigureState(rfe, state);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	int adc1, adc2;
	double vADC1, vADC2;
	double intercept, slope, k, correction, additionalCorrection, rlCorrection, pin_dBm, rl_dB;

	//Read ADC values
	RFE_ReadADC(rfe, RFE_ADC1, &adc1);
	RFE_ReadADC(rfe, RFE_ADC2, &adc2);

	//ADC voltage
	vADC1 = RFE_ADC_VREF * (adc1 / pow(2.0, RFE_ADC_BITS));
	vADC2 = RFE_ADC_VREF * (adc2 / pow(2.0, RFE_ADC_BITS));

	intercept = -87;             // From ADL5513 datasheet (https://www.analog.com/media/en/technical-documentation/data-sheets/ADL5513.pdf) for f = 100 MHz
	slope = 21E-3;               // From ADL5513 datasheet (https://www.analog.com/media/en/technical-documentation/data-sheets/ADL5513.pdf) for f = 100 MHz
	k = 2;                       // ADL5513 output voltage is multiplied by 2 before processed by ADC, so ADL5513 output voltage is vADC/k
	correction = 21;             // Calculated correction from the LimeRFE schematic (20 dB attenuator and approx. 1 dB for the connector, switch, etc.)
	additionalCorrection = -2.0; // Additional correction, empirical, sample & frequency dependent

	rlCorrection = -0.5;      // Additional reflection correction, empirical, sample & frequency dependent

	//User should include the losses in the external coupler used

	pin_dBm = intercept + vADC1 / (slope * k) + correction + additionalCorrection;
	rl_dB = vADC2 / (k*slope) + rlCorrection;

	printf("SWR subsystem source set to 'External'\n");
	printf("Power: %f dBm\n", pin_dBm);
	printf("RL: %f dB\n", rl_dB);

	//Configure LimeRFE to use cellular Band 1.
	//RX and TX port is J3.
	//Notch is off. Attenuation is 0.
	//SWR is enabled. SWR source is internal (signals from the internal coupler from the outputs of the cellular TX amplifier are provided).

	state.channelIDRX = RFE_CID_CELL_BAND01;
	state.channelIDTX = RFE_CID_CELL_BAND01;
	state.selPortRX = RFE_PORT_1;
	state.selPortTX = RFE_PORT_1;
	state.mode = RFE_MODE_TXRX;
	state.notchOnOff = RFE_NOTCH_OFF;
	state.attValue = 0;
	state.enableSWR = RFE_SWR_ENABLE;
	state.sourceSWR = RFE_SWR_SRC_CELL;

	RFE_ConfigureState(rfe, state);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Read ADC values
	RFE_ReadADC(rfe, RFE_ADC1, &adc1);
	RFE_ReadADC(rfe, RFE_ADC2, &adc2);

	//ADC voltage
	vADC1 = RFE_ADC_VREF * (adc1 / pow(2.0, RFE_ADC_BITS));
	vADC2 = RFE_ADC_VREF * (adc2 / pow(2.0, RFE_ADC_BITS));

	intercept = -89;             // From ADL5513 datasheet (https://www.analog.com/media/en/technical-documentation/data-sheets/ADL5513.pdf) for f = 2140 MHz
	slope = 21E-3;               // From ADL5513 datasheet (https://www.analog.com/media/en/technical-documentation/data-sheets/ADL5513.pdf) for f = 2140 MHz
	k = 2;                       // ADL5513 output voltage is multiplied by 2 before processed by ADC, so ADL5513 output voltage is vADC/k
	correction = 25;             // Calculated correction from the LimeRFE schematic (25 dB coupler, att. and switch approx. 5 dB, and loss from amplifier output to the connector approx. 5 dB)
	additionalCorrection = 0;    // Additional correction, empirical, sample & frequency dependent

	pin_dBm = intercept + vADC1 / (slope * k) + correction + additionalCorrection;

	printf("SWR subsystem source set to 'Cellular'\n");
	printf("Power: %f dBm\n", pin_dBm);

	//Reset LimeRFE
	RFE_Reset(rfe);

	//Close port
	RFE_Close(rfe);

	return 0;
}