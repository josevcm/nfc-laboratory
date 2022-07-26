#include "IConnection.h"
#include "ConnectionRegistry.h"
#include "lime/LimeSuite.h"
#include "lms7_device.h"
#include "errno.h"
#include <cmath>
#include "VersionInfo.h"
#include <assert.h>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "Streamer.h"
#include "../limeRFE/RFE_Device.h"

using namespace std;

namespace
{

inline lime::LMS7_Device* CheckDevice(lms_device_t* device)
{
    if (device != nullptr)
        return (lime::LMS7_Device*)device;
    lime::error("Device cannot be NULL.");
    return nullptr;
}

inline lime::LMS7_Device* CheckDevice(lms_device_t* device, unsigned chan)
{
    if (device == nullptr)
    {
        lime::error("Device cannot be NULL.");
        return nullptr;
    }
    lime::LMS7_Device* lms_dev = (lime::LMS7_Device*)device;
    if (chan >= lms_dev->GetNumChannels())
    {
        lime::error("Invalid channel number.");
        return nullptr;
    }
    return lms_dev;
}

inline lime::IConnection* CheckConnection(lms_device_t* device)
{
    if (device == nullptr)
    {
        lime::error("Device cannot be NULL.");
        return nullptr;
    }
    lime::IConnection* conn = ((lime::LMS7_Device*)device)->GetConnection();
    if (conn == nullptr)
    {
        lime::error("Device not connected");
        return nullptr;
    }
    return conn;
}

} //unnamed namespace

API_EXPORT int CALL_CONV LMS_GetDeviceList(lms_info_str_t * dev_list)
{
    std::vector<lime::ConnectionHandle> handles;
    handles = lime::ConnectionRegistry::findConnections();

    if (dev_list != nullptr)
    {
        for (size_t i = 0; i < handles.size(); i++)
        {
            string str = handles[i].serialize();
            if (dev_list[i] == nullptr)
                return -1;
            strncpy(dev_list[i],str.c_str(),sizeof(lms_info_str_t)-1);
            dev_list[i][sizeof(lms_info_str_t)-1]=0;
        }
    }
    return handles.size();
}

API_EXPORT int CALL_CONV LMS_Open(lms_device_t** device, const lms_info_str_t info, void* args)
{
    if (device == nullptr)
    {
        lime::error("Device pointer cannot be NULL");
        return -1;
    }

    std::vector<lime::ConnectionHandle> handles;
    handles = lime::ConnectionRegistry::findConnections();

    for (size_t i = 0; i < handles.size(); i++)
    {
        if (info == NULL || strcmp(handles[i].serialize().c_str(),info) == 0)
        {
            auto dev = lime::LMS7_Device::CreateDevice(handles[i],nullptr);
            if (dev == nullptr)
            {
                lime::error("Unable to open device");
                return -1;
            }
            *device = dev;
            return LMS_SUCCESS;
        }
    }

    lime::error("Specified device could not be found");
    return -1;
}

API_EXPORT int CALL_CONV LMS_Close(lms_device_t * device)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;

    delete lms;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Reset(lms_device_t *device)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->Reset() : -1;
}

API_EXPORT int CALL_CONV LMS_EnableChannel(lms_device_t * device, bool dir_tx, size_t chan, bool enabled)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->EnableChannel(dir_tx, chan,enabled) : -1;
}

API_EXPORT int CALL_CONV LMS_SetSampleRate(lms_device_t * device, float_type rate, size_t oversample)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->SetRate(rate, oversample) : -1;
}

API_EXPORT int CALL_CONV LMS_SetSampleRateDir(lms_device_t *device, bool dir_tx, float_type rate, size_t oversample)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->SetRate(dir_tx,rate,oversample) : -1;
}

API_EXPORT int CALL_CONV LMS_GetSampleRate(lms_device_t *device, bool dir_tx, size_t chan, float_type *host_Hz, float_type *rf_Hz)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    double rate = lms->GetRate(dir_tx,chan,rf_Hz);
    if (host_Hz)
        *host_Hz = rate;

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    auto retRange = lms->GetRateRange(dir_tx);
    range->min = retRange.min;
    range->max = retRange.max;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Init(lms_device_t * device)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->Init() : -1;
}

