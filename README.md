# SDR nfc-laboratory 3

[![GitHub Release](https://img.shields.io/github/release/josevcm/nfc-laboratory.svg)](https://github.com/josevcm/nfc-laboratory/releases/latest) 
[![Windows](https://img.shields.io/badge/Windows-Installer-0078D6?logo=windows&logoColor=white)](https://github.com/josevcm/nfc-laboratory/releases/download/3.3.1/nfc-lab-3.3.1-x86_64.exe) 
[![Linux](https://img.shields.io/badge/Linux-Debian%20%2F%20Ubuntu-FCC624?logo=linux&logoColor=black)](https://github.com/josevcm/nfc-laboratory/releases/download/3.3.1/nfc-lab_3.3.1_amd64.deb)

NFC signal sniffer and protocol decoder using SDR receiver for demodulation and decoding NFC-A, NFC-B, NFC-F and NFC-V
signals in real-time up to 424 Kbps. Logic analyzer for contact smart cards with protocol ISO7816.

## Features

- NFC Real-time signal capture and demodulation.
- ISO7816 Real-time signal capture and decode.
- Radio decoder for contactless ISO14443-A, ISO14443-B, ISO15693 and ISO18092.
- Logic decoder for contact smart cards with protocol ISO7816.
- Signal analysis and protocol timing.
- Signal spectrum and wave view.
- Signal frame and protocol detail view.
- Signal export captures to compressed TRZ format.
- Signal import from WAV and compressed TRZ format.
- Support for AirSpy and RTL-SDR receivers.
- Support for DreamSourceLab DSLogic Plus, Pro16 and Pro32 logic analyzer.

## Installation

### Windows

You can install NFC Laboratory using Windows Package Manager:

```
winget install josevcm.nfc-lab
```

Or download the portable installer from [releases](https://github.com/josevcm/nfc-laboratory/releases/latest/).

### Linux

#### Flatpak (recommended)

Once published on Flathub:

```bash
flatpak install flathub io.github.josevcm.nfc-laboratory
flatpak run io.github.josevcm.nfc-laboratory
```

#### Debian/Ubuntu

Download the `.deb` package from [releases](https://github.com/josevcm/nfc-laboratory/releases/latest/) and install:

```bash
sudo dpkg -i nfc-lab_*_amd64.deb
```

For detailed Flatpak build instructions, see [FLATPAK.md](FLATPAK.md).

## Description

By using an SDR receiver it is possible to capture, demodulate and decode the NFC signal between the card and the
reader.

Currently, detection and decoding is implemented for:

- NFC-A (ISO14443A): 106kbps, 212kbps and 424kbps with ASK / BPSK modulation.
- NFC-B (ISO14443B): 106kbps, 212kbps and 424kbps with ASK / BPSK modulation.
- NFC-V (ISO15693): 26kbps and 53kbps, 1 of 4 code and 1 of 256 code PPM / BPSK modulation (pending FSK).
- NFC-F (ISO18092): Preliminary support to 212kbps and 424kbps with manchester modulation.

For contact smart cards, the ISO7816 protocol is implemented with the help of a logic analyzer from DreamSourceLab.

## Application screenshots

Main signal view.

![APP](doc/img/nfc-lab-screenshot0.png "Main Signal view")

![APP](doc/img/nfc-lab-screenshot1.png "Main Signal view")

NFC wave detail view.

![APP](doc/img/nfc-lab-screenshot2.png "Contactless card wave view")

ISO7816 wave detail view.

![APP](doc/img/nfc-lab-screenshot3.png "Contact card wave view")

Radio spectrum view.

![APP](doc/img/nfc-lab-screenshot4.png "Radio spectrum")

Protocol detail view and filtering capabilities.

![APP](doc/img/nfc-lab-screenshot5.png "Protocol detail example")

![APP](doc/img/nfc-lab-screenshot6.png "Protocol detail example")

![APP](doc/img/nfc-lab-screenshot7.png "Protocol detail example")

As can be seen, the application split functionalities in different tabs:

- At the top left:
  - **Frames**: Graphic view for decoded frames captured in contact interface and contactless interface.
  - **Signal**: Shows the raw signal captured with logic analyzer and the radio interface with protocol markers.
  - **Receiver**: During acquire shows the spectrum of the signal captured in the radio interface.

- At the bottom:
  - **Frames**: Table view for decoded frames captured in contact interface and contactless interface.

## Application settings

Settings are stored in user home directory, inside Roaming folder for windows %USERPROFILE%\AppData\Roaming\josevcm\nfc-lab.ini.
The file is created the first time the application is run and can contain the following sections:

Window state, updated every application close.

```
[window]
timeFormat=false
followEnabled=true
filterEnabled=true
windowWidth=1024
windowHeight=700
```

Logic decoder and ISO7816 status, controlled from the toolbar option **Logic Acquire**, **Logic Decoder** under **Features** and **Protocol** menu. 
Currently, channel signal mappings **channelIO**, **channelCLK**, **channelRST**, **channelVCC** are fixed, change this values has no effect.

```
[decoder.logic]
enabled=true

[decoder.logic.protocol.iso7816]
enabled=true
channelIO=0
channelCLK=1
channelRST=2
channelVCC=3
```

Radio decoder and NFC-A, NFC-B, NFC-F, NFC-V status, controlled from the toolbar option **Radio Acquire**, **Radio Decoder** under **Features** and **Protocol** menu.

```
[decoder.radio]
enabled=true

[decoder.radio.protocol.nfca]
enabled=true

[decoder.radio.protocol.nfcb]
enabled=true

[decoder.radio.protocol.nfcf]
enabled=true

[decoder.radio.protocol.nfcv]
enabled=true
```

Configuration parameters for the Airspy receiver, the best performance is obtained by tuning in 3rd harmonic
at 40.68Mhz.

```
[device.radio.airspy]
centerFreq=40680000
sampleRate=10000000
gainMode=1
gainValue=4
mixerAgc=0
tunerAgc=0
biasTee=0
directSampling=0
enabled=true
```


Configuration parameters for the HydraSDR RFOne receiver, the best performance is obtained by tuning in 2rd harmonic
at 27.12Mhz.

```
[device.radio.hydrasdr]
centerFreq=27120000
sampleRate=10000000
gainMode=1
gainValue=2
mixerAgc=0
tunerAgc=0
biasTee=0
directSampling=0
enabled=true
```

Configuration parameters for the RTL-SDR receiver, the best performance is obtained by tuning to the 2nd harmonic
at 27.12Mhz. Decoding with this device is quite limited due to its low sampling frequency and 8-bit resolution,
it will not offer the necessary quality, is supported only as a reference to experiment with it.

```
[device.radio.rtlsdr]
centerFreq=27120000
sampleRate=3200000
gainMode=1
gainValue=125
biasTee=0
directSampling=0
mixerAgc=0
tunerAgc=0
```

Logging control to see what happened.

```
[logger]
root=WARN
app.main=INFO
app.qt=INFO
decoder.IsoDecoder=WARN
decoder.Iso7816=WARN
decoder.NfcDecoder=WARN
decoder.NfcA=WARN
decoder.NfcB=WARN
decoder.NfcF=WARN
decoder.NfcV=WARN
worker.FourierProcess=WARN
worker.LogicDecoder=INFO
worker.LogicDevice=INFO
worker.RadioDecoder=INFO
worker.RadioDevice=INFO
worker.SignalResampling=WARN
worker.SignalStorage=WARN
worker.TraceStorage=WARN
hw.AirspyDevice=WARN
hw.MiriDevice=WARN
hw.RealtekDevice=WARN
hw.RecordDevice=WARN
hw.DSLogicDevice=WARN
hw.DeviceFactory=WARN
hw.UsbContext=WARN
hw.UsbDevice=WARN
rt.Executor=INFO
rt.Worker=INFO
```

All default values are fixed and can be enough for most of the cases.

## SDR Receivers tested

I have tried several receivers obtaining the best results with AirSpy Mini, I do not have more devices, but surely it
works with others.

- HydraSDR RFOne: New SDR receiver with very good results, tuning the second harmonic at 27.12Mhz, with a sampling frequency of 10 Mbps,
  with these parameters it is possible to capture the communication up to 424 Kbps. This is the recommended device
  to use with this tool. Many thanks to Benjamin Vernoux for his support with this new receiver, see [HydraSDR RFOne](https://github.com/hydrasdr).

- AirSpy Mini or R2: Very good results, tuning the third harmonic at 40.68Mhz, with a sampling frequency of 10 Mbps, 
  with these parameters it is possible to capture the communication up to 424 Kbps. This is the recommended device 
  to use with this tool, see [AirSpy](https://github.com/airspy).

- RTL SDR: It works by tuning the second harmonic at 27.12Mhz, due to the limitation in the maximum sampling frequency 
  of 3Mbps and its 8 bits of resolution only allows you to capture the commands up to 106Kbps and some responses in 
  very clean signals with good antenna. This device is supported only as a reference to experiment with it, I not 
  recommend using it if you want to obtain good results.

- RTL SDR BLOG V4: This device is capable if tunning directly at 13.56Mhz, but keeps limitations of legacy RTLSDR in the maximum 
  sampling frequency of 3Mbps and its 8 bits of resolution only allows you to capture the commands up to 106Kbps and some responses in
  very clean signals with good antenna. This device is supported only as a reference to experiment with it, I not
  recommend using it if you want to obtain good results.

Receivers tested, from left to right:

![Devices](doc/img/nfc-lab-devices1.png "Devices")

Nooelec RTL-SDR with HydraNFC calibration coil:

![Devices](doc/img/nfc-lab-devices2.png "Devices")

AirSpy with custom antenna and ARC122U reader:

![Devices](doc/img/nfc-lab-devices3.png "Devices")

HydraSDR RFOne with custom antenna made from RC522 reader PCB:

![Devices](doc/img/nfc-lab-devices8.png "Devices")

### Driver Setup for RTL-SDR

You can found instructions under https://www.rtl-sdr.com/rtl-sdr-quick-start-guide/

### Upconverters & Bias-tee

To avoid tuning harmonics it is possible to use an up-converter and thus tune directly to the carrier 
frequency of 13.56Mhz. Currently, biasTee is supported for AirSpy and HydraSDR RFOne in combination with SpyVerter thanks to [Benjamin DELPY](https://github.com/gentilkiwi). 

The configuration required is:

```
[device.radio.airspy]
centerFreq=133560000
sampleRate=10000000
gainMode=0
gainValue=3
tunerAgc=false
mixerAgc=false
biasTee=1
```

```
[device.radio.hydrasdr]
centerFreq=133560000
sampleRate=10000000
gainMode=0
gainValue=3
tunerAgc=false
mixerAgc=false
biasTee=1
```
### Direct Sampling mode

Another way to avoid using harmonics is activate direct sampling mode and tune to the carrier frequency of 13.56Mhz in those devices that allow it. 
Currently it is only available for RTLSDR thanks to the contribution of [Vincent Långström](https://github.com/vinicentus). You can use direct 
sampling on either the Q- or I-branch. The Q-branch is preferred due to better results, set the Q-branch with directSampling=2 and the I-branch 
with directSampling=1, directSampling=0 turns off direct sampling.

Note: No all RTLSDR devices support this feature.

The configuration required is:

```
[device.rtlsdr]
...
centerFreq=13560000
directSampling=1
...
```

## Logic Analyzer tested

The only tested LA is DreamSourceLab DSLogic Plus, it works perfectly with the app, Pro16 and Pro32 are also supported but not tested (I don't have one).
Firmware files for this LA are included in the repository, you can find them in the **dat/firmware** folder, this files must
be located inside firmware folder along nfc-lab.exe application. Thanks to [DreamSourceLab](https://www.dreamsourcelab.com/product/dslogic-series/).

![Devices](doc/img/nfc-lab-devices4.png "Devices")

The channel connections required to decode ISO7816 protocol are:

- Channel 0: IO
- Channel 1: CLK
- Channel 2: RST
- Channel 3: VCC

![Devices](doc/img/nfc-lab-devices5.png "Devices")

I use a simple adapter to connect the smart card to the logic analyzer from aliexpress, like [this](https://aliexpress.com/item/4000967188424.html) and [this](https://aliexpress.com/item/1005007077428556.html):

![Devices](doc/img/nfc-lab-devices6.png "Devices")

![Devices](doc/img/nfc-lab-devices7.png "Devices")

## Hardware requirements and performance

The demodulator is designed to run in real time, so it requires a recent computer with a lot of processing capacity.

I have opted for a mixed approach where some optimizations are sacrificed in favor of maintaining 
clarity in the code and facilitating its monitoring and debugging.

For this reason it is possible that certain parts can be improved in performance, but I have done it as a didactic 
exercise rather than a production application.

## Input / Output formats

The application allows you to read and write files in two different formats:

- WAV: Reading signals in 16 bits per sample WAV format with 1 or 2 channels for NFC signals and 4 channels for logic analyzer 
  signals. 

  - Radio signal with 1 channel should contain samples in absolute real values. If 2 channels are used they should contain the sampling of the I / Q components.
  - Logic signal with 4 channels should contain the sampling of the IO, CLK, RST and VCC signals in that order.

- TRZ: The analyzed signal can be stored and read from a compressed format based on TGZ with contains:

  - Signal data in custom binary format.
  - Signal metadata in JSON format.

JSON contents are in entry named **frame.json** inside TRZ file:

```
{
   "frames": [
      {
          "dateTime": 1731144155.0108738,
          "frameData": "05:00:08:39:73",
          "frameFlags": 0,
          "framePhase": 257,
          "frameRate": 105938,
          "frameType": 258,
          "sampleEnd": 115545,
          "sampleRate": 10000000,
          "sampleStart": 108739,
          "techType": 258,
          "timeEnd": 0.0115545,
          "timeStart": 0.0108739
      },
...
}
```

- datetime: Date and time of the frame in seconds since epoch
- frameData: Data of the frame in hexadecimal format.
- frameFlags: Flags of the frame, a combination of the following values:
  - ShortFrame = 0x01
  - Encrypted = 0x02
  - Truncated = 0x08
  - ParityError = 0x10
  - CrcError = 0x20
  - SyncError = 0x40
- framePhase: Phase of the frame, one of the following values:
  - NfcAnyPhase = 0x0100
  - NfcCarrierPhase = 0x0101
  - NfcSelectionPhase = 0x0102
  - NfcApplicationPhase = 0x0103
  - IsoAnyPhase = 0x0200
- frameRate: Rate of the frame, in bits per second, one of the following values
- frameType: Type of the frame, one of the following values:
  - NfcCarrierOff = 0x0100
  - NfcCarrierOn = 0x0101
  - NfcPollFrame = 0x0102
  - NfcListenFrame = 0x0103
  - IsoVccLow = 0x200
  - IsoVccHigh = 0x201
  - IsoRstLow = 0x202
  - IsoRstHigh = 0x203
  - IsoATRFrame = 0x0210
  - IsoRequestFrame = 0x0211
  - IsoResponseFrame = 0x0212
  - IsoExchangeFrame = 0x0213
- sampleStart: Start of the frame in samples.
- sampleEnd: End of the frame in samples.
- techType: Type of technology, one of the following values:
  - NoneTech = 0x0000
  - NfcAnyTech = 0x0100
  - NfcATech = 0x0101
  - NfcBTech = 0x0102
  - NfcFTech = 0x0103
  - NfcVTech = 0x0104
  - IsoAnyTech = 0x0200
  - Iso7816Tech = 0x0201
- timeEnd: End of the frame in seconds.
- timeStart: Start of the frame in seconds.

## Live console output

With -j option the application is able to export the decoded frames to JSON format in console output. Thanks to [Steffen-W](https://github.com/Steffen-W) for this feature.

Each JSON frame contains this fields:

| Field                | Type   | Description                                                           |
|----------------------|--------|-----------------------------------------------------------------------|
| `timestamp`          | int    | Frame sample time                                                     |
| `tech`               | string | NfcA, NfcB, NfcF, NfcV, UNKNOWN                                       |
| `type`               | string | CarrierOff, CarrierOn, Poll, Listen                                   |
| `tech_type`          | int    | Tech: 0x0101=NfcA, 0x0102=NfcB, 0x0103=NfcF, 0x0104=NfcV              |
| `frame_type`         | int    | Type: 0x0100=CarrierOff, 0x0101=CarrierOn, 0x0102=Poll, 0x0103=Listen |
| `sample_rate`        | int    | Sample rate in Hz (e.g., 3200000)                                     |
| `sample_start/end`   | int    | Frame sample start / end (relative)                                   |
| `time_start/end`     | float  | Time in seconds (relative)                                            |
| `date_time`          | float  | Absolute Unix timestamp                                               |
| `rate`               | int    | Bitrate in bps                                                        |
| `flags`              | array  | Errors: crc-error, parity-error, truncated, sync-error                |
| `data`               | string | Hex payload: `"AA:BB:CC"` (optional for carrier events)               |
| `length`             | int    | Number of bytes in frameData                                          |

Example:

```
$./nfc-lab -j
{"date_time":1737800643,"frame_type":256,"sample_end":0,"sample_rate":10000000,"sample_start":0,"tech":"UNKNOWN","tech_type":256,"time_end":0,"time_start":0,"timestamp":0,"type":"CarrierOff"}
{"date_time":1737800643,"frame_type":256,"sample_end":1,"sample_rate":10000000,"sample_start":1,"tech":"UNKNOWN","tech_type":256,"time_end":1e-07,"time_start":1e-07,"timestamp":1,"type":"CarrierOff"}
{"date_time":1737800643.0000038,"frame_type":257,"sample_end":38,"sample_rate":10000000,"sample_start":38,"tech":"UNKNOWN","tech_type":256,"time_end":3.8e-06,"time_start":3.8e-06,"timestamp":38,"type":"CarrierOn"}
{"data":"e0:80:31:73","date_time":1737800643.0031676,"flags":["request"],"frame_type":258,"length":4,"rate":105938,"sample_end":35216,"sample_rate":10000000,"sample_start":31677,"tech":"NfcA","tech_type":257,"time_end":0.0035216,"time_start":0.0031677,"timestamp":31677,"type":"Poll"}
{"data":"06:75:77:81:02:80:02:f0","date_time":1737800643.0036075,"flags":["response"],"frame_type":259,"length":8,"rate":105938,"sample_end":42918,"sample_rate":10000000,"sample_start":36075,"tech":"NfcA","tech_type":257,"time_end":0.0042918,"time_start":0.0036075,"timestamp":36075,"type":"Listen"}
{"data":"02:90:5a:00:00:03:ab:22:e5:00:eb:6b","date_time":1737800643.0061498,"flags":["request"],"frame_type":258,"length":12,"rate":105938,"sample_end":71838,"sample_rate":10000000,"sample_start":61497,"tech":"NfcA","tech_type":257,"time_end":0.0071838,"time_start":0.0061497,"timestamp":61497,"type":"Poll"}
{"data":"02:91:00:29:10","date_time":1737800643.0086663,"flags":["response"],"frame_type":259,"length":5,"rate":105938,"sample_end":91003,"sample_rate":10000000,"sample_start":86662,"tech":"NfcA","tech_type":257,"time_end":0.0091003,"time_start":0.0086662,"timestamp":86662,"type":"Listen"}
{"data":"03:90:6c:00:00:01:08:00:67:ce","date_time":1737800643.0235095,"flags":["request"],"frame_type":258,"length":10,"rate":105938,"sample_end":243727,"sample_rate":10000000,"sample_start":235096,"tech":"NfcA","tech_type":257,"time_end":0.0243727,"time_start":0.0235096,"timestamp":235096,"type":"Poll"}
{"data":"03:d4:17:00:00:91:00:3f:fe","date_time":1737800643.0254395,"flags":["response"],"frame_type":259,"length":9,"rate":105938,"sample_end":262136,"sample_rate":10000000,"sample_start":254396,"tech":"NfcA","tech_type":257,"time_end":0.0262136,"time_start":0.0254396,"timestamp":254396,"type":"Listen"}
{"data":"02:90:bd:00:00:07:01:00:00:00:80:00:00:00:1a:83","date_time":1737800643.0412457,"flags":["request"],"frame_type":258,"length":16,"rate":105938,"sample_end":426181,"sample_rate":10000000,"sample_start":412458,"tech":"NfcA","tech_type":257,"time_end":0.0426181,"time_start":0.0412458,"timestamp":412458,"type":"Poll"}
{"data":"02:04:3c:70:02:52:48:80:24:66:4d:fb:bb:a5:78:8d:00:00:45:67:10:10:20:11:00:70:29:d5:f7:1b:00:00:00:00:00:00:00:01:97:3e:07:d2:04:00:00:00:00:00:00:00:00:00:00:00:00:97:3e:00:00:00:91:af:a7:98","date_time":1737800643.0433269,"flags":["response"],"frame_type":259,"length":64,"rate":105938,"sample_end":487734,"sample_rate":10000000,"sample_start":433268,"tech":"NfcA","tech_type":257,"time_end":0.0487734,"time_start":0.0433268,"timestamp":433268,"type":"Listen"}
```

### Live console output with python parsing

Additionally, with the help of the **tools/py_nfclab** module, it is possible to parse and display the frames in a more friendly way. See [tools/py_nfclab/README.md](tools/py_nfclab/README.md) for more information about this module.

```
$ ./nfc-lab -j | python3 -m tools.py_nfclab
NFC Frame Monitor
Live mode - reading from stdin
(Press Ctrl+C to stop)
================================================================================
[    0.000000]   NfcAnyTech        CarrierOff |                  |   0B | (no data)
[    0.000000]   NfcAnyTech        CarrierOff |                  |   0B | (no data)
[    0.000004]   NfcAnyTech         CarrierOn |                  |   0B | (no data)
[    0.003168]         NfcA 105938       Poll | RATS             |   4B | Cmd:E0 Params:80 CRC:3173
[    0.003608]         NfcA 105938     Listen | I-Block          |   8B | Payload:067577810280 CRC:02F0
[    0.006150]         NfcA 105938       Poll | I-Block          |  12B | Cmd:02 Params:905A000003AB22E500 CRC:EB6B
[    0.008666]         NfcA 105938     Listen | I-Block          |   5B | Payload:029100 CRC:2910
[    0.023510]         NfcA 105938       Poll | I-Block          |  10B | Cmd:03 Params:906C0000010800 CRC:67CE
[    0.025440]         NfcA 105938     Listen | I-Block          |   9B | Payload:03D41700009100 CRC:3FFE
[    0.041246]         NfcA 105938       Poll | I-Block          |  16B | Cmd:02 Params:90BD0000070100000080000000 CRC:1A83
[    0.043327]         NfcA 105938     Listen | I-Block          |  64B | Payload[62B]:02043C700252488024664DFBBBA5788D... CRC:A798
```

## Testing files

In the "wav" folder you can find a series of samples of different captures for the NFC-A, NFC-B, NFC-F and NFC-V 
modulations with their corresponding analysis inside the "json" files.

These files can be opened directly from the NFC-LAB application through the toolbar to see their analysis, but the 
main objective is to pass the unit tests and check the correct operation of the decoder.

To run the unit tests, the **test-sdr** artifact must be compiled and launched using the path to the "wav" folder 
as an argument, for example:

```
test-sdr.exe ../wav/
TEST FILE "test_NFC-A_106kbps_001.wav": PASS
TEST FILE "test_NFC-A_106kbps_002.wav": PASS
TEST FILE "test_NFC-A_106kbps_003.wav": PASS
TEST FILE "test_NFC-A_106kbps_004.wav": PASS
TEST FILE "test_NFC-A_212kbps_001.wav": PASS
TEST FILE "test_NFC-A_424kbps_001.wav": PASS
TEST FILE "test_NFC-A_424kbps_002.wav": PASS
TEST FILE "test_NFC-B_106kbps_001.wav": PASS
TEST FILE "test_NFC-B_106kbps_002.wav": PASS
TEST FILE "test_NFC-F_212kbps_001.wav": PASS
TEST FILE "test_NFC-F_212kbps_002.wav": PASS
TEST FILE "test_NFC-V_26kbps_001.wav": PASS
TEST FILE "test_NFC-V_26kbps_002.wav": PASS
TEST FILE "test_POLL_ABF_001.wav": PASS
TEST FILE "test_POLL_AB_001.wav": PASS
```

## Build instructions

This project is based on Qt6 and MinGW-W64, with minimal dependencies.

Contains the following components:

- /src/nfc-app/app-qt: Application interface based on Qt Widgets
- /src/nfc-app/app-rx: Command line decoder application.
- /src/nfc-lib/lib-ext: External libraries and drivers for SDR and logic analyzer.
- /src/nfc-lib/lib-hw: Hardware abstraction layer for SDR and logic analyzer.
- /src/nfc-lib/lib-lab: Signal processing and protocol decoding.
- /src/nfc-lib/lib-rt: Runtime utilities and thread management.

All can be compiled with mingw-g64, a minimum version is required to support C++17, recommended 11.0 or higher.

### Prerequisites

- CMake version 3.16 or higher
  - `winget install -e --id=Kitware.CMake`
  - alternative see http://www.cmake.org/cmake/resources/software.html
- Git-bash or your preferred git client
  - `winget install -e --id Git.Git`
  - alternative see https://gitforwindows.org/
- MSYS2 if you like to install everything with pacma
  - `winget install -e --id MSYS2.MSYS2`
  - add `C:\msys64\mingw64\bin` to the environment variable `Path`
- Qt6 framework 6.x
  - inside MSYS2: `pacman -S mingw-w64-x86_64-qt6-base`
  - alternative see https://www.qt.io/offline-installers
- GCC / G++ for Linux build, version 11.0 or later
  - inside MSYS2: `pacman -S mingw-w64-ucrt-x86_64-gcc`
  - alternative mingw-w64 11 for windows build see https://www.mingw-w64.org/downloads
- USB lib
  - inside MSYS2: `pacman -S mingw-w64-x86_64-libusb`

### Manual build for Windows

Once you have all pre-requisites ready, clone the repository:
```
git clone https://github.com/josevcm/nfc-laboratory.git
```

Create a **build** directory and configure the project (change `CMAKE_BUILD_TYPE=Debug` and `-B cmake-build-debug` for debug output)

```
cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - MinGW Makefiles" -S nfc-laboratory -B build
```

Compile the project:
```
cmake --build build --target nfc-lab -- -j 6
```

```
cmake
[  1%] Building C object src/nfc-lib/lib-ext/microtar/CMakeFiles/microtar.dir/src/main/c/microtar.c.obj
[  2%] Building C object src/nfc-lib/lib-ext/mufft/CMakeFiles/mufft-sse.dir/src/main/c/x86/kernel.sse.c.obj
[  2%] Building C object src/nfc-lib/lib-ext/airspy/CMakeFiles/airspy.dir/src/main/c/airspy.c.obj
....
[ 98%] Linking CXX executable nfc-lab.exe
[100%] Built target nfc-lab
```

Create a coppy of the application for easier access:

```
cp .\build\src\nfc-app\app-qt\nfc-lab.exe nfc-lab.exe
```

Application is ready to use!

If you do not have an SDR receiver, I have included a small capture sample signal in file "wav/capture-424kbps.wav" that
serves as an example to test demodulation.

### Manual build for Linux

Install dependencies (ubuntu)

```
sudo apt install cmake g++ g++-11 qt6-base-dev libqt6svg6 libusb-1.0-0-dev zlib1g-dev libgl1-mesa-dev libairspy-dev librtlsdr-dev
```

Clone the repository
```
git clone https://github.com/josevcm/nfc-laboratory.git
```

Create a **build** directory and configure the project (change `CMAKE_BUILD_TYPE=Debug` and `-B cmake-build-debug` for debug output)

```
cmake -DCMAKE_BUILD_TYPE=Release -S nfc-laboratory -B build
```

Compile the project:

```
cmake --build build --target nfc-lab -- -j$(nproc)
```

Copy the base configuration files to the build directory:

```
cp -r nfc-laboratory/dat/firmware build/src/nfc-app/app-qt/
```

Create a symbolic link to the application for easier access:

```
ln -s build/src/nfc-app/app-qt/nfc-lab nfc-lab
```

Launch the application:

```
./nfc-lab
```

## Source code licensing

If you think it is an interesting job, or you plan to use it for something please email me and let me know, I
will be happy to exchange experiences, thank you very much.

This project is published under the terms of the GPLv3 license, however there are parts of it subject to other types of
licenses, please check if you are interested in this work.

- AirSpy SDR driver at `src/nfc-lib/lib-ext/airspy` see https://github.com/airspy/airspyone_host
- HydraSDR RFOne driver at `src/nfc-lib/lib-ext/hydrasdr` see https://github.com/hydrasdr/rfone_host
- MiriSDR driver at `src/nfc-lib/lib-ext/mirisdr` see https://github.com/f4exb/libmirisdr-4
- RTL SDR driver at `src/nfc-lib/lib-ext/rtlsdr` see https://osmocom.org/projects/rtl-sdr
- mufft library at `src/nfc-lib/lib-ext/mufft` see https://github.com/Themaister/muFFT
- nlohmann json at `src/nfc-lib/lib-ext/nlohmann` see https://github.com/nlohmann/json
- microtar at `src/nfc-lib/lib-ext/microtar` see https://github.com/rxi/microtar
- QCustomPlot at `src/nfc-app/app-qt/src/main/cpp/3party/customplot` see https://www.qcustomplot.com/
- QDarkStyleSheet at `src/nfc-app/app-qt/src/main/assets/theme` see https://github.com/ColinDuquesnoy/QDarkStyleSheet
- Crapto1 at `src/nfc-lib/lib-ext/crapto1`

## Releases

Precompiled installer for x86_64 can be found in repository, you can download latest version from [releases](https://github.com/josevcm/nfc-laboratory/releases/latest/)

# How it works?

## Basic notions of the signals to be analyzed

Normal NFC cards work on the 13.56 Mhz frequency, therefore the first step is receive this signal and demodulate to get
the baseband stream. For this purpose any SDR device capable of tuning this frequency can be used, i have the
fantastic and cheap AirSpy Mini capable of tuning from 24Mhz to 1700Mhz. (https://airspy.com/airspy-mini/)

However, it is not possible to tune 13.56Mhz with this receiver, instead I use the second harmonic at 27.12Mhz or third
at 40.68Mhz with good results.

The received signal will be composed of the I and Q components as in the following image.

![IQ](doc/img/nfc-baseband-iq.png "IQ signal capture")

From these components the real magnitude is calculated using the classic formula sqrt (I ^ 2 + Q ^ 2). Let's see a capture
of the signal received in baseband (after I/Q to magnitude transform) for the REQA command and its response:

![REQA](doc/img/nfc-baseband-reqa.png "REQA signal capture")

As can be seen, it is a signal modulated in 100% ASK that corresponds to the NFC-A REQA 26h command of the NFC 
specifications, the response of the card uses something called load modulation that manifests as a series of pulses on 
the main signal after the command. This is the most basic modulation, but each of the NFC-A / B / F / V standards has 
its own characteristics.

### NFC-A modulation

The standard corresponds to the ISO14443A specifications which describe the way it is modulated as well as the 
applicable timings.

Reader frames are encoded using 100% ASK with modified miller encoding.

![NFCA ASK](doc/img/nfca-ask-miller.png "NFC-A ASK reader frame signal")

When the speed is 106 Kbps card responses are encoded using manchester scheme with OOK load modulation over a 
subcarrier at 848 KHz.

![NFCA OOK](doc/img/nfca-ask-ook.png "NFC-A OOK card response signal")

For higher speeds, 212 kbps, 424 kbps and 848 kbps it uses a NRZ-L with binary phase change modulation, BPSK, over 
same subcarrier.

![NFCA BPSK](doc/img/nfca-bpsk.png "NFC-A BPSK card response signal")

### NFC-B modulation

The standard corresponds to the ISO14443B specifications which describe the way it is modulated as well as the 
applicable timings.

Reader frames are encoded in 10% ASK using NRZ-L encoding.

![NFCB ASK](doc/img/nfcb-ask-nrz.png "NFC-B ASK reader frame signal")

Responses from the card are encoded with binary phase change modulation, BPSK, using NRZ-L encoding.

![NFCB ASK](doc/img/nfcb-bpsk.png "NFC-B BPSK card response signal")

### NFC-F modulation

The standard corresponds to the ISO18092 and JIS.X.6319 specifications which describe the way it is modulated as well 
as the applicable timings.

Support speeds from 212 kbps to 848 kbps, both reader and card frames are encoded using either observed or reversed 
manchester as see below.

![NFCF Manchester](doc/img/nfcf-manchester.png "NFC-F manchester reader frame signal")

Observed manchester modulation.

![NFCF OBSERVE](doc/img/nfcf-observe.png "NFC-F manchester observed modulation")

Reversed manchester modulation.

![NFCF REVERSE](doc/img/nfcf-reverse.png "NFC-B manchester reversed modulation")

### NFC-V modulation

The standard corresponds to the ISO15693 specifications which describe the way it is modulated as well as the 
applicable timings.

The coding is based on pulse position modulation (PPM) where the information is encoded by modifying the time when the 
pulse is located within each time slot.

There are two modes, 1 of 4 and 1 of 256, where each symbol encodes 2 and 8 bits respectively, this is the example for 
the first one.

![NFCV PPM 2 bit](doc/img/nfcv-ppm2.png "NFC-V PPM reader modulation")

Card responses can be encoded in two different ways depending on the value of bit 0 in the flags field of the request made by VCD. 

If the flags bit 0 = 0, the card will respond using only one subcarrier at fc/32 (423,75 kHz), with OOK modulation, (as in NFC-A). 
If the flags bit 0 = 1, the card will respond using two subcarriers at fc/32 (423,75 kHz) and fc/28 (484,28 kHz) with 2-FSK modulation. 

NOTE: Currently only the first mode is supported in this software (sorry).

Depending on the encoding, the possible speeds are 26Kbps and 53Kbps, however these cards can be read from greater 
distances.

## Signal processing

Now we are going to see how to decode this.

### First step, prepare base signals

Before starting to decode each of these modulations, it is necessary to start with a series of basic signals that will
help us in the rest of the process.

The concepts that I am going to explain next are very well described on Sam Koblenski's page 
(https://sam-koblenski.blogspot.com/2015/08/everyday-dsp-for-programmers-basic.html) which I recommend you read to 
fully understand all the processes related to the analysis that we are going to carry out.

Remember that the sample received from the SDR receiver is made up of the I / Q values, therefore the first step is to 
obtain the real signal.

Once we have the real signal, it is necessary to eliminate the continuous component (DC) that will greatly facilitate 
the subsequent analysis process. For this we will use a simple IIR filter.

To calculate the modulation depth we need to know the envelope of the signal as if it were not modulated by the pulses 
or sub-carrier, for this we will use a simple slow exponential average.

Finally we will obtain the standard deviation or variance of the signal that will help us to calculate the appropriate detection thresholds
based on the background noise.

![Signal processing](doc/img/signal-pre-process.png "Signal pre-processing")

An example of each component, x(t), w(t), v(t) and a(t).

![Signal processing](doc/img/signal-pre-process-example.png "Signal pre-processing")

### Next, identify the type of modulation needed

As we have seen in the description, the NFC-A / B / F / V standards will use different modulations but all are based on
two basic techniques, amplitude modulation and phase modulation.

For the encoding of each symbol they use Miller, Manchester or NRZ-L. The first two can be detected by correlation
techniques and for NRZ-L it is enough to detect the level of the signal at each point of synchronization, let's see it
in detail.

### Basic notions of signal correlation

The correlation operation is a measure of how much one signal resembles another that serves as a reference. It is used
intensively in digital signal analysis. With analog signals, the correlation of each sample x(t) requires N 
multiplications, therefore a symbol needs N^2 multiplications, being a costly process.

But since the reference signal is digital, it only has two possible values 0 or 1, which greatly simplifies the
calculation by eliminating all the multiplications, allowing the correlation to be carried out by process a simple
moving average.

![SYMBOLS S0 S1](doc/img/nfc-decoder-symbols.png  "Basic Symbols")

These would be the two basic symbols that we need to carry out the correlation, if you study a little the operations
that need to be carried out you will see that they are reduced to calculating the mean over the duration of the symbol
and then obtaining the difference between the critical points, t = 0, t = N / 2 and t = N as seen in next diagram.

![SYMBOLS Correlation](doc/img/nfc-symbol-correlation.png  "Correlation Process")

We will widely use this operation to extract the information within the NFC signals

### Demodulation of ASK miller and manchester signals

For ASK modulated signals, it is enough to carry out the correlation described above on the baseband signal x(t). Below
is the correlation functions for the two basic symbols S0, S1 used to calculate all the others. Last value is function
SD and represent the absolute difference between S0 and S1 necessary to detect the timmings.

![CORRELATION](doc/img/nfc-decoder-example.png "Decoder request correlation")

When the speed is 106 kbps, the answer can be extracted by applying the same technique, but instead of using the signal
x(t) we will do it with w(t) multiplying it by itself obtaining a measure of the power that we will then integrate
over 1/4 of the symbol period, in such a way that we will obtain a fairly clear ASK signal to be able to apply the
correlation described above, this is the diagram of the process.

![NFC ASK request](doc/img/nfc-demodulator-ask-request.png "Decoder response correlation")

Card is much weaker but enough to allow its detection using the same technique for patterns E, D, F,
here it is shown in better scale the process described. From top to bottom the signals are: x(t), w(t), w(t)^2 and y(t).

![NFC ASK response](doc/img/nfc-demodulator-response.png "Decoder response correlation")

### Demodulation of BPSK signals

For BPSK demodulation a reference signal is required to detect the phase changes (carrier recovery), since that is
complex I have chosen to implement it by multiplying each symbol by the preceding one, so that it is possible to
determine the value of symbols through the changes produced between then.

It is very important that the signal does not contain a DC shift, therefore the signal w(t) obtained previously
is taken as input to the process.

![BPSK1](doc/img/nfc-demodulator-bpsk-process.png "414Kbps BPSK demodulation process")

Below you can see the signal modulated in BPSK for a response frame at 424Kbps, followed by the demodulation y(t) and
integration process over a quarter of a symbol r(t).

![BPSK2](doc/img/nfc-demodulator-bpsk-detector.png "414Kbps BPSK response demodulation")

Finally, by checking if the result is positive or negative, the value of each symbol can be determined. It is somewhat
more complex since timing and synchronization must be considered but with this signal is straightforward detect symbol
values.

### Symbol detection

From the correlation process we obtain a flow of symbols where it is already possible to apply the specific decoding
defined in each of the standards, NFC-A / B / F / V. Each symbol correlation is evaluated in the appropriate instants
according to the synchronization.

The correlation process begins with the calculation of the S0 and S1 values that represent the basic symbols
subsequently used to discriminate between the NFC patterns X, Y, Z, E, D, F, M, N etc. that are subsequently interpreted 
by a state machine in accordance with the specifications of ISO ISO14443A, ISO14443B, ISO15693 and Felica to obtain a 
byte stream that can be easily processed.

![DEC2](doc/img/nfc-demodulator-pattern-process.png "Pattern and frame detection")

### Bitrate discrimination

So, we have seen how demodulation is performed, but how does this apply when there are different speeds? Well, since we
do not know in advance the transmission speed it is necessary to apply the same process for all possible speeds through
a bank of correlators. Really only is necessary to do it for the first symbol of each frame, once the bitrate is known
the rest are decoded using that speed.

![DEC3](doc/img/nfc-demodulator-speed-detector.png "Bitrate discrimination")

