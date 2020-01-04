# nfc-spy
 NFC protocol monitor and analyzer via SDR receiver
 
## Description
 By using an SDR receiver it is possible to capture, demodulate and decode the NFC signal between the card and the reader.
 
## Signal processing
 The first step is receive the 13.56 MHz signal from reader, for this purpose any SDR device capable of tuning this frequency can be used, i have the fantastic and cheap AirSpy Mini capable of tuning from 27Mhz to 1700Mhz.
 
 However, it is not possible to tune 13.56Mhz with this receiver, instead it is possible to use the second harmonic at 27.12Mhz or third at 40.68Mhz.
 
 Let's see a capture of the signal received in baseband