API_EXPORT int CALL_CONV LMS_ReadCustomBoardParam(lms_device_t *device,
                           uint8_t param_id, float_type *val, lms_name_t units)
{
    auto conn = CheckConnection(device);
    if (conn == nullptr)
        return -1;
    std::string str;
    int ret=conn->CustomParameterRead(&param_id,val,1,&str);
    if (units)
        strncpy(units,str.c_str(),sizeof(lms_name_t)-1);
    return ret;
}

API_EXPORT int CALL_CONV LMS_WriteCustomBoardParam(lms_device_t *device,
                        uint8_t param_id, float_type val, const lms_name_t units)
{
    auto conn = CheckConnection(device);
    if (conn == nullptr)
        return -1;

    std::string str = units == nullptr ? "" : units;
    return conn->CustomParameterWrite(&param_id,&val,1,str);
}

API_EXPORT int CALL_CONV LMS_VCTCXOWrite(lms_device_t * device, uint16_t val)
{
    if (LMS_WriteCustomBoardParam(device, BOARD_PARAM_DAC, val, "")<0)
        return -1;

    auto conn = CheckConnection(device);
    auto port = dynamic_cast<lime::LMS64CProtocol*>(conn);
    if (port) //can use LMS64C protocol to write eeprom value
    {
        lime::DeviceInfo dinfo = port->GetDeviceInfo();
        if (dinfo.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2)) //LimeSDR-Mini v2.x
        {
            unsigned char packet[64] = { 0x8C, 0, 56, 0, 0, 0, 0, 0, 0x02, 0, 0, 0, 0, 2, 0, 0xFF, 0, 0, 0, 1 };//packet: Flash write 2 btes, addr 16
            packet[32] = val & 0xFF;              //values start at offset=32
            packet[33] = val >> 8;

            if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
                return -1;
        }
        else
        {
            unsigned char packet[64] = { 0x8C, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 3 };//packet: eeprom write 2 btes, addr 16
            packet[32] = val & 0xFF;              //values start at offset=32
            packet[33] = val >> 8;

            if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
                return -1;
        }
    }
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_VCTCXORead(lms_device_t * device, uint16_t *val)
{
    auto conn = CheckConnection(device);
    if (!conn)
        return -1;
    auto port = dynamic_cast<lime::LMS64CProtocol*>(conn);
    if (port) //can use LMS64C protocol to read eeprom value
    {
        lime::DeviceInfo dinfo = port->GetDeviceInfo();
        if (dinfo.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2)) //LimeSDR-Mini v2.x
        {
            unsigned char packet[64] = { 0x8D, 0, 56, 0, 0, 0, 0, 0, 0x02, 0, 0, 0, 0, 2, 0, 0xFF, 0, 0, 0, 1 }; //packet: eeprom read 2 bytes, addr 16
            if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
                return -1;
            *val = packet[32] | (packet[33] << 8); //values start at offset=32
        }
        else
        {
            unsigned char packet[64] = { 0x8D, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 3 }; //packet: eeprom read 2 bytes, addr 16
            if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
                return -1;
            *val = packet[32] | (packet[33] << 8); //values start at offset=32
        }
    }
    else //fall back to reading runtime value
    {
        uint8_t id = BOARD_PARAM_DAC;
        double dval;
        if (conn->CustomParameterRead(&id, &dval, 1, nullptr)!=LMS_SUCCESS)
            return -1;
        *val = dval;
    }
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetClockFreq(lms_device_t *device, size_t clk_id, float_type *freq)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    *freq = lms->GetClockFreq(clk_id);
    return *freq > 0 ? 0 : -1;
}

API_EXPORT int CALL_CONV LMS_SetClockFreq(lms_device_t *device, size_t clk_id, float_type freq)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->SetClockFreq(clk_id, freq) : -1;
}

API_EXPORT  int CALL_CONV LMS_Synchronize(lms_device_t *dev, bool toChip)
{
    lime::LMS7_Device* lms = CheckDevice(dev);
    return lms ? lms->Synchronize(toChip) : -1;
}

