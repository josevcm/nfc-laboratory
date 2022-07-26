#include <chrono>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "LimeSDRTest.h"
#include "VersionInfo.h"
#include <thread>
#include "kiss_fft.h"
#include "FPGA_Mini.h"
#include <ctime>
#include <iostream>

using namespace lime;

const std::vector<std::string>testNames =
{"Clock Network Test", "FPGA EEPROM Test", "LMS7002M Test", "RF Loopback Test"};

unsigned LimeSDRTest::step = 0;
std::atomic<bool> LimeSDRTest::running(false);
LimeSDRTest::TestCallback LimeSDRTest::callback = nullptr;
std::chrono::steady_clock::time_point LimeSDRTest::tp_start;

LimeSDRTest::LimeSDRTest(LMS7_Device* dev)
{
    step = 0;
    device = dev;
}

LimeSDRTest::~LimeSDRTest()
{
    if (device != nullptr)
    {
        delete device;
        device = nullptr;
    }
}

void LimeSDRTest::UpdateStatus(int event, const char* msg)
{
    if (callback)
        callback(step,event,msg);
}

void LimeSDRTest::OnLogEvent(const lime::LogLevel level, const char *msg)
{
    if (running.load() && level<= 2)
        UpdateStatus(LMS_TEST_INFO,msg);
}

LimeSDRTest* LimeSDRTest::Connect()
{
    auto handles = LMS7_Device::GetDeviceList();

    if (handles.size() < 1)
    {
        UpdateStatus(LMS_TEST_FAIL, "Error: No Devices Connected");
        return nullptr;
    }
    if (handles.size() > 1)
    {
        UpdateStatus(LMS_TEST_INFO, "Warning: Multiple devices detected, connecting to the first device");
    }

    auto dev = LMS7_Device::CreateDevice(handles[0]);

    if (dev == nullptr)
    {
        UpdateStatus(LMS_TEST_FAIL, "Error: Unable to connect");
        return nullptr;
    }

    auto info = dev->GetInfo();
    if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDR))
        || strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI)))
    {
        std::string str = "->Device: ";
        str += handles[0].serialize();
        str += ", HW=" + std::string(info->hardwareVersion);
        str += ", GW=" + std::string(info->gatewareVersion);

        UpdateStatus(LMS_TEST_INFO, str.c_str());
        if (str.find("USB 3") == std::string::npos)
        {
            str = "Warning: USB3 not available";
            UpdateStatus(LMS_TEST_INFO, str.c_str());
        }
        UpdateStatus(LMS_TEST_LOGFILE, handles[0].serial.c_str());
        
        str = " Chip temperature: " + std::to_string(int(dev->GetChipTemperature())) + " C";
        UpdateStatus(LMS_TEST_INFO, str.c_str());
    }

    if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDR)))
        return new LimeSDRTest_USB(dev);
    else if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI)))
        return new LimeSDRTest_Mini(dev);
    else if (handles[0].media.find("USB")!=std::string::npos && strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMENET_MICRO)))
        return new LimeNET_Micro_Test(dev);
    else
    {
        UpdateStatus(LMS_TEST_FAIL, "Board not supported");
        return nullptr;
    }

}

int LimeSDRTest::TransferLMS64C(unsigned char* packet)
{
    auto stream = dynamic_cast<LMS64CProtocol*>(device->GetConnection());
    if (stream->Write(packet, 64) != 64)
    {
        UpdateStatus(LMS_TEST_FAIL, "  Error:Write failed");
        return -1;
    }
    memset(packet, 0, 64);
    if (stream->Read(packet, 64, 5000) != 64)
    {
        UpdateStatus(LMS_TEST_FAIL, "  Error: Read failed");
        return -1;
    }

    if (packet[1] != 1)
    {
        std::string str = "Operation failed: error code " + std::to_string(packet[1]);
        UpdateStatus(LMS_TEST_FAIL, str.c_str());
        return -1;
    }
    return 0;
}

