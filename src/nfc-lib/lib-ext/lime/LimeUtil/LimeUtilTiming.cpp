/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include <VersionInfo.h>
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS7002M.h>
#include <iostream>
#include <chrono>

using namespace lime;

struct LMS7002M_quiet : LMS7002M
{
    void Log(const char* text, LogType type)
    {
        if (type == LOG_INFO) return;
        LMS7002M::Log(text, type);
    }
};

int deviceTestTiming(const std::string &argStr)
{
    auto handles = ConnectionRegistry::findConnections(argStr);
    if(handles.size() == 0)
    {
        std::cout << "No devices found" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected to [" << handles[0].serialize() << "]" << std::endl;
    auto conn = ConnectionRegistry::makeConnection(handles[0]);

    std::cout << "Creating instance of LMS7002M:" << std::endl;
    auto lms7 = new LMS7002M_quiet;
    lms7->SetConnection(conn);

    std::cout << std::endl;
    std::cout << "Timing basic operations:" << std::endl;

    //time spi write access
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SPI_write(0x0000, 0x0000);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> SPI write register:\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time spi read access
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SPI_read(0x0000, true);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> SPI read register:\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time NCO setting
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetNCOFrequency(LMS7002M::Tx, 0, i*(1e6/numIters));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> TSP NCO setting:\t\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time LNA setting
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetRFELNA_dB(0);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> RFE gain setting:\t\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time PAD setting
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetTRFPAD_dB(0);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> TRF gain setting:\t\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Timing tuning operations:" << std::endl;

    //time CGEN tuning
    {
        const size_t numIters(100);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetFrequencyCGEN(1e6*(100+i), false);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> CGEN PLL tuning:\t\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time LO tuning
    {
        const size_t numIters(100);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetFrequencySX(LMS7002M::Tx, 1e6*(100+i));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> RF PLL tuning:\t\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time TX filter
    {
        const size_t numIters(20);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->TuneTxFilter(10e6 + i*(3e6/numIters));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> TBB filter tuning:\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time RX filter
    {
        const size_t numIters(20);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->TuneRxFilter(10e6 + i*(3e6/numIters));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> RBB filter tuning:\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time TX corrections
    {
        const size_t numIters(20);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->CalibrateTx(10e6 + i*(3e6/numIters));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> TX corrections:\t\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time RX corrections
    {
        const size_t numIters(20);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->CalibrateRx(10e6 + i*(3e6/numIters));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << "  >>> RX corrections:\t\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Done timing!" << std::endl;
    delete lms7;
    ConnectionRegistry::freeConnection(conn);
    return EXIT_SUCCESS;
}