API_EXPORT int CALL_CONV LMS_GPIORead(lms_device_t *dev,  uint8_t* buffer, size_t len)
{
    auto conn = CheckConnection(dev);
    return conn ? conn->GPIORead(buffer, len) : -1;
}

API_EXPORT int CALL_CONV LMS_GPIOWrite(lms_device_t *dev, const uint8_t* buffer, size_t len)
{
    auto conn = CheckConnection(dev);
    return conn ? conn->GPIOWrite(buffer, len) : -1;
}

API_EXPORT int CALL_CONV LMS_GPIODirRead(lms_device_t *dev,  uint8_t* buffer, size_t len)
{
    auto conn = CheckConnection(dev);
    return conn ? conn->GPIODirRead(buffer, len) : -1;
}

API_EXPORT int CALL_CONV LMS_GPIODirWrite(lms_device_t *dev, const uint8_t* buffer, size_t len)
{
    auto conn = CheckConnection(dev);
    return conn ? conn->GPIODirWrite(buffer,len) : -1;
}

API_EXPORT int CALL_CONV LMS_EnableCache(lms_device_t *dev, bool enable)
{
    lime::LMS7_Device* lms = CheckDevice(dev);
    return lms ? lms->EnableCache(enable) : -1;
}

API_EXPORT int CALL_CONV LMS_GetChipTemperature(lms_device_t *dev, size_t ind, float_type *temp)
{
    *temp = 0;
    lime::LMS7_Device* lms = CheckDevice(dev);
    if (!lms)
        return -1;
    if (lms->ReadLMSReg(0x2F) == 0x3840)
    {
        lime::error("Feature is not available on this chip revision");
        return -1;
    }
    *temp = lms->GetChipTemperature(ind);
    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t * device, bool dir_tx)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->GetNumChannels(dir_tx) : -1;
}

API_EXPORT int CALL_CONV LMS_SetLOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type frequency)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    int ret = lms ? lms->SetFrequency(dir_tx, chan,frequency) : -1;

#ifdef LIMERFE
    auto rfe = lms->GetLimeRFE();
    if (rfe && ret == 0)
        rfe->SetFrequency(dir_tx, chan, frequency);
#endif
    return ret;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type *frequency)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    *frequency = lms->GetFrequency(dir_tx, chan);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    auto retRange = lms->GetFrequencyRange(dir_tx);
    range->min = retRange.min;
    range->max = retRange.max;
    range->step = 0;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t *device, bool dir_tx, size_t chan, lms_name_t *list)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    auto names = lms->GetPathNames(dir_tx, chan);
    if (list != nullptr)
        for (size_t i = 0; i < names.size(); i++)
        {
            strncpy(list[i], names[i].c_str(), sizeof(lms_name_t) - 1);
            list[i][sizeof(lms_name_t) - 1] = 0;
        }
    return names.size();
}

API_EXPORT int CALL_CONV LMS_SetAntenna(lms_device_t *device, bool dir_tx, size_t chan, size_t path)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->SetPath(dir_tx,chan,path) : -1;
}


API_EXPORT int CALL_CONV LMS_GetAntenna(lms_device_t *device, bool dir_tx, size_t chan)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->GetPath(dir_tx, chan) : -1;
}

API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t *device, bool dir_tx, size_t chan, size_t path, lms_range_t *range)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    lime::LMS7_Device::Range ret;
    if (dir_tx)
        ret = lms->GetTxPathBand(path,chan);
    else
        ret = lms->GetRxPathBand(path,chan);

    range->max = ret.max;
    range->min = ret.min;
    range->step = 0;

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetLPFBW(lms_device_t *device, bool dir_tx, size_t chan, float_type bandwidth)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->SetLPF(dir_tx,chan,true,bandwidth) : -1;
}

API_EXPORT int CALL_CONV LMS_GetLPFBW(lms_device_t *device, bool dir_tx, size_t chan, float_type *bandwidth)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;
    *bandwidth = lms->GetLPFBW(dir_tx,chan);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t *device, bool dir_tx, size_t chan, bool enabled)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->SetLPF(dir_tx,chan,enabled,-1) : -1;
}

API_EXPORT int CALL_CONV LMS_SetGFIRLPF(lms_device_t *device, bool dir_tx, size_t chan, bool enabled, float_type bandwidth)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->ConfigureGFIR(dir_tx, chan, enabled, bandwidth) : -1;
}