int LimeSDRTest::FPGA_EEPROM_Test()
{
    unsigned char buf[64];
    char str[64];
    UpdateStatus(LMS_TEST_INFO, "->Read EEPROM");

    memset(buf, 0, 64);
    buf[0] = CMD_MEMORY_RD;
    buf[2] = 56;
    buf[13] = 7;				//read count
    buf[19] = 3;

    if (TransferLMS64C(buf) != 0)
        return -1;

    std::snprintf(str, sizeof(str), "->Read data: %02X %02X %02X %02X %02X %02X %02X", buf[32], buf[33], buf[34], buf[35], buf[36], buf[37], buf[38]);
    UpdateStatus(LMS_TEST_INFO, str);

    if (buf[32]<16 || buf[35]<16 || buf[33]>12 || buf[36]>12 || buf[34]>31 || buf[37]>31 || buf[34]==0 || buf[37]==0)
        return -1;
    return 0;
}

int LimeSDRTest::LMS7002mTest()
{
    UpdateStatus(LMS_TEST_INFO, "->Perform Registers Test");

    auto lmsControl = device->GetLMS();
    int st = lmsControl->SPI_write(0xA6, 0x0001);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, "SPI_write() failed");
    st = lmsControl->SPI_write(0x92, 0xFFFF);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, "SPI_write() failed");
    st = lmsControl->SPI_write(0x93, 0x03FF);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, "SPI_write() failed");

    if (lmsControl->RegistersTest(nullptr) != 0)
        return -1;

    lmsControl->ResetChip();

    UpdateStatus(LMS_TEST_INFO, "->External Reset line test");
    if (lmsControl->SPI_write(0x0020, 0xFFFD) != 0)
        return -1;
    int status;
    int val = lmsControl->SPI_read(0x20, true, &status);
    if (status != 0)
        return -1;

    char str[64];
    std::snprintf(str, sizeof(str), "  Reg 0x20: Write value 0xFFFD, Read value 0x%04X", val);
    UpdateStatus(LMS_TEST_INFO, str);
    if (val != 0xFFFD)
        return -1;

    lmsControl->ResetChip();
    val = lmsControl->SPI_read(0x20, true, &status);
    if (status != 0)
        return -1;

    std::snprintf(str, sizeof(str), "  Reg 0x20: value after reset 0x0%4X", val);
    UpdateStatus(LMS_TEST_INFO, str);
    if (val != 0xFFFF)
        return -1;
    return 0;
}

int LimeSDRTest::InitFPGATest(unsigned test, double timeout)
{
    auto conn = device->GetConnection();
    unsigned completed;
    uint32_t addr[] = { 0x61, 0x63};
    uint32_t vals[] = { 0x0, 0x0};
    if (conn->WriteRegisters(addr, vals, 2) != 0)
        return -1;

    auto start = std::chrono::steady_clock::now();
    if (conn->WriteRegister(0x61, test) != 0)
        return -1;

    if (timeout < 0)
        return 0;

    while (1)
    {
        if (conn->ReadRegister(0x65, completed) != 0)
            return -1;

        if ((completed & test) == test)
            return completed;

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        if (elapsed_seconds.count() > timeout)
        {
            std::string str = "  Test Timeout. Last result: ";
            str += std::to_string(completed & test);
            UpdateStatus(LMS_TEST_INFO, str.c_str());
            return completed;
        }
    }
}

int LimeSDRTest::GPIFClkTest()
{
    if ((InitFPGATest(0x01, 1.0) & 0x1) != 0x1)
        return -1;

    uint32_t addr[] = { 0x69, 0x69, 0x69 };
    uint32_t vals[3];
    auto conn = device->GetConnection();
    if (conn->ReadRegisters(addr, vals, 3) != 0)
        return -1;

    std::string str = "  Test results: " + std::to_string(vals[0]) + "; " + std::to_string(vals[1]) + "; " + std::to_string(vals[2]);
    if (vals[0] == vals[1] && vals[1] == vals[2])
    {
        str += " - FAILED";
        UpdateStatus(LMS_TEST_INFO, str.c_str());
        return -1;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, str.c_str());
    return 0;
}

