#include "lms7_device.h"
#include <iostream>
#include <iomanip>
#include <getopt.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>
#include "FPGA_common.h"
#include "Logger.h"

using namespace std;
using namespace lime;

auto t1 = chrono::high_resolution_clock::now();
auto t2 = chrono::high_resolution_clock::now();

bool ConfigureCGEN(LMS7_Device* device, double freqMHz);

int printHelp(void);

int log_level = 3;

void log_func(const lime::LogLevel level, const char *message)
{
    if (level <= log_level)
        std::cout << message << std::endl;
}

int main(int argc, char** argv)
{
    double beginFreq = 50e6;
    double endFreq = 500e6;
    double stepFreq = 1e6;
    string configFilename = "";
    bool init = false;
    int deviceIndex = -1;

    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            {"beginFreq",   required_argument, 0, 'b'},
            {"stepFreq",    required_argument, 0, 's'},
            {"endFreq",     required_argument, 0, 'e'},
            {"log",         required_argument, 0, 'l'},
            {"config",      required_argument, 0, 'c'},
            {"device",      required_argument, 0, 'd'},
            {"help",        no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long (argc, argv, "b:s:e:l:c::d:h", long_options, &option_index);

        if (c == -1) //no parameters given
            break;
        switch (c)
        {
        case 'b':{
            stringstream ss;
            ss << optarg;
            ss >> beginFreq;
            break;
        }
        case 's':{
            stringstream ss;
            ss << optarg;
            ss >> stepFreq;
            break;
        }
        case 'e':{
            stringstream ss;
            ss << optarg;
            ss >> endFreq;
            break;
        }
        case 'l':{
            stringstream ss;
            ss << optarg;
            ss >> log_level;
            break;
        }
        case 'c':{
            if (optarg != NULL){
                stringstream ss;
                ss << optarg;
                ss >> configFilename;
            }
            else init = true;
            break;
        }
        case 'd':{
            stringstream ss;
            ss << optarg;
            ss >> deviceIndex;
            break;
        }
        case 'h':
            printHelp();
            return 0;
        case '?':
            /* getopt_long already printed an error message. */
            break;
        default:
            std::cout << "Invalid option" << std::endl;
            abort();
        }
    }

    cout << "beginFreq = " << beginFreq/1e6 << " MHz" << endl;
    cout << "stepFreq  = " << stepFreq/1e6 << " MHz" << endl;
    cout << "endFreq   = " << endFreq/1e6 << " MHz" << endl;

    LMS7_Device* device;
    std::vector<lime::ConnectionHandle> handles = LMS7_Device::GetDeviceList();
    if(handles.size() == 0)
    {
        cout << "No devices found" << endl;
        return -1;
    }
    if(handles.size() == 1) //open the only available device
        device = lime::LMS7_Device::CreateDevice(handles[0],nullptr);
    else //display device selection
    {
        if(deviceIndex < 0)
        {
            cout << "Device list:" << endl;
            for (size_t i = 0; i < handles.size(); i++)
               cout << setw(2) << i << ". " << handles[i].name << endl;
            cout << "Select device index (0-" << handles.size()-1 << "): ";
            int selection = 0; cin >> selection;
            selection = selection % handles.size();
            device = lime::LMS7_Device::CreateDevice(handles[selection],nullptr);
        }
        else
            device = lime::LMS7_Device::CreateDevice(handles[deviceIndex],nullptr);
    }
    if(device == nullptr)
    {
        cout << "Failed to connected to device" << endl;
        return -1;
    }
    auto info = device->GetInfo();
    cout << "\nConnected to: " << info->deviceName
    << " FW: " << info->firmwareVersion << " HW: " << info->hardwareVersion << endl;

    if(configFilename.length() > 0)
    {
        if(device->LoadConfig(configFilename.c_str()) != 0)
        {
            return -1;
        }
    }
    else if (init)
    {
        device->Init();
    }

    lime::registerLogHandler(log_func);
    int errors = 0;
    for (double freq = beginFreq; freq < endFreq; freq += stepFreq)
        if (ConfigureCGEN(device, freq)==false)
            errors++;
    cout << "Errors: " << errors << endl;
    delete device;
    return 0;
}

bool ConfigureCGEN(LMS7_Device* device, double freqMHz)
{
    std::cout << "Set CGEN " << freqMHz/1e6 << " MHz: ";
    LMS7002M* lms = device->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (lms->SetInterfaceFrequency(freqMHz, interp, decim)!=0)
    {
        std::cout << "LMS VCO Fail" << endl;
        return false;
    }

    auto chipInd = lms->GetActiveChannelIndex()/2;
    auto fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    if (interp != 7)
        fpgaTxPLL /= pow(2.0, interp);
    auto fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
    if (decim != 7)
        fpgaRxPLL /= pow(2.0, decim);
    auto fpga = device->GetFPGA();
    if (fpga)
    {
        int status = fpga->SetInterfaceFreq(fpgaTxPLL,fpgaRxPLL,chipInd);
        if (status != 0)
        {
            std::cout << "FPGA Fail" << endl;
            return false;
        }
    }
    std::cout << "OK" << endl;
    return true;
}

/***********************************************************************
 * print help
 **********************************************************************/
int printHelp(void)
{
    std::cout << "Usage pll_sweep [options]" << std::endl;
    std::cout << "available options:" << std::endl;
    std::cout << "    --help \t\t Print this help message" << std::endl;
    std::cout << "    --beginFreq \t sweep start frequency" << std::endl;
    std::cout << "    --endFreq \t\t sweep end frequency" << std::endl;
    std::cout << "    --stepFreq \t\t sweep frequency step" << std::endl;
    std::cout << "    --log \t\t log level" << std::endl;
    std::cout << "    --config \t\t load config" << std::endl;
    std::cout << "    --device \t\t device index" << std::endl;
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

