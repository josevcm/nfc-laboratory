/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include "lime/LimeSuite.h"
#include <ConnectionRegistry.h>
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <vector>

int deviceCalSweep(
    const std::string &argStr,
    const double start,
    const double stop,
    const double step,
    const double bw,
    const std::string &dirStr,
    const std::string &chansStr)
{
    //check the frequency range
    if (start == 0.0 || stop == 0.0 || step == 0.0)
    {
        std::cerr << "Unspecified range --start, stop, step!" << std::endl;
        return EXIT_FAILURE;
    }

    //get a list of the directions to calibrate over
    std::vector<bool> dirs;
    if (dirStr == "RX") dirs.push_back(LMS_CH_RX);
    else if (dirStr == "TX") dirs.push_back(LMS_CH_TX);
    else if (dirStr == "BOTH")
    {
        dirs.push_back(LMS_CH_RX);
        dirs.push_back(LMS_CH_TX);
    }
    else
    {
        std::cerr << "Unknown directions --dir=" << dirStr << std::endl;
        return EXIT_FAILURE;
    }
    
    //open the device
    lms_device_t *device(nullptr);
  
    lime::ConnectionHandle hint(argStr);
    auto handles = lime::ConnectionRegistry::findConnections(hint);

    if(handles.size() == 0)
    {
        std::cerr << "No available device!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Connected to [" << handles[0].ToString() << "]" << std::endl;

    if (LMS_Open(&device, handles[0].serialize().c_str(), nullptr) != 0)
    {
        std::cerr << "Failed to open" << std::endl;
        return EXIT_FAILURE;
    }

    if (LMS_EnableCache(device, true) != 0)
    {
        std::cerr << "Failed to enable cal cache" << std::endl;
        return EXIT_FAILURE;
    }

    //get a list of the channels to calibrate over
    std::vector<std::pair<bool, size_t>> channelMatrix;
    for (const auto &dir_tx : dirs)
    {
        if (chansStr == "ALL")
        {
            for (int i = 0; i < LMS_GetNumChannels(device, dir_tx); i++)
            {
                channelMatrix.emplace_back(dir_tx, size_t(i));
            }
        }
        else
        {
            channelMatrix.emplace_back(dir_tx, std::stoi(chansStr));
        }
    }

    //enable all channels in the matrix and set to the first antenna
    for (const auto chanConfig : channelMatrix)
    {
        LMS_EnableChannel(device, chanConfig.first, chanConfig.second, true);
    }

    //summary
    std::cout << "Cal sweep over [" << start/1e6 << ", " << stop/1e6 << ", " << step/1e6 << "] MHz, channels=" << chansStr << ", dir=" << dirStr << std::endl;

    for (double freq = start; freq <= stop; freq += step)
    {
        std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
        std::cout << "@@  Calibrating for freq = " << freq/1e6 << " MHz" << std::endl;
        std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
        std::cout << std::endl;

        for (int path = 1; path < 3; path++)
        {
            //apply identical paths across channels
            //(BAND1/BAND2 for Tx) (LNAL, LNAW for Rx)
            for (int i = 0; i < LMS_GetNumChannels(device, LMS_CH_RX); i++)
                LMS_SetAntenna(device, LMS_CH_RX, i, path+1);
            for (int i = 0; i < LMS_GetNumChannels(device, LMS_CH_TX); i++)
                LMS_SetAntenna(device, LMS_CH_TX, i, path);

            //iterate through the matrix of channel options
            for (const auto chanConfig : channelMatrix)
            {
                if (LMS_SetLOFrequency(device, chanConfig.first, chanConfig.second, freq) != 0)
                {
                    std::cerr << "Error tuning (skipping)" << std::endl;
                    continue;
                }
                if (LMS_Calibrate(device, chanConfig.first, chanConfig.second, bw, 0) != 0)
                {
                    std::cerr << "Error calibrating (skipping)" << std::endl;
                    continue;
                }
            }
        }
        std::cout << std::endl;
    }

    std::cout << "Cleanup..." << std::endl;
    LMS_Close(device);
    return EXIT_SUCCESS;
}
