import ctypes
from ctypes import *
import sys
import os
import time

import limeRFE_H as LH

if os.name == 'nt':
    libLimeSuite = ctypes.WinDLL ("C:\\Path\\to\\LimeSuite.dll")
else:
    libLimeSuite = ctypes.cdll.LoadLibrary('./libLimeSuite.so')

rfe = c_void_p()

# Open port
if os.name == 'nt':
    rfe = libLimeSuite.RFE_Open(c_char_p("COM24"), None);
else:
    rfe = libLimeSuite.RFE_Open(c_char_p("/dev/ttyUSB0"), None);    

if (rfe == 0):
    print("Error initializing serial port")
    exit(1)

print "Port opened"

#Configure LimeRFE to use channel HAM 2m channel in receive mode.
#Transmit output is routed to TX/RX output. Notch is off. Attenuation is 0.
result = libLimeSuite.RFE_Configure(rfe, c_int(LH.CID_HAM_0145), c_int(LH.CID_HAM_0145), c_int(LH.PORT_1), c_int(LH.PORT_1), c_int(LH.MODE_RX), c_int(LH.NOTCH_VALUE_OFF), c_int(0))
if(result != 0):
    print "RFE_Configure Error Code:", result
    print "Check \'LimeRFE error codes' in LimeRFE.h"
    libLimeSuite.RFE_Close(rfe)
    exit(1)

time.sleep(1.0)

# Change mode to transmit
libLimeSuite.RFE_Mode(rfe, c_int(LH.MODE_TX))

time.sleep(1.0)

# Reset LimeRFE
libLimeSuite.RFE_Reset(rfe)

# Close port
libLimeSuite.RFE_Close(rfe)