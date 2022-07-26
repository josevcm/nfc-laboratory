/**
    @file   basicTX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal TX example
 */
#include <iostream>
#include <chrono>
#include <math.h>
#include "lime/LimeSuite.h"

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
    const double frequency = 500e6;  //center frequency to 500 MHz
    const double sample_rate = 5e6;    //sample rate to 5 MHz
    const double tone_freq = 1e6; //tone frequency
    const double f_ratio = tone_freq/sample_rate;
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device)!=0)
        error();

    //Enable TX channel,Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, true)!=0)
        error();

    //Set sample rate
    if (LMS_SetSampleRate(device, sample_rate, 0)!=0)
        error();
    cout << "Sample rate: " << sample_rate/1e6 << " MHz" << endl;

    //Set center frequency
    if (LMS_SetLOFrequency(device,LMS_CH_TX, 0, frequency)!=0)
        error();
    cout << "Center frequency: " << frequency/1e6 << " MHz" << endl;

    //select TX1_1 antenna
    if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX1)!=0)
        error();

    //set TX gain
    if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.7) != 0)
        error();

    //calibrate Tx, continue on failure
    LMS_Calibrate(device, LMS_CH_TX, 0, sample_rate, 0);
    
    //Streaming Setup
    
    lms_stream_t tx_stream;                 //stream structure
    tx_stream.channel = 0;                  //channel number
    tx_stream.fifoSize = 256*1024;          //fifo size in samples
    tx_stream.throughputVsLatency = 0.5;    //0 min latency, 1 max throughput
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //floating point samples
    tx_stream.isTx = true;                  //TX channel
    LMS_SetupStream(device, &tx_stream);

    //Initialize data buffers
    const int buffer_size = 1024*8;
    float tx_buffer[2*buffer_size];     //buffer to hold complex values (2*samples))
    for (int i = 0; i <buffer_size; i++) {      //generate TX tone
        const double pi = acos(-1);
        double w = 2*pi*i*f_ratio;
        tx_buffer[2*i] = cos(w);
        tx_buffer[2*i+1] = sin(w);
    }   
    cout << "Tx tone frequency: " << tone_freq/1e6 << " MHz" << endl;

    const int send_cnt = int(buffer_size*f_ratio) / f_ratio; 
    cout << "sample count per send call: " << send_cnt << std::endl;

    LMS_StartStream(&tx_stream);         //Start streaming
    //Streaming
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(10)) //run for 10 seconds
    {
        //Transmit samples
        int ret = LMS_SendStream(&tx_stream, tx_buffer, send_cnt, nullptr, 1000);
        if (ret != send_cnt)
            cout << "error: samples sent: " << ret << "/" << send_cnt << endl;
        //Print data rate (once per second)
        if (chrono::high_resolution_clock::now() - t2 > chrono::seconds(1))
        {
            t2 = chrono::high_resolution_clock::now();
            lms_stream_status_t status;
            LMS_GetStreamStatus(&tx_stream, &status);  //Get stream status
            cout << "TX data rate: " << status.linkRate / 1e6 << " MB/s\n"; //link data rate
        }
    }
    //Stop streaming
    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device, &tx_stream);

    //Disable TX channel
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, false)!=0)
        error();

    //Close device
    if (LMS_Close(device)==0)
        cout << "Closed" << endl;
    return 0;
}