API_EXPORT int CALL_CONV LMS_GetLPFBWRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;

    auto ret = lms->GetLPFRange(dir_tx,0);
    range->max = ret.max;
    range->min = ret.min;
    range->step = 0;

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t *device, bool dir_tx, size_t chan, float_type gain)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    if (gain > 1.0)
        gain = 1.0;
    else if (gain < 0)
        gain = 0;
    auto range = lms->GetGainRange(dir_tx,chan,"");
    return lms->SetGain(dir_tx,chan,range.min+gain*(range.max-range.min));
}

API_EXPORT int CALL_CONV LMS_SetGaindB(lms_device_t *device, bool dir_tx,
                                                size_t chan,unsigned gain)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
#ifdef NEW_GAIN_BEHAVIOUR
    return lms ? lms->SetGain(dir_tx,chan,dir_tx ? gain : double(gain-12.0f)) : -1;
#else
    return lms ? lms->SetGain(dir_tx,chan,int(gain-12)) : -1;
#endif
}

API_EXPORT int CALL_CONV LMS_GetNormalizedGain(lms_device_t *device, bool dir_tx, size_t chan,float_type *gain)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    auto range = lms->GetGainRange(dir_tx,chan,"");
    *gain = (lms->GetGain(dir_tx,chan)-range.min)/(range.max-range.min);

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetGaindB(lms_device_t *device, bool dir_tx, size_t chan, unsigned *gain)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;
#ifdef NEW_GAIN_BEHAVIOUR
    if(dir_tx) *gain = lms->GetGain(dir_tx, chan) + 0.5;
    else *gain = lms->GetGain(dir_tx,chan)+12+0.5;
#else
    *gain = lms->GetGain(dir_tx,chan)+12+0.5;    
#endif        
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Calibrate(lms_device_t *device, bool dir_tx, size_t chan, double bw, unsigned flags)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

#ifdef LIMERFE
    auto rfe = lms->GetLimeRFE();
    if (rfe)
        rfe->OnCalibrate(chan, false);
#endif
    int ret = lms->Calibrate(dir_tx, chan, bw, flags);

#ifdef LIMERFE
    if (rfe)
        rfe->OnCalibrate(chan, true);
#endif
    return ret;
}

API_EXPORT int CALL_CONV LMS_LoadConfig(lms_device_t *device, const char *filename)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->LoadConfig(filename) : -1;
}

API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t *device, const char *filename)
{
    lime::LMS7_Device* lms = CheckDevice(device);

    return lms ? lms->SaveConfig(filename) : -1;
}

API_EXPORT int CALL_CONV LMS_SetTestSignal(lms_device_t *device, bool dir_tx, size_t chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    if (sig > LMS_TESTSIG_DC)
    {
        lime::error("Invalid signal.");
        return -1;
    }

    return lms->SetTestSignal(dir_tx,chan,sig,dc_i,dc_q);
}

API_EXPORT int CALL_CONV LMS_GetTestSignal(lms_device_t *device, bool dir_tx, size_t chan, lms_testsig_t *sig)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    int tmp = lms->GetTestSignal(dir_tx,chan);
    if (tmp < 0)
        return -1;

    *sig = (lms_testsig_t)tmp;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t *device, bool dir_tx, size_t ch, const float_type *freq, float_type pho)
{
    lime::LMS7_Device* lms = CheckDevice(device, ch);
    if (!lms)
        return -1;

    if (freq != nullptr)
    {
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
            if (lms->SetNCOFreq(dir_tx, ch, i, freq[i]) != 0)
                return -1;
        lms->WriteParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, 0);
        lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch);
    }
    return lms->GetLMS()->SetNCOPhaseOffsetForMode0(dir_tx, pho);
}


