# SDR nfc-laboratory v2.0

NFC signal sniffer and protocol decoder using SDR receiver, capable of demodulate in real-time the comunication
with contacless cards up to 424Kpbs.

## Description

By using an SDR receiver it is possible to capture, demodulate and decode the NFC signal between the card and the
reader.

I do not have as an objective to explain the NFC norms or modulation techniques, there is a multitude of documentation
accessible through Google, I will describe as simply as possible the method that i have used to implement this software.

Currently, detection and decoding for NFC-A (ISO14443A), NFC-B (ISO14443B) and NFC-V (ISO15693) modulation has been implemented.

## Signal processing

The first step is receive the 13.56MHz signal and demodulate to get the baseband ASK stream, for this purpose any SDR
device capable of tuning this frequency can be used, i have the fantastic and cheap AirSpy Mini capable of tuning from
24Mhz to 1700Mhz. (https://airspy.com/airspy-mini/)

However, it is not possible to tune 13.56Mhz with this receiver, instead I use the second harmonic at 27.12Mhz or third
at 40.68Mhz with good results.

Let's see a capture of the signal received in baseband (after I/Q to magnitude transform) for the REQA command and its
response:

![REQA](doc/nfc-baseband-reqa.png?raw=true "REQA signal capture")

As can be seen, it is a signal modulated in 100% ASK that corresponds to the REQA 26h command of the NFC specifications,
the response of the card uses something called load modulation that manifests as a series of pulses on the main signal
after the command.

### Demodulation

Due to the digital nature of the signal I used a technique called symbol correlation which is equivalent to carrying out
the convolution of the signal with the shape of each symbol to be detected. Without going into details, the NFC-A
modulation is based on 6 patterns: Y, X and Z for reader commands and E, D, F for card responses (see NFC specifications
for complete description).

Demodulation is performed by calculating the correlation for these patterns and detecting when the maximum approximation
to each of them occurs. Below is the correlation functions for the two basic symbols S0, S1 used to calculate all the
others. Last value is function SD represent the absolute difference between S0 and S1 necessary to detect the timmings.

![CORRELATION](doc/nfc-decoder-log.png?raw=true "Decoder symbol correlation")

The response of the card is much weaker but enough to allow its detection using the same technique for patterns E, D, F,
here it is shown in better scale.

![CORRELATION](doc/nfc-response-log.png?raw=true "Decoder response correlation")

### Symbol detection

For the detection of each symbol the value of each correlation is evaluated in the appropriate instants according to the
synchronization. The number of samples per symbol is defined as N and must be calculated before starting the process
knowing the sampling frequency (fc) and symbol duration.

The correlation process begins with the calculation of the S0 and S1 values that represent the basic symbols
subsequently used to discriminate between the NFC patterns X, Y, Z, E, D and F, as shown below.

![DEC1](doc/nfc-demodulator-correlation.png?raw=true "Symbols S0 and S1 correlation")

This results in a flow of patterns X, Y, Z, E, D, F that are subsequently interpreted by a state machine in accordance
with the specifications of ISO 14443-3 to obtain a byte stream that can be easily processed.

![DEC2](doc/nfc-demodulator-pattern-process.png?raw=true "Pattern and frame detection")

### Bitrate discrimination

So, we have seen how demodulation is performed, but how does this apply when there are different speeds? Well, since we
do not know in advance the transmission speed it is necessary to apply the same process for all possible speeds through
a bank of correlators. Really only is necessary to do it for the first symbol of each frame, once the bitrate is known
the rest are decoded using that speed.

![DEC3](doc/nfc-demodulator-speed-detector.png?raw=true "Bitrate discrimination")

### BPSK modulation

ASK modulation is relatively simple and easy to implement, however the specification ISO 14443 defines the use of BPSK
for card responses when the speed is 212Kbps or higher.

For BPSK demodulation a reference signal is required to detect the phase changes (carrier recovery), since that is
complex i have chosen to implement it by multiplying each symbol by the preceding one, so that it is possible to
determine the value of symbols through the changes produced between then.

![BPSK1](doc/nfc-demodulator-bpsk-process.png?raw=true "414Kbps BPSK demodulation process")

Below you can see the signal modulated in BPSK for a response frame at 424Kbps, followed by the demodulation y(t) and
integration process over a quarter of a symbol r(t).

![BPSK2](doc/nfc-demodulator-bpsk-detector.png?raw=true "414Kbps BPSK response demodulation")

Finally, by checking if the result is positive or negative, the value of each symbol can be determined. It is somewhat
more complex since timing and synchronization must be considered.

### Signal quality analysis

This version includes a spectrum analyzer to show the power and quality of the received signal.

## Application example

An example of the result can be seen below.

Signal spectrum view.

![APP](doc/nfc-lab-capture1.png?raw=true "Application example")

Signal wave view.

![APP](doc/nfc-lab-capture2.png?raw=true "Application example")

Signal frame and protocol time measurement.

![APP](doc/nfc-lab-capture3.png?raw=true "Protocol timing example")

Protocol detail view.

![APP](doc/nfc-lab-capture4.png?raw=true "Protocol detail example")

Inside the "doc" folder you can find a [video](doc/VID-20210912-WA0004.mp4?raw=true) with an example of how it works.

## SDR Receivers tested

I have tried several receivers obtaining the best results with AirSpy Mini, I do not have more devices, but surely it
works with others.

- AirSpy Mini or R2: Better results, tuning the third harmonic 40.68Mhz, with a sampling frequency of 10 Mbps, with these
  parameters it is possible to capture the communication up to 424 Kbps.

- RTL SDR: It works by tuning the second harmonic 27.12Mhz, due to the limitation in the maximum sampling frequency 
  of 3Mbps and its 8 bits of precision only allows you to capture the commands.

![Devices](doc/nfc-lab-devices1.png?raw=true "Devices")

![Devices](doc/nfc-lab-devices2.png?raw=true "Devices")

![Devices](doc/nfc-lab-devices3.png?raw=true "Devices")

### Upconverters

To avoid the use of harmonics it is possible to use an up-converter and thus tune directly to the carrier 
frequency of 13.56Mhz, although I have not tried this combination.

## Hardware requirements and performance

The demodulator is designed to run in real time, so it requires a recent computer with a lot of processing capacity.

During development, I have opted for a mixed approach where some optimizations are sacrificed in favor of maintaining clarity in the code and facilitating its monitoring and debugging.

For this reason it is possible that certain parts can be improved in performance, but I have done it as a didactic exercise rather than a production application.

## Build instructions

This project has two main components and is based on Qt5 and MinGW-W64:

- /src/nfc-app: Application interface based on Qt Widgets
- /src/nfc-lib: A core library without dependencies of Qt (for other uses)

And can be build with mingw-g64

### Prerequisites:

- Qt5 framework 5.x for Windows, see https://www.qt.io/offline-installers
- A recent mingw-w64 for windows, see https://www.mingw-w64.org/downloads
- CMake version 3.17 or higher, see http://www.cmake.org/cmake/resources/software.html
- Git-bash or your preferred client for Windows, see https://gitforwindows.org/ 

### Manual build without IDE

Using git-bash, download repository:

```
$ git clone https://github.com/josevcm/nfc-laboratory.git
Cloning into 'nfc-laboratory'...
remote: Enumerating objects: 1629, done.
remote: Counting objects: 100% (1003/1003), done.
remote: Compressing objects: 100% (776/776), done.
remote: Total 1629 (delta 188), reused 990 (delta 179), pack-reused 626
Receiving objects: 100% (1629/1629), 32.09 MiB | 10.60 MiB/s, done.
Resolving deltas: 100% (312/312), done.
Updating files: 100% (975/975), done.
```

Prepare release makefiles, or change `CMAKE_BUILD_TYPE=Debug` and `-B cmake-build-debug` for debug output:

```
$ cmake.exe -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - MinGW Makefiles" -S nfc-laboratory -B cmake-build-release
-- The C compiler identification is GNU 8.1.0
-- The CXX compiler identification is GNU 8.1.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: D:/develop/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/gcc.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: D:/develop/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/g++.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- USB_LIBRARY: C:/Users/jvcampos/build/nfc-laboratory/dll/usb-1.0.20/x86_64-w64-mingw32/lib/libusb-1.0.dll.a
-- GLEW_LIBRARY: C:/Users/jvcampos/build/nfc-laboratory/dll/glew-2.1.0/x86_64-w64-mingw32/lib/libglew32.dll.a
-- FT_LIBRARY: C:/Users/jvcampos/build/nfc-laboratory/dll/freetype-2.11.0/x86_64-w64-mingw32/lib/libfreetype.dll.a
-- Configuring done
-- Generating done
-- Build files have been written to: C:/Users/jvcampos/build/cmake-build-release
```

Launch build:

```
$ cmake.exe --build cmake-build-release --target nfc-lab -- -j 6
Scanning dependencies of target mufft-sse
Scanning dependencies of target mufft-sse3
Scanning dependencies of target rt-lang
Scanning dependencies of target mufft-avx
Scanning dependencies of target airspy
Scanning dependencies of target rtlsdr
[  1%] Building C object src/nfc-lib/lib-ext/mufft/CMakeFiles/mufft-sse.dir/src/main/cpp/x86/kernel.sse.c.obj
[  2%] Building C object src/nfc-lib/lib-ext/mufft/CMakeFiles/mufft-sse3.dir/src/main/cpp/x86/kernel.sse3.c.obj
[  3%] Building C object src/nfc-lib/lib-ext/mufft/CMakeFiles/mufft-avx.dir/src/main/cpp/x86/kernel.avx.c.obj
[  4%] Building CXX object src/nfc-lib/lib-rt/rt-lang/CMakeFiles/rt-lang.dir/src/main/cpp/Executor.cpp.obj
[  4%] Building C object src/nfc-lib/lib-ext/airspy/CMakeFiles/airspy.dir/src/main/cpp/airspy.c.obj
[  5%] Building C object src/nfc-lib/lib-ext/rtlsdr/CMakeFiles/rtlsdr.dir/src/main/cpp/librtlsdr.c.obj
....
[100%] Linking CXX executable nfc-lab.exe
[100%] Built target nfc-lab
```

### Prepare Qt deployment

To run the application correctly it is necessary to deploy the Qt components together with the libraries, fonts and the generated artifact.

```
mkdir qt-deploy
cp -rf nfc-laboratory/dat/conf/ qt-deploy/
cp -rf nfc-laboratory/dat/fonts/ qt-deploy/
cp nfc-laboratory/dll/glew-2.1.0/x86_64-w64-mingw32/bin/*.dll qt-deploy/
cp nfc-laboratory/dll/usb-1.0.20/x86_64-w64-mingw32/bin/*.dll qt-deploy/
cp nfc-laboratory/dll/freetype-2.11.0/x86_64-w64-mingw32/bin/*.dll qt-deploy/
cp cmake-build-release/src/nfc-app/app-qt/nfc-lab.exe qt-deploy/
```

Run `windeployqt.exe` tool to create required folders and copy required Qt DLLs

```
$ windeployqt.exe --release --compiler-runtime --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw qt-deploy/nfc-lab.exe
C:\Users\jvcampos\build\qt-deploy\nfc-lab.exe 64 bit, release executable
Adding Qt5Svg for qsvgicon.dll
Direct dependencies: Qt5Core Qt5Gui Qt5PrintSupport Qt5Widgets
All dependencies   : Qt5Core Qt5Gui Qt5PrintSupport Qt5Widgets
To be deployed     : Qt5Core Qt5Gui Qt5PrintSupport Qt5Svg Qt5Widgets
Updating Qt5Core.dll.
Updating Qt5Gui.dll.
Updating Qt5PrintSupport.dll.
Updating Qt5Svg.dll.
Updating Qt5Widgets.dll.
Updating libgcc_s_seh-1.dll.
Updating libstdc++-6.dll.
Updating libwinpthread-1.dll.
Patching Qt5Core.dll...
Creating directory C:/Users/jvcampos/build/qt-deploy/iconengines.
Updating qsvgicon.dll.
Creating directory C:/Users/jvcampos/build/qt-deploy/imageformats.
Updating qgif.dll.
Updating qicns.dll.
Updating qico.dll.
Updating qjpeg.dll.
Updating qsvg.dll.
Updating qtga.dll.
Updating qtiff.dll.
Updating qwbmp.dll.
Updating qwebp.dll.
Creating directory C:/Users/jvcampos/build/qt-deploy/platforms.
Updating qwindows.dll.
Creating directory C:/Users/jvcampos/build/qt-deploy/printsupport.
Updating windowsprintersupport.dll.
Creating directory C:/Users/jvcampos/build/qt-deploy/styles.
Updating qwindowsvistastyle.dll.
```

Application is ready to use!

If you do not have an SDR receiver, I have included a small capture sample signal in file "wav/capture-424kbps.wav" that
serves as an example to test demodulation.

### Build from QtCreator

Thanks to bvernoux for this instructions:

Working solution is to use Qt Creator (Tested with latest Qt Creator 5.0.1 with Qt 5.15.2 + mingw81_64 ) then import the nfc-laboratory/CMakeLists.txt project and built it with MinGW 64-bit (Tested with success with Qt 5.15.2 + mingw81_64 on Windows10Pro 21H1)
Example of batch used(requires msys2/linux cp/rm commands) to do the deployment (after build of the release version with Qt Creator)

```
set qtpath=C:\Qt\5.15.2\mingw81_64\bin\
set PATH=%qtpath%;%PATH%
set execpath="%qtpath%\windeployqt.exe"
set nflab_git_path=D:\_proj\__Lab_Tools\NFC\nfc-laboratory
set build_path=%nflab_git_path%\..\build-nfc-laboratory-Desktop_Qt_5_15_2_MinGW_64_bit-Release\src\nfc-app\app-qt

cp %build_path%/nfc-lab.exe ./
%execpath% --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw nfc-lab.exe

cp %qtpath%\libgcc_s_seh-1.dll ./
cp %qtpath%\libwinpthread-1.dll ./
cp %qtpath%\libstdc++-6.dll ./

cp -a %nflab_git_path%\dat\conf ./
cp -a %nflab_git_path%\dat\fonts ./

cp %nflab_git_path%\dll\glew-2.1.0\x86_64-w64-mingw32\bin\libglew32.dll ./
cp %nflab_git_path%\dll\freetype-2.11.0\x86_64-w64-mingw32\bin\libfreetype.dll ./
cp %nflab_git_path%\dll\usb-1.0.20\x86_64-w64-mingw32\bin\libusb-1.0.dll ./
```
### Build from Jetbrains CLion

This is my favorite IDE and that I use for all projects. 

Install all prerequisites, and register MinGW toolchain in `File->Settings->Build, Execution, Deployment->Toolchains`

![Build Settings](doc/clion-toolchain-settings.png)

Next download project from GITHUB and create new CMake project from source, then build release and debug verions.

Prepare the Qt deployment environment by executing the script `build-runenv.ps1`, now is ready to launch!

## Source code and licensing

If you think it is an interesting job or you plan to use it for something please send me an email and let me know, I
will be happy to exchange experiences, thank you very much.

This project is published under the terms of the MIT license, however there are parts of it subject to other types of
licenses, please check if you are interested in this work.

- AirSpy SDR driver at `src/nfc-lib/lib-ext/airspy` see https://github.com/airspy/airspyone_host
- RTL SDR driver at `src/nfc-lib/lib-ext/rtlsdr` see https://osmocom.org/projects/rtl-sdr
- nlohmann json at `src/nfc-lib/lib-ext/nlohmann` see https://github.com/nlohmann/json
- mufft library at `src/nfc-lib/lib-ext/mufft` see https://github.com/Themaister/muFFT
- QCustomPlot at `src/nfc-app/app-qt/src/main/cpp/support` see https://www.qcustomplot.com/
- QDarkStyleSheet at `src/nfc-app/app-qt/src/main/assets/theme` see https://github.com/ColinDuquesnoy/QDarkStyleSheet

## Next steps, work in Android?

I have been able to migrate this SW to Android (very simplified) by connecting an SDR
receiver and sniff NFC frames in real-time, interesting thing to investigate, maybe start a new project with this...

## Releases

Precompiled installer for x86 64 bit can be found in repository