int LimeSDRTest::VCTCXOTest()
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
    val = 254;
    if (conn->CustomParameterWrite(&id, &val, 1, units) != 0)
        return -1;

    if ((InitFPGATest(0x04, 1.0) & 0x4) != 0x4)
        return restore_dac(-1);;

    if (conn->ReadRegisters(addr, vals, 2) != 0)
        return restore_dac(-1);;

    count2 = vals[0] + (vals[1] << 16);
    std::string str = "  Results : " + std::to_string(count1) + " (min); " + std::to_string(count2) + " (max)";

    if ((count1 + 50 > count2) || (count1 + 300 < count2))
    {
        str += " - FAILED";
        UpdateStatus(LMS_TEST_INFO, str.c_str());
        return restore_dac(-1);;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, str.c_str());
    return restore_dac(0);
}

int LimeSDRTest::Reg_write(uint16_t address, uint16_t data)
{
    auto conn = device->GetConnection();
    if (conn == nullptr) return -1;
    int status;
    status = conn->WriteRegister(address, data);
    return status == 0 ? 0 : -1;
}

uint16_t LimeSDRTest::Reg_read(uint16_t address)
{
    auto conn = device->GetConnection();
    if (conn == nullptr) return 0;
    uint32_t dataRd = 0;
    int status;
    status = conn->ReadRegister(address, dataRd);
    if (status == 0)
        return dataRd & 0xFFFF;
    else
        return 0;
}

bool LimeSDRTest::RunTest(float &peakval, float &peakfreq, int ch)
{
    const double rfTestTolerance_dB = 6.0;
    const double rfTestTolerance_Hz = 50e3;

    float samplerate = device->GetRate(false,0);
    float peakAmplitude = -1000, peakFrequency = 0;
    bool passed = false;

    const unsigned int buffer_size = 16*sizeof(lime::FPGA_DataPacket);
    const int fftSize = buffer_size/8;

    char data_buffer[buffer_size] = {0};
    long totalBytesReceived = 0;

    uint16_t regA = Reg_read(0x000A)&(~0x3);     //switch off Rx/Tx
    Reg_write(0x000A, regA);
    Reg_write(0x0007, 1<<ch);
    Reg_write(0x0008, device->ReadParam(LMS7_LML1_SISODDR)? 0x40 : 0x100);

    //Receive samples
    auto conn = device->GetConnection();
    conn->ResetStreamBuffers();
    int handle = conn->BeginDataReading(data_buffer, buffer_size,0);
    Reg_write(0x000A, regA | 0x1); //switch on Rx
    if (conn->WaitForReading(handle, 1000) == false)
    {
        UpdateStatus(LMS_TEST_INFO, "  Failed to read samples: Timeout");
        return false;
    }
    if (( totalBytesReceived =  conn->FinishDataReading(data_buffer, buffer_size, handle))<=0)
    {
        UpdateStatus(LMS_TEST_INFO, "  Failed to read samples: No samples received");
        return false;
    }

    Reg_write(0x000A, regA & ~0x3); //stop Tx Rx

    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cfg m_kissBuffer = kiss_fft_alloc(fftSize, 0, NULL, NULL);
    int16_t* bufStart = (int16_t*)(data_buffer+2*sizeof(lime::FPGA_DataPacket)); // pointer buffer
    //Parse samples from packet
    for (int samplesCollected = 0; samplesCollected < fftSize; samplesCollected++)
    {
        if ((samplesCollected%1020) == 0)		//skip packet headers
            bufStart += 8;
        m_fftCalcIn[samplesCollected].r = (float)*bufStart++;
        m_fftCalcIn[samplesCollected].i = (float)*bufStart++;
    }

    kiss_fft(m_kissBuffer, m_fftCalcIn, m_fftCalcOut);    // FFT calculation

    for (int i = 1; i<fftSize; ++i)                     //skip DC
    {
        float output = 20*log10(sqrt(m_fftCalcOut[i].r*m_fftCalcOut[i].r + m_fftCalcOut[i].i*m_fftCalcOut[i].i)/(fftSize*(1<<15)));
        if (output > peakAmplitude)
        {
            peakAmplitude = output;
            peakFrequency = i * samplerate / fftSize;
        }
    }

    if (peakFrequency > samplerate/2)
        peakFrequency = peakFrequency-samplerate;

    if ((peakAmplitude > peakval - rfTestTolerance_dB)
     && (peakAmplitude < peakval + rfTestTolerance_dB)
     && (peakFrequency < peakfreq+ rfTestTolerance_Hz)
     && (peakFrequency > peakfreq- rfTestTolerance_Hz))
    {
        passed = true;
    }

    free(m_kissBuffer);
    delete[] m_fftCalcOut;
    delete[] m_fftCalcIn;

    peakval = peakAmplitude;
    peakfreq = peakFrequency;

    return passed;
}

