/**
    @file   singleRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  RX example
 */
#include "lime/LimeSuite.h"
#include <iostream>
#include <chrono>
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace std;

//Device structure, should be initialize to NULL
lms_device_t* device = NULL;

int error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

int main(int argc, char** argv)
{
    //Find devices
    //First we find number of devices, then allocate large enough list,  and then populate the list
    int n;
    if ((n = LMS_GetDeviceList(NULL)) < 0)//Pass NULL to only obtain number of devices
        error();
    cout << "Devices found: " << n << endl;
    if (n < 1)
        return -1;

    lms_info_str_t* list = new lms_info_str_t[n];   //allocate device list

    if (LMS_GetDeviceList(list) < 0)                //Populate device list
        error();

    for (int i = 0; i < n; i++)                     //print device list
        cout << i << ": " << list[i] << endl;
    cout << endl;

    //Open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    delete [] list;                                 //free device list

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
        error();

    //Set center frequency to 800 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 800e6) != 0)
        error();

    //print currently set center frequency
    float_type freq;
    if (LMS_GetLOFrequency(device, LMS_CH_RX, 0, &freq) != 0)
        error();
    cout << "\nCenter frequency: " << freq / 1e6 << " MHz\n";

    //select antenna port
    lms_name_t antenna_list[10];    //large enough list for antenna names.
                                    //Alternatively, NULL can be passed to LMS_GetAntennaList() to obtain number of antennae
    if ((n = LMS_GetAntennaList(device, LMS_CH_RX, 0, antenna_list)) < 0)
        error();

    cout << "Available antennae:\n";            //print available antennae names
    for (int i = 0; i < n; i++)
        cout << i << ": " << antenna_list[i] << endl;

    if ((n = LMS_GetAntenna(device, LMS_CH_RX, 0)) < 0) //get currently selected antenna index
        error();
    //print antenna index and name
    cout << "Automatically selected antenna: " << n << ": " << antenna_list[n] << endl;

    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAW) != 0) // manually select antenna
        error();

    if ((n = LMS_GetAntenna(device, LMS_CH_RX, 0)) < 0) //get currently selected antenna index
        error();
   //print antenna index and name
    cout << "Manually selected antenna: " << n << ": " << antenna_list[n] << endl;

    //Set sample rate to 8 MHz, preferred oversampling in RF 8x
    //This set sampling rate for all channels
    if (LMS_SetSampleRate(device, 8e6, 8) != 0)
        error();
    //print resulting sampling rates (interface to host , and ADC)
    float_type rate, rf_rate;
    if (LMS_GetSampleRate(device, LMS_CH_RX, 0, &rate, &rf_rate) != 0)  //NULL can be passed
        error();
    cout << "\nHost interface sample rate: " << rate / 1e6 << " MHz\nRF ADC sample rate: " << rf_rate / 1e6 << "MHz\n\n";

    //Example of getting allowed parameter value range
    //There are also functions to get other parameter ranges (check LimeSuite.h)

    //Get allowed LPF bandwidth range
    lms_range_t range;
    if (LMS_GetLPFBWRange(device,LMS_CH_RX,&range)!=0)
        error();

    cout << "RX LPF bandwitdh range: " << range.min / 1e6 << " - " << range.max / 1e6 << " MHz\n\n";

    //Configure LPF, bandwidth 8 MHz
    if (LMS_SetLPFBW(device, LMS_CH_RX, 0, 8e6) != 0)
        error();

    //Set RX gain
    if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0.7) != 0)
        error();
    //Print RX gain
    float_type gain; //normalized gain
    if (LMS_GetNormalizedGain(device, LMS_CH_RX, 0, &gain) != 0)
        error();
    cout << "Normalized RX Gain: " << gain << endl;

    unsigned int gaindB; //gain in dB
    if (LMS_GetGaindB(device, LMS_CH_RX, 0, &gaindB) != 0)
        error();
    cout << "RX Gain: " << gaindB << " dB" << endl;

    //Perform automatic calibration
    if (LMS_Calibrate(device, LMS_CH_RX, 0, 8e6, 0) != 0)
        error();

    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
    if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV8, 0, 0) != 0)
        error();

    //Streaming Setup

    //Initialize stream
    lms_stream_t streamId;
    streamId.channel = 0; //channel number
    streamId.fifoSize = 1024 * 1024; //fifo size in samples
    streamId.throughputVsLatency = 1.0; //optimize for max throughput
    streamId.isTx = false; //RX channel
    streamId.dataFmt = lms_stream_t::LMS_FMT_F32; //32-bit floats
    if (LMS_SetupStream(device, &streamId) != 0)
        error();

    //Data buffers
    const int bufersize = 10000; //complex samples per buffer
    float buffer[bufersize * 2]; //must hold I+Q values of each sample
    //Start streaming
    LMS_StartStream(&streamId);

#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-1:1]\n set yrange[-1:1]\n");
#endif
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(10)) //run for 10 seconds
    {
        int samplesRead;
        //Receive samples
        samplesRead = LMS_RecvStream(&streamId, buffer, bufersize, NULL, 1000);
        //I and Q samples are interleaved in buffer: IQIQIQ...
	/*
		INSERT CODE FOR PROCESSING RECEIVED SAMPLES
	*/
        //Plot samples
#ifdef USE_GNU_PLOT
        gp.write("plot '-' with points\n");
        for (int j = 0; j < samplesRead; ++j)
            gp.writef("%f %f\n", buffer[2 * j], buffer[2 * j + 1]);
        gp.write("e\n");
        gp.flush();
#endif
        //Print stats (once per second)
        if (chrono::high_resolution_clock::now() - t2 > chrono::seconds(1))
        {
            t2 = chrono::high_resolution_clock::now();
            lms_stream_status_t status;
            //Get stream status
            LMS_GetStreamStatus(&streamId, &status);
            cout << "RX data rate: " << status.linkRate / 1e6 << " MB/s\n"; //link data rate
            cout << "RX fifo: " << 100 * status.fifoFilledCount / status.fifoSize << "%" << endl; //percentage of FIFO filled
        }
    }

    //Stop streaming
    LMS_StopStream(&streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &streamId); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);

    return 0;
}