API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type *freq, float_type *pho)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    if (freq != nullptr)
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
            freq[i] = std::fabs(lms->GetNCOFreq(dir_tx, chan, i));

    if (pho != nullptr)
    {
        uint16_t value = lms->ReadLMSReg(dir_tx ? 0x0241 : 0x0441, chan/2);
        *pho = 360.0 * value / 65536.0;
    }
    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNCOPhase(lms_device_t *device, bool dir_tx, size_t ch, const float_type *phase, float_type fcw)
{
    lime::LMS7_Device* lms = CheckDevice(device, ch);
    if (!lms)
        return -1;

    if (lms->SetNCOFreq(dir_tx, ch, 0, fcw) != 0)
        return -1;

    if (phase != nullptr)
    {
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
            if (lms->SetNCOPhase(dir_tx, ch, i, phase[i])!=0)
                return -1;
        if ((lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch) != 0))
            return -1;
    }
    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNCOPhase(lms_device_t *device, bool dir_tx, size_t ch, float_type *phase, float_type *fcw)
{
    lime::LMS7_Device* lms = CheckDevice(device, ch);
    if (!lms)
        return -1;

    if (phase != nullptr)
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
            phase[i] = lms->GetNCOPhase(dir_tx, ch, i);

    if (fcw != nullptr)
        *fcw = lms->GetNCOFreq(dir_tx, ch, 0);

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNCOIndex(lms_device_t *device, bool dir_tx, size_t chan, int ind, bool down)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    if ((lms->WriteParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, ind < 0 ? 1 : 0, chan)!=0)
    || (lms->WriteParam(dir_tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, ind < 0 ? 0 : 1, chan)!=0))
        return -1;

    if (ind < LMS_NCO_VAL_COUNT)
    {
            if ((lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX,ind)!=0)
            || (lms->WriteParam(dir_tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP, down)!=0))
                return -1;
    }
    else
    {
        lime::error("Invalid NCO index value");
        return -1;
    }
    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNCOIndex(lms_device_t *device, bool dir_tx, size_t chan)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    if (!lms)
        return -1;

    if (lms->ReadParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, chan) != 0)
    {
        lime::error("NCO is disabled");
        return -1;
    }
    return lms->ReadParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, chan);
}

API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t *device, uint32_t address, uint16_t *val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    *val = lms->ReadLMSReg(address);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t *device, uint32_t address, uint16_t val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->WriteLMSReg(address, val) : -1;
}

API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t *device, uint32_t address, uint16_t *val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    int value = lms->ReadFPGAReg(address);
    if (value < 0)
        return value; // operation failed return error code
    else if (val)
        *val = value;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t *device, uint32_t address, uint16_t val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->WriteFPGAReg(address,val) : -1;
}

API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t *device, struct LMS7Parameter param, uint16_t *val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    *val = lms->ReadParam(param);
    return LMS_SUCCESS;;
}

API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t *device, struct LMS7Parameter param, uint16_t val)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;
    return lms->WriteParam(param, val);
}

API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, const float_type* coef,size_t count)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->SetGFIRCoef(dir_tx,chan,filt,coef,count) : -1;
}

API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->GetGFIRCoef(dir_tx,chan,filt,coef) : -1;
}

API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, bool enabled)
{
    lime::LMS7_Device* lms = CheckDevice(device, chan);
    return lms ? lms->SetGFIR(dir_tx,chan,filt,enabled) : -1;
}

API_EXPORT int CALL_CONV LMS_SetupStream(lms_device_t *device, lms_stream_t *stream)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if(stream == nullptr)
    {
        lime::error("stream cannot be NULL.");
        return -1;
    }

    lime::StreamConfig config;
    config.bufferLength = stream->fifoSize;
    config.channelID = stream->channel;
    config.performanceLatency = stream->throughputVsLatency;
    config.align = stream->channel & LMS_ALIGN_CH_PHASE;
    switch(stream->dataFmt)
    {
        case lms_stream_t::LMS_FMT_F32:
            config.format = lime::StreamConfig::FMT_FLOAT32;
            config.linkFormat = lime::StreamConfig::FMT_INT16;
            break;
        case lms_stream_t::LMS_FMT_I16:
            config.format = lime::StreamConfig::FMT_INT16;
            config.linkFormat = lime::StreamConfig::FMT_INT16;
            break;
        case lms_stream_t::LMS_FMT_I12:
            config.format = lime::StreamConfig::FMT_INT12;
            config.linkFormat = lime::StreamConfig::FMT_INT12;
            break;
        default:
            config.format = lime::StreamConfig::FMT_FLOAT32;
            config.linkFormat = lime::StreamConfig::FMT_INT16;
    }

    switch (stream->linkFmt)
    {
    case lms_stream_t::LMS_LINK_FMT_I16:
        config.linkFormat = lime::StreamConfig::FMT_INT16;
        break;
    case lms_stream_t::LMS_LINK_FMT_I12:
        config.linkFormat = lime::StreamConfig::FMT_INT12;
    case lms_stream_t::LMS_LINK_FMT_DEFAULT: // do nothing
        break;
    }

    config.isTx = stream->isTx;
    stream->handle = size_t(lms->SetupStream(config));
    return stream->handle == 0 ? -1 : 0;
}

