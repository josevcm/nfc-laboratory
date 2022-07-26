/*
 * File:   lms7_device.h
 * Author: ignas
 *
 * Created on March 9, 2016, 12:54 PM
 */

#ifndef LMS7_DEVICE_H
#define	LMS7_DEVICE_H
#include "LMS7002M.h"
#include "lime/LimeSuite.h"
#include <vector>
#include <string>
#include "Streamer.h"
#include "IConnection.h"

class RFE_Device;

namespace lime
{
class LIME_API LMS7_Device
{
public:
    struct Range {
        Range(double a = 0, double b = 0){ min = a, max = b; };
        double min;
        double max;
    };
    virtual ~LMS7_Device();
    LMS7_Device(LMS7_Device *obj = nullptr);
    lime::IConnection* GetConnection(unsigned chan =0);
    lime::FPGA* GetFPGA();
    virtual int Init();
    virtual int EnableChannel(bool dir_tx, unsigned chan, bool enabled);
    int Reset();
    virtual unsigned GetNumChannels(const bool tx=false) const;
    virtual int SetRate(double f_MHz, int oversample);
    virtual int SetRate(bool tx, double f_MHz, unsigned oversample = 0);
    virtual int SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample = 0);
    int SetFPGAInterfaceFreq(int interp = -1, int dec = -1, double txPhase = 999, double rxPhase = 999);
    virtual double GetRate(bool tx, unsigned chan, double *rf_rate_Hz = NULL) const;
    virtual Range GetRateRange(bool dir = false, unsigned chan = 0)const;
    virtual std::vector<std::string> GetPathNames(bool dir_tx, unsigned chan = 0) const;
    virtual int SetPath(bool tx,unsigned chan, unsigned path);
    int GetPath(bool tx, unsigned chan) const;
    virtual int SetFrequency(bool tx, unsigned chan, double f_Hz);
    double GetFrequency(bool tx, unsigned chan) const;
    virtual Range GetFrequencyRange(bool tx) const;
    virtual Range GetRxPathBand(unsigned path, unsigned chan) const;
    virtual Range GetTxPathBand(unsigned path, unsigned chan) const;
    int SetLPF(bool tx, unsigned chan, bool en, double bandwidth=-1);
    double GetLPFBW(bool tx,unsigned chan) const;
    Range GetLPFRange(bool tx,unsigned chan) const;
    int SetGFIRCoef(bool tx, unsigned chan, lms_gfir_t filt, const double* coef, unsigned count);
    int GetGFIRCoef(bool tx, unsigned chan, lms_gfir_t filt, double* coef) const;
    int SetGFIR(bool tx, unsigned chan, lms_gfir_t filt, bool enabled);
    int SetGain(bool dir_tx, unsigned chan, double value, const std::string &name = "");
    double GetGain(bool dir_tx, unsigned chan, const std::string &name = "") const;
    Range GetGainRange(bool dir_tx, unsigned chan, const std::string &name = "") const;
    int SetTestSignal(bool dir_tx, unsigned chan, lms_testsig_t sig, int16_t dc_i = 0, int16_t dc_q = 0);
    int GetTestSignal(bool dir_tx, unsigned chan) const;
    int SetNCOFreq(bool tx, unsigned ch, int ind, double freq);
    double GetNCOFreq(bool tx, unsigned ch, int ind) const;
    int SetNCOPhase(bool tx, unsigned ch, int ind, double phase);
    double GetNCOPhase(bool tx, unsigned ch, int ind) const;
    virtual int Calibrate(bool dir_tx, unsigned chan, double bw, unsigned flags);
    virtual std::vector<std::string> GetProgramModes() const;
    virtual int Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const;
    double GetClockFreq(unsigned clk_id, int channel = -1) const;
    virtual int SetClockFreq(unsigned clk_id, double freq, int channel = -1);
    lms_dev_info_t* GetInfo();
    int Synchronize(bool toChip);
    int SetLogCallback(void(*func)(const char* cstr, const unsigned int type));
    int EnableCache(bool enable);
    double GetChipTemperature(int ind = -1) const;
    int LoadConfig(const char *filename, int ind = -1);
    int SaveConfig(const char *filename, int ind = -1) const;
    int ReadLMSReg(uint16_t address, int ind = -1) const;
    int WriteLMSReg(uint16_t address, uint16_t val, int ind = -1) const;
    int ReadFPGAReg(uint16_t address) const;
    int WriteFPGAReg(uint16_t address, uint16_t val) const;
    uint16_t ReadParam(const struct LMS7Parameter& param, int channel = -1, bool forceReadFromChip = false) const;
    int ReadParam(const std::string& param, int channel = -1, bool forceReadFromChip = false) const;
    int WriteParam(const struct LMS7Parameter& param, uint16_t val, int channel = -1);
    int WriteParam(const std::string& param, uint16_t val, int channel = -1);
    int SetActiveChip(unsigned ind);
    lime::LMS7002M* GetLMS(int index = -1) const;
    int UploadWFM(const void **samples, uint8_t chCount, int sample_count, lime::StreamConfig::StreamDataFormat fmt) const;
    static LMS7_Device* CreateDevice(const lime::ConnectionHandle& handle, LMS7_Device *obj = nullptr);
    static std::vector<lime::ConnectionHandle> GetDeviceList();
    int ConfigureGFIR(bool tx, unsigned ch, bool enabled, double bandwidth);

    lime::StreamChannel* SetupStream(const lime::StreamConfig &config);
    int DestroyStream(lime::StreamChannel* streamID);
    uint64_t GetHardwareTimestamp(void) const;
    void SetHardwareTimestamp(const uint64_t now);

    int MCU_AGCStart(uint32_t wantedRSSI);
    int MCU_AGCStop();

    RFE_Device* GetLimeRFE() const;
    void SetLimeRFE(RFE_Device* dev);

protected:

    struct ChannelInfo
    {
    public:
        ChannelInfo(): lpf_bw(0), gfir_bw(-1.0), cF_offset_nco(0), sample_rate(30e6), freq(-1.0){}
        double lpf_bw;
        double gfir_bw;
        double cF_offset_nco;
        double sample_rate;
        double freq;
    };
    lms_dev_info_t devInfo;
    std::vector<ChannelInfo> tx_channels;
    std::vector<ChannelInfo> rx_channels;
    lime::IConnection* connection;
    std::vector<lime::LMS7002M*> lms_list;
    lime::LMS7002M* SelectChannel(unsigned chan) const;
    unsigned lms_chip_id;
    std::vector<lime::Streamer*> mStreamers;
    lime::FPGA* fpga;
    RFE_Device* limeRFE;
};

}

#endif	/* LMS7_DEVICE_H */

