#include <chrono>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "LimeSDRTest.h"
#include <thread>
#include "kiss_fft.h"
#include "FPGA_Mini.h"
#include <ctime>
#include "Si5351C.h"
#include "ADF4002.h"

using namespace lime;

int LimeSDRTest_USB::ClockNetworkTest()
{
    int ret = 0;
    UpdateStatus(LMS_TEST_INFO, "->FX3 GPIF clock test");
    if (GPIFClkTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "->FX3 GPIF clock test FAILED");
        ret = -1;
    }

    UpdateStatus(LMS_TEST_INFO, "->Si5351C test");
    if (Si5351CTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "  FAILED");
        ret = -1;
    }

    UpdateStatus(LMS_TEST_INFO, "->ADF4002 Test");
    if (ADF4002Test() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "  FAILED");
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

int LimeSDRTest_USB::Si5351CTest()
{
    auto port = device->GetConnection();
    std::shared_ptr<Si5351C> si5351module(new Si5351C());
    si5351module->Initialize(port);
    si5351module->SetPLL(0, 25000000, 0);
    si5351module->SetPLL(1, 25000000, 0);
    for (int i = 0; i < 8; ++i)
        si5351module->SetClock(i, 27000000, true, false);
    Si5351C::Status status = si5351module->ConfigureClocks();
    if (status != Si5351C::SUCCESS || si5351module->UploadConfiguration() != Si5351C::SUCCESS)
    {
        UpdateStatus(LMS_TEST_INFO, "Failed to configure Si5351C");
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); //some settle time

    if ((InitFPGATest(0x02, 1.0) & 0x2) != 0x2)
        return -1;

    uint32_t addr[] = { 0x6A, 0x6B, 0x6C, 0x6D, 0x6F, 0x70, 0x71 };
    uint32_t vals[7];
    if (port->ReadRegisters(addr, vals, 7) != 0)
        return -1;

    std::string str = "";
    int result = 0;
    for (int i = 0; i < 7; i++)
    {
        str += "  CLK" + std::to_string(i) + ": " + std::to_string(vals[0]) + " / 17554";;
        int check = (int)vals[i] - vals[0];
        if (check > 1 || check < -1)
        {
            result = -1;
            str += " - FAILED\n";
            continue;
        }
        check = (int)vals[i] - 17554;
        if (check > 5 || check < -5)
        {
            str += " - FAILED\n";
            result = -1;
            continue;
        }
        str += " - PASSED\n";
    }
    UpdateStatus(LMS_TEST_INFO, str.substr(0,str.length()-1).c_str());
    return result;
}


int LimeSDRTest_USB::ADF4002Test()
{
    uint32_t data_l[] = { 0x1F8093, 0x1F80F2, 0x0001F4, 0x018001 };
    uint32_t data_h[] = { 0x1F8093, 0x1F80B2, 0x0001F4, 0x018001 };
    auto port = device->GetConnection();
    // ADF4002 needs to be writen 4 values of 24 bits
    if (port->TransactSPI(0x30, data_l, nullptr, 4) != 0)
        return -1;

    if (InitFPGATest(0x08, -1) == -1)
        return -1;

    for (int i = 0; i < 5; i++)
    {
        if (port->TransactSPI(0x30, data_h, nullptr, 4) != 0)
            return -1;
        if (port->TransactSPI(0x30, data_l, nullptr, 4) != 0)
            return -1;
    }

    unsigned adf_cnt;
    if (port->ReadRegister(0x74, adf_cnt) != 0)
        return -1;

    std::string str = "  Result: " + std::to_string(adf_cnt);
    if (adf_cnt != 10)
    {
            str += " - FAILED";
            UpdateStatus(LMS_TEST_INFO, str.c_str());
            return -1;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, str.c_str());
    return 0;
}


int LimeSDRTest_USB::RFTest()
{
    auto testPath = [this](float rxfreq, int gain, int rxPath)->bool{
        const float tx_offset = 5e6;
        bool passed = true;
        if (device->GetConnection()->WriteRegister(0x17, rxPath==1? 0x11:0x44) != 0)
            return false;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Tx, rxfreq+tx_offset)!=0)
            return false;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Rx, rxfreq)!=0)
            return false;

        for (int ch = 0; ch < 2; ch++) {
            RFTestData testinfo = {rxfreq, rxfreq+tx_offset, -15, tx_offset, ch};
            device->SetPath(false, ch, rxPath);
            device->SetPath(true, ch, rxPath==4? 1 : 2);
            if (rxPath == 1)
                device->SetGain(false,ch, gain);
            else
                device->WriteParam(LMS7_G_RXLOOPB_RFE, gain, ch);
            bool testPassed = RunTest(testinfo.peakval,testinfo.peakfreq, ch);
            std::string str = RFTestInfo(testinfo, testPassed);
            UpdateStatus(LMS_TEST_INFO, str.c_str());
            if (testPassed == false)
                passed = false;
        }
        return passed;
    };
    UpdateStatus(LMS_TEST_INFO, "Note: The test should be run without anything connected to RF ports");
    UpdateStatus(LMS_TEST_INFO, "->Configure LMS");
    device->Init();
    if (device->SetRate(61.44e6, 0)!=0)
    {
        UpdateStatus(LMS_TEST_FAIL, "Failed to set sample rate");
        return -1;
    }

    device->EnableChannel(true, 0, true);
    device->SetTestSignal(true, 0, LMS_TESTSIG_DC, 0x7000, 0x7000);
    device->EnableChannel(true, 1, true);
    device->EnableChannel(false, 1, true);
    device->SetTestSignal(true, 1, LMS_TESTSIG_DC, 0x7000, 0x7000);

    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_2-> LNA_L):");
    bool passed = testPath(800e6, 1, 5);
    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_1 -> LNA_W):");
    passed &= testPath(1800e6, 3, 4);
    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_2-> LNA_H):");
    passed &= testPath(2500e6, 21, 1);

    return passed ? 0 : -1;
}