API_EXPORT int CALL_CONV LMS_DestroyStream(lms_device_t *device, lms_stream_t *stream)
{
    if(stream == nullptr)
    {
        lime::error("Stream cannot be NULL.");
        return -1;
    }
    if (stream->handle == 0)
    {
        lime::error("Invalid stream handle");
        return -1;
    }

    lime::LMS7_Device* lms =  CheckDevice(device);
    lms->DestroyStream((lime::StreamChannel*)stream->handle);
    stream->handle = 0;
    return 0;
}

API_EXPORT int CALL_CONV LMS_StartStream(lms_stream_t *stream)
{
    if (stream==nullptr || stream->handle==0)
        return 0;
    return reinterpret_cast<lime::StreamChannel*>(stream->handle)->Start();
}

API_EXPORT int CALL_CONV LMS_StopStream(lms_stream_t *stream)
{
    if (stream==nullptr || stream->handle==0)
        return 0;
    return reinterpret_cast<lime::StreamChannel*>(stream->handle)->Stop();
}

API_EXPORT int CALL_CONV LMS_RecvStream(lms_stream_t *stream, void *samples, size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms)
{
    if (stream==nullptr || stream->handle==0)
        return -1;
    lime::StreamChannel* channel = (lime::StreamChannel*)stream->handle;
    lime::StreamChannel::Metadata metadata;
    metadata.flags = 0;
    if (meta)
    {
        metadata.flags |= meta->waitForTimestamp * lime::RingFIFO::SYNC_TIMESTAMP;
        metadata.timestamp = meta->timestamp;
    }
    else metadata.timestamp = 0;

    int status = channel->Read(samples, sample_count, &metadata, timeout_ms);
    if (meta)
        meta->timestamp = metadata.timestamp;
    return status;
}

API_EXPORT int CALL_CONV LMS_SendStream(lms_stream_t *stream, const void *samples, size_t sample_count, const lms_stream_meta_t *meta, unsigned timeout_ms)
{
    if (stream==nullptr || stream->handle==0)
        return -1;
    lime::StreamChannel* channel = (lime::StreamChannel*)stream->handle;
    lime::StreamChannel::Metadata metadata;
    metadata.flags = 0;
    if (meta)
    {
        metadata.flags |= meta->waitForTimestamp * lime::RingFIFO::SYNC_TIMESTAMP;
        metadata.flags |= meta->flushPartialPacket * lime::RingFIFO::END_BURST;
        metadata.timestamp = meta->timestamp;
    }
    else metadata.timestamp = 0;

    return channel->Write(samples, sample_count, &metadata, timeout_ms);
}

API_EXPORT int CALL_CONV LMS_UploadWFM(lms_device_t *device,
                                         const void **samples, uint8_t chCount,
                                         size_t sample_count, int format)
{
    lime::LMS7_Device* lms = (lime::LMS7_Device*)device;
    lime::StreamConfig::StreamDataFormat fmt;
    switch(format)
    {
        case 0:
            fmt = lime::StreamConfig::StreamDataFormat::FMT_INT12;
            break;
        case 1:
            fmt = lime::StreamConfig::StreamDataFormat::FMT_INT16;
            break;
        case 2:
            fmt = lime::StreamConfig::StreamDataFormat::FMT_FLOAT32;
            break;
        default:
            fmt = lime::StreamConfig::StreamDataFormat::FMT_INT12;
            break;
    }
    return lms->UploadWFM(samples, chCount, sample_count, fmt);
}

