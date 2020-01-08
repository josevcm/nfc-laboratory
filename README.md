# SDR nfc-signal-monitor 
 A simple NFC signal and protocol sniffer using SDR receiver, capable of demodulate in real-time the comunication with contacless cards up to 424Kpbs.
 
## Description
 By using an SDR receiver it is possible to capture, demodulate and decode the NFC signal between the card and the reader.
 
 I do not have as an objective to explain the NFC norms or modulation techniques, there is a multitude of documentation accessible through Google, i will describe as simply as possible the method that i have used to implement this software.
 
 Currently only detection and decoding for NFC-A modulation has been implemented.
 
## Signal processing
 The first step is receive the 13.56MHz signal and demodulate to get the baseband ASK stream, for this purpose any SDR device capable of tuning this frequency can be used, i have the fantastic and cheap AirSpy Mini capable of tuning from 27Mhz to 1700Mhz. (https://airspy.com/airspy-mini/)
 
 However, it is not possible to tune 13.56Mhz with this receiver, instead i use the second harmonic at 27.12Mhz or third at 40.68Mhz with good results.
 
 Let's see a capture of the signal received in baseband (after I/Q to magnitude transform) for the REQA command and its response:
 
 ![REQA](/doc/nfc-baseband-reqa.png?raw=true "REQA signal capture")

 As can be seen, it is a signal modulated in 100% ASK that corresponds to the REQA 26h command of the NFC specifications, the response of the card uses something called load modulation that manifests as a series of pulses on the main signal after the command.
 
### Demodulation

 Due to the digital nature of the signal i used a technique called symbol correlation which is equivalent to carrying out the convolution of the signal with the shape of each symbol to be detected. Without going into details, the NFC-A modulation is based on 6 patterns: Y, X and Z for reader commands and E, D, F for card responses (see NFC specifications for complete description).
 
 Demodulation is performed by calculating the correlation for these patterns and detecting when the maximum approximation to each of them occurs. Below is the correlation functions for the two basic symbols S0, S1 used to calculate all the others. Last value is function SD represent the absolute difference between S0 and S1 necessary to detect the timmings.
 
 ![CORRELATION](/doc/nfc-decoder-log.png?raw=true "Decoder symbol correlation")

 The response of the card is much weaker but enough to allow its detection using the same technique for patterns E, D, F, here it is shown in better scale.
 
 ![CORRELATION](/doc/nfc-response-log.png?raw=true "Decoder response correlation")
 
### Symbol detection
 
 For the detection of each symbol the value of each correlation is evaluated in the appropriate instants according to the synchronization. The number of samples per symbol is defined as N and must be calculated before starting the process knowing the sampling frequency (fc) and symbol duration.
 
 The correlation process begins with the calculation of the S0 and S1 values that represent the basic symbols subsequently used to discriminate between the NFC patterns X, Y, Z, E, D and F, as shown below.
 
 ![DEC1](/doc/nfc-demodulator-correlation.png?raw=true "Symbols S0 and S1 correlation")
 
 This results in a flow of patterns X, Y, Z, E, D, F that are subsequently interpreted by a state machine in accordance with the specifications of ISO 14443-3 to obtain a byte stream that can be easily processed.
 
 ![DEC2](/doc/nfc-demodulator-pattern-process.png?raw=true "Pattern and frame detection")
  
### Bitrate discrimination

So, we have seen how demodulation is performed, but how does this apply when there are different speeds? Well, since we do not know in advance the transmission speed it is necessary to apply the same process for all possible speeds through a bank of correlators. Really only is necessary to do it for the first symbol of each frame, once the bitrate is known the rest are decoded using that speed.

![DEC3](/doc/nfc-demodulator-speed-detector.png?raw=true "Bitrate discrimination")

### BPSK modulation

 ASK modulation is relatively simple and easy to implement, however the specification ISO 14443 defines the use of BPSK for card responses when the speed is 212Kbps or higher.
 
 For BPSK demodulation a reference signal is required to detect the phase changes (carrier recovery), since that is complex i have chosen to implement it by multiplying each symbol by the preceding one, so that it is possible to determine the value of symbols through the changes produced between then.
 
 ![BPSK1](/doc/nfc-demodulator-bpsk-process.png?raw=true "414Kbps BPSK demodulation process")
 
 Below you can see the signal modulated in BPSK for a response frame at 424Kbps, followed by the demodulation y(t) and integration process over a quarter of a symbol r(t).

 ![BPSK2](/doc/nfc-demodulator-bpsk-detector.png?raw=true "414Kbps BPSK response demodulation")
 
 Finally, by checking if the result is positive or negative, the value of each symbol can be determined. It is somewhat more complex since timing and synchronization must be considered.
 
 ## Application example
 
 An example of the result can be seen below. 
 
 ![APP](/doc/nfc-spy-capture1.png?raw=true "Application example")
 
 Of course, once the frames have been obtained, many improvements can be applied, such as time measurement or protocol details.
 
 ![APP](/doc/nfc-spy-capture2.png?raw=true "Protocol detail example")
 
 Example of PPS command with a speed change from 106Kbps to 424Kbps demonstrating the BPSK demodulation capability.
 
 ![APP](/doc/nfc-spy-capture3.png?raw=true "Protocol detail example")
 
 ## SDR Receivers tested
 
 I have tried several receivers obtaining the best results with AirSpy Mini, I do not have more devices but surely it works with others.
 
 - AirSpy Mini: Better results, tuning the third harmonic 40.68Mhz, with a sampling frequency of 10 Mbps, with these parameters it is possible to capture the communication up to 424 Kbps. 
 
  - RTL SDR: Works tuning the second harmonic 27.12Mhz, due to the limitation in the maximum sampling rate of 3Mbps, it only allows you to capture the commands.
  
  - Lime SDR: I couldn't finish the job with this receiver, maybe one day ...
 
 ## Source code
 
 If you think it is an interesting job or you plan to use it for something please send me an email and let me know, i will be happy to exchange experiences, thank you very much. 
 
 This project is published under the terms of the MIT license, however there are parts of it subject to other types of licenses, please check if you are interested in this work.
 
 - AirSpy SDR driver (src/support/airspy)
 - RTL SDR driver (src/support/rtlsdr)
 - Lime SDR driver (src/support/lime)
 - Posix for windows (src/support/posix)
 - QCustomPlot library (src/support/plot)
 
 ### Build project
 
 This project is based on Qt5 and MinGW, the last release tested has been 5.12.4. Binary files for Windows are included in this repository. I have not tried to compile it on Linux but it should be possible by making some minor adaptations.
 
 If you do not have an SDR receiver, i have included a small capture sample signal in file "wav/capture-424kbps.wav" that serves as an example to test demodulation.
 
 
 
