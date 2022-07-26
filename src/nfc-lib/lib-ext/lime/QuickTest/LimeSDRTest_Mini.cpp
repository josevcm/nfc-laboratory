#include <chrono>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "LimeSDRTest.h"
#include <thread>
#include "kiss_fft.h"
#include "FPGA_Mini.h"
#include <ctime>

using namespace lime;

int LimeSDRTest_Mini::ClockNetworkTest()
{
    int ret = 0;
    UpdateStatus(LMS_TEST_INFO, "->REF clock test");
    if (GPIFClkTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "->REF clock test FAILED");
        ret = -1;
    }

    UpdateStatus(LMS_TEST_INFO, "->VCTCXO test");
    if (VCTCXOTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "  FAILED");
        ret = -1;
    }
    return ret;
}

int LimeSDRTest_Mini::RFTest()
{
    auto testPath = [this](float freq, int rxPath)->bool{
        const float tx_offset = 5e6;
        bool passed = true;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Tx, freq+tx_offset)!=0)
            return false;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Rx, freq)!=0)
            return false;
        RFTestData testinfo = {freq, freq+tx_offset, -14, tx_offset, 0};
        device->SetPath(false, 0, rxPath);
        device->SetPath(true, 0, rxPath==1? 1 : 2);
        if (device->GetConnection()->WriteRegister(0x17, rxPath==1? 0x2200:0x1100) != 0)
            return false;
        bool testPassed = RunTest(testinfo.peakval,testinfo.peakfreq, 0);
        std::string str = RFTestInfo(testinfo, testPassed);
        UpdateStatus(LMS_TEST_INFO, str.c_str());
        if (testPassed == false)
            passed = false;
        return passed;
    };

    UpdateStatus(LMS_TEST_INFO, "->Configure LMS");
    device->Init();
    if (device->SetRate(30.72e6, 0)!=0)
    {
        UpdateStatus(LMS_TEST_FAIL, "Failed to set sample rate");
        return -1;
    }
    device->SetGain(false, 0, 26);
    device->EnableChannel(true, 0, true);
    device->SetTestSignal(true,0,LMS_TESTSIG_DC, 0x7000, 0x7000);

    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_2 -> LNA_W):");
    bool passed = testPath(1000e6, 3);
    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_1 -> LNA_H):");
    passed &= testPath(2100e6, 1);

    return passed ? 0 : -1;
}