API_EXPORT int CALL_CONV LMS_EnableTxWFM(lms_device_t *device, unsigned ch, bool active)
{
    uint16_t regAddr = 0x000D;
    uint16_t regValue = 0;
    int status = 0;
    status = LMS_WriteFPGAReg(device, 0xFFFF, 1<<(ch/2));
    if (status != 0)
        return status;
    status = LMS_ReadFPGAReg(device, regAddr, &regValue);
    if(status != 0)
        return status;
    regValue = regValue & ~0x6; //clear WFM_LOAD, WFM_PLAY
    regValue |= active << 1;
    status = LMS_WriteFPGAReg(device, regAddr, regValue);
    return status;
}

API_EXPORT int CALL_CONV LMS_GetStreamStatus(lms_stream_t *stream, lms_stream_status_t* status)
{
    assert(stream != nullptr);
    lime::StreamChannel* channel = (lime::StreamChannel*)stream->handle;
    if(channel == nullptr)
        return -1;
    lime::StreamChannel::Info info = channel->GetInfo();

    status->active = info.active;
    status->droppedPackets = info.droppedPackets;
    status->fifoFilledCount = info.fifoItemsCount;
    status->fifoSize = info.fifoSize;
    status->linkRate = info.linkRate;
    status->overrun = info.overrun;
    status->underrun = info.underrun;
    status->timestamp = info.timestamp;
    return LMS_SUCCESS;
}

API_EXPORT const lms_dev_info_t* CALL_CONV LMS_GetDeviceInfo(lms_device_t *device)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    return lms ? lms->GetInfo() : nullptr;
}

API_EXPORT int CALL_CONV LMS_GetProgramModes(lms_device_t *device, lms_name_t *list)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;

    auto names = lms->GetProgramModes();
    if (list != nullptr)
        for (size_t i = 0; i < names.size(); i++)
        {
            strncpy(list[i], names[i].c_str(), sizeof(lms_name_t) - 1);
            list[i][sizeof(lms_name_t) - 1] = 0;
        }
    return names.size();
}

API_EXPORT int CALL_CONV LMS_Program(lms_device_t *device, const char *data, size_t size, const lms_name_t mode, lms_prog_callback_t callback)
{
    lime::LMS7_Device* lms = CheckDevice(device);
    if (!lms)
        return -1;

    std::string prog_mode(mode);
    return lms->Program(prog_mode, data, size, callback);
}

API_EXPORT const char * CALL_CONV LMS_GetLastErrorMessage(void)
{
    return lime::GetLastErrorMessage();
}

API_EXPORT const char* LMS_GetLibraryVersion()
{
    static char libraryVersion[32];
    sprintf(libraryVersion, "%.31s", lime::GetLibraryVersion().c_str());
    return libraryVersion;
}

static LMS_LogHandler api_msg_handler;
static void APIMsgHandler(const lime::LogLevel level, const char *message)
{
    api_msg_handler(level,message);
}

API_EXPORT void LMS_RegisterLogHandler(LMS_LogHandler handler)
{
    if (handler)
    {
        lime::registerLogHandler(APIMsgHandler);
        api_msg_handler = handler;
    }
    else
        lime::registerLogHandler(nullptr);
}

extern "C" API_EXPORT int CALL_CONV LMS_TransferLMS64C(lms_device_t *dev, int cmd, uint8_t* data, size_t *len)
{
    auto conn = CheckConnection(dev);
    if (!conn)
        return -1;

    lime::LMS64CProtocol::GenericPacket pkt;

    pkt.cmd = lime::eCMD_LMS(cmd);
    for (size_t i = 0; i < *len; ++i)
        pkt.outBuffer.push_back(data[i]);

    lime::LMS64CProtocol* port = dynamic_cast<lime::LMS64CProtocol *>(conn);
    if (port->TransferPacket(pkt) != 0)
        return -1;

    for (size_t i = 0; i < pkt.inBuffer.size(); ++i)
        data[i] = pkt.inBuffer[i];
    *len = pkt.inBuffer.size();

    if (pkt.status != lime::STATUS_COMPLETED_CMD)
    {
        lime::error("%s", lime::status2string(pkt.status));
        return -1;
    }

    return LMS_SUCCESS;
}