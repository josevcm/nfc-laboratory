# nfc-spy
 NFC protocol analyzer via SDR receiver
 
## Description
 By using an SDR receiver it is possible to capture, demodulate and decode the NFC signal between the card and the reader.
 
 I do not have as an objective to explain the NFC norms or modulation techniques, there is a multitude of documentation accessible through Google, i will describe as simply as possible the method that i have used to implement this software.
 
## Signal processing
 The first step is receive the 13.56MHz signal and demodulate to get the baseband ASK stream, for this purpose any SDR device capable of tuning this frequency can be used, i have the fantastic and cheap AirSpy Mini capable of tuning from 27Mhz to 1700Mhz.
 
 However, it is not possible to tune 13.56Mhz with this receiver, instead it is possible to use the second harmonic at 27.12Mhz or third at 40.68Mhz.
 
 Let's see a capture of the signal received in baseband for the REQA command and its response:
 
 ![REQA](/screenshots/nfc-baseband-reqa.png?raw=true "REQA signal capture")

 As can be seen, it is a signal modulated in 100% ASK that corresponds to the REQA 26h command of the NFC specifications, the response of the card uses something called load modulation that manifests as a series of pulses on the main signal after the command.
 
### Demodulation

 Due to the digital nature of the signal I used a technique called symbol correlation which is equivalent to carrying out the convolution of the signal with the shape of each symbol to be detected. Without going into details, the NFC-A modulation is based on 6 symbols: Y, X and Z for commands and E, D, F for card responses (see NFC specifications for complete description).
 
 Demodulation is performed by calculating the correlation for each of these symbols and detecting when the maximum approximation to each of them occurs.
