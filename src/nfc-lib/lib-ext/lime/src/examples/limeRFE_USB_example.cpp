/**
@file   limeRFE_USB_example.cpp
@author Lime Microsystems (www.limemicro.com)
@brief  LimeRFE example with I2C communication via LimeSDR
*/
#include <lime/LimeSuite.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>

#include "limeRFE.h"

int openPort(char* portName);
void closePort(int fd);

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Error: Wrong number of parameters\n");
		printf("Usage: limeRFE_USB_test <LimeRFE COM port>\n");
		printf("Example: limeRFE_USB_test COM3\n");
		exit(1);
	}

	//Open port
        rfe_dev_t* rfe = RFE_Open(argv[1], nullptr);

	if (rfe == nullptr) {
		std::cout << "Error: failed to open device" << std::endl;
		return -1;
	}
	else {
		std::cout << "Port opened" << std::endl;
	}

	//Configure LimeRFE to use channel HAM 2m channel in receive mode.
	//Transmit output is routed to TX/RX output. Notch is off. Attenuation is 0.
	RFE_Configure(rfe, RFE_CID_HAM_0145, RFE_CID_HAM_0145, RFE_PORT_1, RFE_PORT_1, RFE_MODE_RX, RFE_NOTCH_OFF, 0, 0, 0);
	//or simpler (by using default arguments)
	//RFE_Configure(NULL, fd, RFE_CID_HAM_0145);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Change mode to transmit
	RFE_Mode(rfe, RFE_MODE_TX);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Reset LimeRFE
	RFE_Reset(rfe);

	//Close port
	RFE_Close(rfe);

	return 0;
}
