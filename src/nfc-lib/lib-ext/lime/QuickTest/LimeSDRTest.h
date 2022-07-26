/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   LimeSDRTest.h
 * Author: ignas
 *
 * Created on August 25, 2016, 9:50 AM
 */

#ifndef LIMESDRTEST_H
#define LIMESDRTEST_H

#include "lms7_device.h"
#include <IConnection.h>
#include <atomic>
#include "Logger.h"

#define LMS_TEST_FAIL       -1
#define LMS_TEST_SUCCESS    0
#define LMS_TEST_INFO       1
#define LMS_TEST_LOGFILE    4

extern const std::vector<std::string>testNames;

class LimeSDRTest
{
    typedef std::function<int(int testID, int event, const char* msg)> TestCallback;

public:
    static int RunTests(TestCallback cb, bool nonblock = true);
    static int CheckDevice(std::string &str);
    
protected:
    
    struct RFTestData
    {
        float rxfreq;
        float txfreq;
        float peakval;
        float peakfreq;
        int ch;
    };
    
    LimeSDRTest(lime::LMS7_Device* dev);
    virtual ~LimeSDRTest();
    static void UpdateStatus(int event, const char* msg = nullptr);
    int InitFPGATest(unsigned test, double timeout);
    int GPIFClkTest();
    virtual int VCTCXOTest();
    bool RunTest(float &peakval, float &peakFreq, int ch = 0);
    static std::string RFTestInfo(const RFTestData& data, bool passed);	
    lime::LMS7_Device* device;
    
private:
    
    static LimeSDRTest* Connect();
    int FPGA_EEPROM_Test();
    virtual int ClockNetworkTest()=0;
    virtual int RFTest() = 0;
    int LMS7002mTest();
    int Reg_write(uint16_t address, uint16_t data);
    uint16_t Reg_read(uint16_t address);

    static unsigned step;
    static TestCallback callback;
    int Perform_tests();
    int TransferLMS64C(unsigned char* packet);

    static std::atomic<bool> running;
    static std::chrono::steady_clock::time_point tp_start;
    static void OnLogEvent(const lime::LogLevel level, const char *message);
};

class LimeSDRTest_Mini : public LimeSDRTest
{
    friend class LimeSDRTest;
    LimeSDRTest_Mini(lime::LMS7_Device* dev):LimeSDRTest(dev){};
    int ClockNetworkTest() override;
    int RFTest() override;
};

class LimeSDRTest_USB : public LimeSDRTest
{
    friend class LimeSDRTest;

    int ClockNetworkTest() override;
    int RFTest() override;
    int Si5351CTest();
protected:
    int ADF4002Test();
    LimeSDRTest_USB(lime::LMS7_Device* dev):LimeSDRTest(dev){}; 
};

class LimeNET_Micro_Test : public LimeSDRTest_USB
{
    friend class LimeSDRTest;
    LimeNET_Micro_Test(lime::LMS7_Device* dev):LimeSDRTest_USB(dev){};
    int ClockNetworkTest() override;
    int VCTCXOTest() override;
    int RFTest() override;
};

#endif /* LIMESDRTEST_H */

