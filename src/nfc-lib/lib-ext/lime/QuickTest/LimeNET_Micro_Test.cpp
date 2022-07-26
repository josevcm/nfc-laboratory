#include "LimeSDRTest.h"

using namespace lime;

int LimeNET_Micro_Test::ClockNetworkTest()
{
    int ret = 0;
    UpdateStatus(LMS_TEST_INFO, "->REF clock test");
    if (GPIFClkTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, "->REF clock test FAILED");
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


int LimeNET_Micro_Test::VCTCXOTest()
{
    const uint8_t id = 0;
    double val = 0;
    unsigned count1;
    unsigned count2;
    double dac_value;
    std::string units = "";
    auto conn = device->GetConnection();

    if (conn->CustomParameterRead(&id, &dac_value, 1, nullptr) != 0)
        return -1;

    auto restore_dac = [&,id](int ret){
        if (conn->CustomParameterWrite(&id, &dac_value, 1, units) != 0)
            return -1;
        return ret;
    };

    if (conn->CustomParameterWrite(&id, &val, 1, units) != 0)
        return -1;

    if ((InitFPGATest(0x04, 1.0) & 0x4) != 0x4)
        return restore_dac(-1);

    uint32_t addr[] = { 0x72, 0x73 };
    uint32_t vals[2];
    if (conn->ReadRegisters(addr, vals, 2) != 0)
        return restore_dac(-1);

    count1 = vals[0] + (vals[1] << 16);
    val = 65500;
    if (conn->CustomParameterWrite(&id, &val, 1, units) != 0)
        return -1;

    if ((InitFPGATest(0x04, 1.0) & 0x4) != 0x4)
        return restore_dac(-1);;

    if (conn->ReadRegisters(addr, vals, 2) != 0)
        return restore_dac(-1);;

    count2 = vals[0] + (vals[1] << 16);
    std::string str = "  Results : " + std::to_string(count1) + " (min); " + std::to_string(count2) + " (max)";

    if ((count1 + 16 > count2) || (count1 + 64 < count2))
    {
        str += " - FAILED";
        UpdateStatus(LMS_TEST_INFO, str.c_str());
        return restore_dac(-1);;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, str.c_str());
    return restore_dac(0);
}

int LimeNET_Micro_Test::RFTest()
{
    auto testPath = [this](float rxfreq, int gain, int lbPath)->bool{
        const float tx_offset = 2e6;
        bool passed = true;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Tx, rxfreq+tx_offset)!=0)
            return false;
        if (device->GetLMS()->SetFrequencySX(lime::LMS7002M::Rx, rxfreq)!=0)
            return false;
        RFTestData testinfo = {rxfreq, rxfreq+tx_offset, -14, tx_offset, 0};
        device->GetLMS()->Modify_SPI_Reg_bits(LMS7param(MAC), 1);  //channel A
        device->GetLMS()->SetPathRFE(lbPath==2 ? LMS7002M::PATH_RFE_LB2 : LMS7002M::PATH_RFE_LB1);
        device->GetLMS()->SetBandTRF(lbPath);
        if (lbPath==1)
            device->GetLMS()->Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 1);
        if (device->GetConnection()->WriteRegister(0x17, lbPath==1? 0x5601:0x6501) != 0)
            return false;

        device->WriteParam(LMS7_G_RXLOOPB_RFE, gain);
        bool testPassed = RunTest(testinfo.peakval,testinfo.peakfreq, 0);
        std::string str = RFTestInfo(testinfo, testPassed);
        UpdateStatus(LMS_TEST_INFO, str.c_str());
        if (testPassed == false)
            passed = false;
        return passed;
    };
    UpdateStatus(LMS_TEST_INFO, "->Configure LMS");
    device->Init();
    if (device->SetRate(8e6, 0)!=0)
    {
        UpdateStatus(LMS_TEST_FAIL, "Failed to set sample rate");
        return -1;
    }

    device->EnableChannel(true, 0, true);
    device->SetTestSignal(true, 0, LMS_TESTSIG_DC, 0x6000, 0x6000);
    UpdateStatus(LMS_TEST_INFO, "->Testing using internal LMS7002M loopback");
    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_1-> LNA_H):");
    bool passed = testPath(1800e6, 5, 1);
    UpdateStatus(LMS_TEST_INFO, "->Run Tests (TX_2 -> LNA_L):");
    passed &= testPath(750e6, 2, 2);

    return passed ? 0 : -1;
}