std::string LimeSDRTest::RFTestInfo(const RFTestData& data, bool passed)
{
    char buffer[128];
    if (data.peakval < -999)
        snprintf(buffer, sizeof(buffer)-1, "  CH%d (SXR=%3.1fMHz, SXT=%3.1fMHz) - ", data.ch, data.rxfreq / 1e6, data.txfreq / 1e6);
    else
    snprintf(buffer, sizeof(buffer)-1, "  CH%d (SXR=%3.1fMHz, SXT=%3.1fMHz): Result:(%3.1f dBFS, %3.2f MHz) - ", data.ch, data.rxfreq/1e6, data.txfreq/1e6,data.peakval,data.peakfreq/1e6);
    std::string str = buffer;
    str += passed == true ? "PASSED" : "FAILED";
    return str;
}

int LimeSDRTest::Perform_tests()
{
    int status = 0;
    step = 0;
    int tests_failed = 0;
    while (running.load() == true)
    {
        int ret = 0;
        std::string str;
        if (step < testNames.size())
        {
            str = "\n[ " + testNames[step] + " ]";
            UpdateStatus(LMS_TEST_INFO, str.c_str());
        }
        else
        {
            callback(-1, status == 0 ? LMS_TEST_SUCCESS : LMS_TEST_FAIL, status == 0 ? "\n=> Board tests PASSED <=\n" : "\n=> Board tests FAILED <=\n");
            running.store(false);
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> duration = end - tp_start;
            char str[64];
            std::snprintf(str, sizeof(str), "Elapsed time: %1.2f seconds\n",duration.count());
            callback(-1,  LMS_TEST_INFO, str);
            break;
        }

        switch (step)
        {
            case 0: ret = ClockNetworkTest(); break;
            case 1: ret = FPGA_EEPROM_Test(); break;
            case 2: ret = LMS7002mTest(); break;
            case 3: ret = RFTest(); break;
        }

        str = "->" + testNames[step];
        str += (ret == 0) ? " PASSED" : " FAILED";
        UpdateStatus((ret == 0) ? LMS_TEST_SUCCESS : LMS_TEST_FAIL, str.c_str());
        if (ret != 0)
            tests_failed |= (1<<step);
        status += ret;
        step++;
    }
    running.store(false);
    delete this;
    return tests_failed;
}

int LimeSDRTest::RunTests(TestCallback cb, bool nonblock)
{
    lime::registerLogHandler(&LimeSDRTest::OnLogEvent);
    step = -1;
    callback = cb;
    if (running.load() == true)
        return -1;
    running.store(true);
    tp_start = std::chrono::steady_clock::now();
    std::string str = "[ TESTING STARTED ]\n->Start time: ";
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    str += std::ctime(&time);
    str += "->LimeSuite version: " + lime::GetLibraryVersion() + "\n";
    UpdateStatus(LMS_TEST_INFO, str.c_str());

    auto testObj = Connect();
    if (!testObj)
    {
        UpdateStatus(LMS_TEST_FAIL, "Failed to connect");
        running.store(false);
        return -1;
    }
    int ret = 0;
    if (nonblock)
    {
        auto testFunc = std::bind(&LimeSDRTest::Perform_tests, testObj);
        auto workThread = std::thread(testFunc, testObj);
        workThread.detach();
    }
    else
        ret = testObj->Perform_tests();
    return ret;
}

int LimeSDRTest::CheckDevice(std::string &str)
{
    if (running.load() == true)
        return 0;

    auto handles = LMS7_Device::GetDeviceList();
    if (handles.size() < 1)
    {
        str = "Error: No Board Detected";
        return -1;
    }
    else if (handles.size() > 1)
    {
        str = "Error: Multiple Board Detected";
        return -2;
    }
    str = "Device: ";
    str += handles[0].name;

    if (handles[0].serial.length() > 4)
    {
        str += " (";
        str += handles[0].serial + ")";
    }
    return 0;
}