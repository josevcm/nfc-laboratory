/*
 * File:   lms7_device.cpp
 * Author: ignas
 *
 * Created on March 9, 2016, 12:54 PM
 */
#include <cmath>

#include "lms7_device.h"
#include "qLimeSDR.h"
#include "LimeSDR_mini.h"
#include "LimeNET_micro.h"
#include "LimeSDR.h"
#include "LimeSDR_PCIE.h"
#include "LimeSDR_Core.h"
#include "GFIR/lms_gfir.h"
#include "IConnection.h"
#include "dataTypes.h"
#include "MCU_BD.h"
#include "FPGA_common.h"
#include "ConnectionRegistry.h"
#include "ADF4002.h"
#include "mcu_programs.h"
#include "Logger.h"
#include "device_constants.h"
#include "LMSBoards.h"

namespace lime
{

std::vector<lime::ConnectionHandle> LMS7_Device::GetDeviceList()
{
    return lime::ConnectionRegistry::findConnections();
}

LMS7_Device* LMS7_Device::CreateDevice(const lime::ConnectionHandle& handle, LMS7_Device *obj)
{
    LMS7_Device* device;
    if (obj)
    {
        lime::ConnectionRegistry::freeConnection(obj->connection);
        obj->connection = nullptr;
    }

    auto conn = lime::ConnectionRegistry::makeConnection(handle);
    if (!conn)
        return nullptr;

    if (!conn->IsOpen())
    {
        lime::ConnectionRegistry::freeConnection(conn);
        lime::ReportError(EBUSY, "Failed to open. Device is busy.");
        return nullptr;
    }
    auto info = conn->GetDeviceInfo();
    if (info.deviceName ==  lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI) || info.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2))
        device = new LMS7_LimeSDR_mini(conn,obj);
    else if (info.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR_QPCIE))
        device = new LMS7_qLimeSDR(conn,obj);
    else if (info.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR_PCIE))
        device = new LMS7_LimeSDR_PCIE(conn,obj);
    else if (info.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMENET_MICRO))
        device = new LMS7_LimeNET_micro(conn,obj);
    else if (info.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR_CORE_SDR))
        device = new LMS7_CoreSDR(conn,obj);
    else if (info.deviceName != lime::GetDeviceName(lime::LMS_DEV_UNKNOWN))
        device = new LMS7_LimeSDR(conn,obj);
    else
        device = new LMS7_Generic(conn,obj);
    return device;
}

LMS7_Device::LMS7_Device(LMS7_Device *obj) : connection(nullptr), lms_chip_id(0),fpga(nullptr), limeRFE(nullptr)
{
    if (obj != nullptr)
    {
        std::swap(lms_list,obj->lms_list);
        for (auto lms : lms_list)
            lms->SetConnection(nullptr);
        this->rx_channels = obj->rx_channels;
        this->tx_channels = obj->tx_channels;
        lime::ConnectionRegistry::freeConnection(obj->connection);
        obj->connection = nullptr;
        delete obj;
    }
    else
    {
        lms_list.push_back(new lime::LMS7002M());
        tx_channels.resize(GetNumChannels());
        rx_channels.resize(GetNumChannels());
    }
}

LMS7_Device::~LMS7_Device()
{
    for (unsigned i = 0; i < lms_list.size();i++)
        delete lms_list[i];

    for (unsigned i = 0; i < mStreamers.size(); i++)
        delete mStreamers[i];

    if (fpga) delete fpga;
    lime::ConnectionRegistry::freeConnection(connection);
}

lime::IConnection* LMS7_Device::GetConnection(unsigned chan)
{
    return connection;
}

lime::FPGA* LMS7_Device::GetFPGA()
{
    return fpga;
}

lime::LMS7002M* LMS7_Device::SelectChannel(unsigned ch) const
{
    lime::LMS7002M* lms = lms_list.at(ch/2);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),(ch%2)+1);
    return lms;
}

int LMS7_Device::ConfigureGFIR(bool tx, unsigned ch, bool enabled, double bandwidth)
{
    double w,w2;
    int L;
    int div = 1;

    if (tx)
        tx_channels[ch].gfir_bw = enabled ? bandwidth : -1;
    else
        rx_channels[ch].gfir_bw = enabled ? bandwidth : -1;

    bandwidth /= 1e6;
    lime::LMS7002M* lms = SelectChannel(ch);

    if (enabled && bandwidth <= 0)
    {
        lime::warning("GFIR LPF cannot be set to the requested bandwidth");
        enabled = false;
    }

    if (enabled)
    {
        double interface_MHz;
        int ratio;

        if (tx)
        {
            ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
            interface_MHz = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx) / 1e6;
        }
        else
        {
            ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
            interface_MHz = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx) / 1e6;
        }

        if (ratio != 7)
            div = (2<<(ratio));

        w = (bandwidth/2)/(interface_MHz/div);

        L = div > 8 ? 8 : div;
        div -= 1;

        w2 = w*1.1;
        if (w2 > 0.495)
        {
            w2 = w*1.05;
            if (w2 > 0.495)
            {
                lime::warning("GFIR LPF cannot be set to the requested bandwidth");
                enabled = false; //Filter disabled
            }
        }
    }

    if (tx)
    {
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP),enabled==false);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP),enabled==false);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP),enabled==false);
    }
    else
    {
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), enabled == false);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), enabled == false);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), enabled == false);
        bool sisoDDR = lms->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        if (ch%2)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXBLML), !(enabled|sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXBLML), enabled? 3 : 0);
        }
        else
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXALML), !(enabled|sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXALML),  enabled? 3 : 0);
        }
    }
    if (!enabled)
        return 0;

    double coef[120];
    double coef2[40];
    short gfir1[120];
    short gfir2[40];

    GenerateFilter(L*15, w, w2, 1.0, 0, coef);
    GenerateFilter(L*5, w, w2, 1.0, 0, coef2);

    int sample = 0;
    for(int i=0; i<15; i++)
    {
	for(int j=0; j<8; j++)
        {
            if( (j < L) && (sample < L*15) )
            {
                gfir1[i*8+j] = (coef[sample]*32767.0);
		sample++;
            }
            else
            {
		gfir1[i*8+j] = 0;
            }
	}
    }

    sample = 0;
    for(int i=0; i<5; i++)
    {
	for(int j=0; j<8; j++)
        {
            if( (j < L) && (sample < L*5) )
            {
                gfir2[i*8+j] = (coef2[sample]*32767.0);
		sample++;
            }
            else
            {
		gfir2[i*8+j] = 0;
            }
	}
    }

    L-=1;

    if (tx)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP), L) != 0)
           return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP), div);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP), L);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP), div);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP), L);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP), div);

    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP), L) != 0)
          return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP), div);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP), L);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP), div);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), L);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), div);

    }

    if ((lms->SetGFIRCoefficients(tx, 0, gfir2, 40) != 0)
        || (lms->SetGFIRCoefficients(tx, 1, gfir2, 40) != 0)
        || (lms->SetGFIRCoefficients(tx, 2, gfir1, 120) != 0))
        return -1;

    return lms->ResetLogicregisters();
}

unsigned LMS7_Device::GetNumChannels(const bool tx) const
{
    return 2;
}

int LMS7_Device::SetRate(double f_Hz, int oversample)
{
    double nco_f=0;
    for (unsigned i = 0; i < GetNumChannels(false);i++)
    {
         if (rx_channels[i].cF_offset_nco > nco_f)
             nco_f = rx_channels[i].cF_offset_nco;
         if (tx_channels[i].cF_offset_nco > nco_f)
             nco_f = tx_channels[i].cF_offset_nco;
         tx_channels[i].sample_rate = f_Hz;
         rx_channels[i].sample_rate = f_Hz;
    }

    if (oversample == 0)
    {
        const int n = lime::cgenMax/(4*f_Hz);
        oversample = (n >= 32) ? 32 : (n >= 16) ? 16 : (n >= 8) ? 8 : (n >= 4) ? 4 : 2;
    }

    if (nco_f != 0)
    {
        int nco_over = 2+2*(nco_f-1)/f_Hz;
        if (nco_over > 32)
        {
            lime::error("Cannot achieve desired sample rate: rate too low");
            return -1;
        }
        oversample = oversample > nco_over ? oversample : nco_over;
    }

    int decim = 4;
    if (oversample <= 16)
    {
        const int decTbl[] = {0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
        decim = decTbl[oversample];
    }

    oversample = 2<<decim;

    for (unsigned i = 0; i < lms_list.size(); i++)
    {
         lime::LMS7002M* lms = lms_list[i];
        if ((lms->SetFrequencyCGEN(f_Hz*4*oversample) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decim) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), decim) != 0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
            || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), decim, decim) != 0))
            return -1;
         lms_chip_id = i;
         if (SetFPGAInterfaceFreq(decim, decim)!=0)
             return -1;
    }

    for (unsigned i = 0; i < GetNumChannels();i++)
    {
        if (rx_channels[i].cF_offset_nco != 0)
           SetNCOFreq(false, i, 0, rx_channels[i].cF_offset_nco);

        if (tx_channels[i].cF_offset_nco != 0)
           SetNCOFreq(true, i, 0, -tx_channels[i].cF_offset_nco);
        auto gfir_bw = tx_channels[i].gfir_bw;
        if (gfir_bw > 0)
            ConfigureGFIR(true, i, true, gfir_bw);
        gfir_bw = rx_channels[i].gfir_bw;
        if (gfir_bw > 0)
            ConfigureGFIR(false, i, true, gfir_bw);
    }

    return 0;
}

int LMS7_Device::SetRate(bool tx, double f_Hz, unsigned oversample)
{
    double tx_clock;
    double rx_clock;
    double cgen;

    int decimation;
    int interpolation;

    double nco_rx=0;
    double nco_tx=0;
    int min_int = 1;
    int min_dec = 1;
    bool retain_nco = false;

    lime::LMS7002M* lms = lms_list[0];

    for (unsigned i = 0; i < GetNumChannels();i++)
    {
        if (rx_channels[i].cF_offset_nco > nco_rx)
            nco_rx = rx_channels[i].cF_offset_nco;
        if (tx_channels[i].cF_offset_nco > nco_tx)
            nco_tx = tx_channels[i].cF_offset_nco;
        if (tx)
            tx_channels[i].sample_rate = f_Hz;
        else
            rx_channels[i].sample_rate = f_Hz;
    }

    if (oversample == 0)
    {
        int n = tx ? lime::cgenMax/f_Hz : lime::cgenMax/(4*f_Hz);
        oversample = (n >= 32) ? 32 : (n >= 16) ? 16 : (n >= 8) ? 8 : (n >= 4) ? 4 : 2;
    }

    if (nco_rx != 0 || nco_tx != 0)
    {
        retain_nco = true;
        min_int = 2+2*(nco_tx-1)/tx_channels[0].sample_rate;
        min_dec = 2+2*(nco_rx-1)/rx_channels[0].sample_rate;
        unsigned int nco_over = tx ? min_int : min_dec;
        if (nco_over > 32)
        {
            lime::ReportError(ERANGE, "Cannot achieve desired sample rate: rate too low");
            return -1;
        }
        oversample = oversample > nco_over ? oversample : nco_over;
    }

    int tmp = 4;
    if (oversample <= 16)
    {
        const int decTbl[] = {0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
        tmp = decTbl[oversample];
    }

    int ratio = 2<<tmp;

    if (tx)
    {
        interpolation = tmp;
        decimation = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        rx_clock = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        tx_clock = f_Hz*ratio;
    }
    else
    {
        decimation = tmp;
        interpolation = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        tx_clock = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
        rx_clock = f_Hz * ratio;
    }

    int div_index = floor(log2(tx_clock/rx_clock)+0.5);

    while (div_index < -1)
    {
        if (tx)
        {
           if ((decimation > 0) && (min_dec <= (1<<decimation)))
           {
             decimation--;
             div_index++;
           }
           else if (interpolation < 4)
           {
             interpolation++;
             div_index++;
           }
           else
           {
               div_index = -1;
               break;
           }
        }
        else
        {
           if (interpolation < 4)
           {
             interpolation++;
             div_index++;
           }
           else if ((decimation > 0) && (min_dec <= (1<<decimation)))
           {
             decimation--;
             div_index++;
           }
           else
           {
               div_index = -1;
               break;
           }
        }
    }

    while (div_index > 5)
    {
        if (tx)
        {
           if (decimation < 4)
           {
             decimation++;
             div_index--;
           }
           else if ((interpolation > 0) && (min_int <= (1<<interpolation)))
           {
             interpolation--;
             div_index--;
           }
           else
           {
             div_index = 5;
             break;
           }
        }
        else
        {
           if ((interpolation > 0) && (min_int <= (1<<interpolation)))
           {
             interpolation--;
             div_index--;
           }
           else if (decimation < 4)
           {
             decimation++;
             div_index--;
           }
           else
           {
               div_index = 5;
               break;
           }
        }
    }

    if (min_int > (2<<interpolation) || min_dec > (2<<decimation))
    {
        lime::ReportError(ERANGE, "Unable to meet NCO oversampling requirements");
        return -1;
    }

    int clk_mux;
    int clk_div;

    switch (div_index)
    {
        case -1://2:1
                clk_mux = 0;
                clk_div = 3;
                break;
        case 0://1:1
                clk_mux = 0;
                clk_div = 2;
                break;
        case 1: //1:2
                clk_mux = 0;
                clk_div = 1;
                break;
        case 2://1:4
                clk_mux = 0;
                clk_div = 0;
                break;
        case 3: //1:8
                clk_mux = 1;
                clk_div = 1;
                break;
        case 4: //1:16
                clk_mux = 1;
                clk_div = 2;
                break;
        case 5: //1:32
                clk_mux = 1;
                clk_div = 3;
                break;
    }

    if (tx)
    {
        ratio = 2<<interpolation;
        cgen = f_Hz*ratio;
    }
    else
    {
        ratio = 2<<decimation;
        cgen = f_Hz * ratio * 4;
    }

    if ((tx && clk_mux == 0)||(tx == false && clk_mux == 1))
    {
        while (cgen*(1<<clk_div)>lime::cgenMax)
        {
            if (clk_div > 0)
            {
                clk_div--;
            }
            else
            {
               lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
               return -1;
            }
        }
        cgen *= (1 << clk_div);
    }

    if (cgen > lime::cgenMax)
    {
        lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
        return -1;
    }

    for (unsigned i = 0; i < lms_list.size(); i++)
      {
        lms = lms_list[i];
        if ((lms->SetFrequencyCGEN(cgen, retain_nco) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), clk_mux) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), clk_div) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
	    || (lms->SetInterfaceFrequency(cgen, interpolation, decimation) != 0))
	  return -1;
         lms_chip_id = i;
         if (SetFPGAInterfaceFreq(interpolation, decimation)!=0)
             return -1;
      }

    for (unsigned i = 0; i < GetNumChannels();i++)
    {
        if (rx_channels[i].cF_offset_nco != 0)
           SetNCOFreq(false, i, 0, rx_channels[i].cF_offset_nco);

        if (tx_channels[i].cF_offset_nco != 0)
           SetNCOFreq(true, i, 0, -tx_channels[i].cF_offset_nco);
        auto gfir_bw = tx ? tx_channels[i].gfir_bw : rx_channels[i].gfir_bw;
        if (gfir_bw > 0)
            ConfigureGFIR(tx, i, true, gfir_bw);
    }


    return 0;
}

int LMS7_Device::SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample)
{
    if (SetRate(true, txRate, oversample)!=0)
        return -1;
    return SetRate(false, rxRate, oversample);
}

int LMS7_Device::SetFPGAInterfaceFreq(int interp, int dec, double txPhase, double rxPhase)
{
    if (!fpga)
        return 0;
    auto lms = lms_list[lms_chip_id];
    if (interp < 0)
        interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    if (dec < 0)
        dec = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));

    auto fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    if (interp != 7)
    {
        auto siso =  lms->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        fpgaTxPLL /= pow(2.0, interp + siso);
    }
    auto fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
    if (dec != 7)
    {
        auto siso =  lms->Get_SPI_Reg_bits(LMS7_LML2_SISODDR);
        fpgaRxPLL /= pow(2.0, dec + siso);
    }

    if (std::fabs(rxPhase) > 360 || std::fabs(txPhase) > 360)
    {
        if (fpga->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, lms_chip_id) != 0)
            return -1;
    }
    else if (fpga->SetInterfaceFreq(fpgaTxPLL,fpgaRxPLL, txPhase, rxPhase, lms_chip_id) != 0)
        return -1;
    return lms->ResetLogicregisters();
}

double LMS7_Device::GetRate(bool tx, unsigned chan, double *rf_rate_Hz) const
{
    double interface_Hz;
    int ratio;
    lime::LMS7002M* lms = SelectChannel(chan);

    if (tx)
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    }
    else
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
    }

    if (rf_rate_Hz)
        *rf_rate_Hz = interface_Hz;

    if (ratio != 7)
        interface_Hz /= 2*pow(2.0, ratio);

    return interface_Hz;
}

LMS7_Device::Range LMS7_Device::GetRateRange(bool /*dir*/, unsigned /*chan*/) const
{
    return Range(100e3, 61.44e6);
}

std::vector<std::string> LMS7_Device::GetPathNames(bool dir_tx, unsigned /*chan*/) const
{
    if (dir_tx)
        return {"NONE", "BAND1", "BAND2"};
    else
        return {"NONE", "LNAH", "LNAL", "LNAW", "LB1", "LB2"};
}

int LMS7_Device::SetPath(bool tx, unsigned chan, unsigned path)
{
    if (path >= GetPathNames(tx, chan).size())
        path = tx ? 1 : lime::LMS7002M::PATH_RFE_LNAL; //default settings: LNAL, band1

    lime::LMS7002M* lms = SelectChannel(chan);

    if (tx)
        return lms->SetBandTRF(path);
    return lms->SetPathRFE(lime::LMS7002M::PathRFE(path));
}

int LMS7_Device::GetPath(bool tx, unsigned chan) const
{
    lime::LMS7002M* lms = SelectChannel(chan);
    if (tx)
        return lms->GetBandTRF();
    return lms->GetPathRFE();
}

LMS7_Device::Range LMS7_Device::GetRxPathBand(unsigned path, unsigned chan) const
{
  switch (path)
  {
      case LMS_PATH_LNAH: return Range(2e9, 2.6e9);
      case LMS_PATH_LNAW: return Range(700e6, 2.6e9);
      case LMS_PATH_LNAL: return Range(700e6, 900e6);
      default: return Range();
  }
}

LMS7_Device::Range LMS7_Device::GetTxPathBand(unsigned path, unsigned chan) const
{
  switch (path)
  {
      case LMS_PATH_TX2: return Range(2e9, 2.6e9);
      case LMS_PATH_TX1: return Range(30e6, 1.9e9);
      default: return Range();
  }
}

int LMS7_Device::SetLPF(bool tx,unsigned chan, bool en, double bandwidth)
{
    lime::LMS7002M* lms = SelectChannel(chan);

    auto bw_range = GetLPFRange(tx,chan);
    std::vector<ChannelInfo>& channels = tx ? tx_channels : rx_channels;

    if (!en){
        bandwidth = bw_range.max;
    }
    else if (bandwidth < 0){
        bandwidth = channels[chan].lpf_bw;
    }
    else{
        if (bandwidth < bw_range.min){
            lime::warning("%cXLPF set to %.3f MHz (requested %0.3f MHz [out of range])", tx ? 'T':'R', bw_range.min/1e6, bandwidth/1e6);
            bandwidth = bw_range.min;
        }
        else if (bandwidth > bw_range.max){
            lime::warning("%cXLPF set to %.3f MHz (requested %0.3f MHz [out of range])", tx ? 'T':'R', bw_range.max/1e6, bandwidth/1e6);
            bandwidth = bw_range.max;
        }
        channels[chan].lpf_bw = bandwidth;
    }
    int status = 0;
    if(tx)
    {
        int gain = lms->GetTBBIAMP_dB();
        status = lms->TuneTxFilter(bandwidth);
        lms->SetTBBIAMP_dB(gain);
    }
    else
        status = lms->TuneRxFilter(bandwidth);

    if (status!=0)
        return -1;

    lime::info("%cX LPF configured",tx ? 'T' : 'R');
    return 0;
}

double LMS7_Device::GetLPFBW(bool tx,unsigned chan) const
{
    return tx ? tx_channels[chan].lpf_bw : rx_channels[chan].lpf_bw;
}

LMS7_Device::Range LMS7_Device::GetLPFRange(bool tx, unsigned chan) const
{
    return tx ? Range(5e6, 130e6) : Range(1.4001e6, 130e6);
}

int LMS7_Device::SetGFIRCoef(bool tx, unsigned chan, lms_gfir_t filt, const double* coef,unsigned count)
{
    short gfir[120];
    unsigned int L;
    int div = 1;
    int ret = 0;

    if (tx)
        tx_channels[chan].gfir_bw = -1.0;
    else
        rx_channels[chan].gfir_bw = -1.0;

    if (count > 120)
    {
        lime::ReportError(ERANGE, "Max number of coefficients for GFIR3 is 120 and for GFIR1(2) - 40");
        return -1;
    }
    else if (count > 40 && filt != LMS_GFIR3)
    {
        lime::ReportError(ERANGE, "Max number of coefficients for GFIR1(2) is 40");
        return -1;
    }
    lime::LMS7002M* lms = SelectChannel(chan);

    int ratio;

    if (tx)
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    }
    else
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    }

    if (ratio != 7)
        div = (2<<(ratio));

    if ((div > 8) || (count == 120) || (count == 40 && filt != LMS_GFIR3))
        L = 8;
    else
        L = div;

    if ((filt==LMS_GFIR3 ? 15 : 5)*L < count)
    {
        lime::warning("GFIR: disabling auto coef ordering (auto length < coef count)");
        L = 8;
    }

    double max=0;
    for (unsigned i=0; i< count; i++)
        if (fabs(coef[i])>max)
            max=fabs(coef[i]);

    if (max < 1.0)
        max = 1.0f;
    else if (max > 10.0)
        max = 32767.0f;

    unsigned sample = 0;
    for(int i=0; i< (filt==LMS_GFIR3 ? 15 : 5); i++)
    {
        for(unsigned int j=0; j<8; j++)
        {
            if( (j < L) && (sample < count) )
            {
                gfir[i*8+j] = (coef[sample]*32767.0/max);
                sample++;
            }
            else
            {
                gfir[i*8+j] = 0;
            }
        }
    }

    div -= 1;
    L = div > 7 ? 7 : div;

    if (tx)
    {
      if (filt ==LMS_GFIR1)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP), div) != 0))
              return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP), div) != 0))
              return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP), div) != 0))
              return -1;
      }
    }
    else
    {
      if (filt ==LMS_GFIR1)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP), div) != 0))
            return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP), div) != 0))
            return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), L) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), div) != 0))
            return -1;
      }
    }

    if (lms->SetGFIRCoefficients(tx, filt, gfir, filt == LMS_GFIR3 ? 120 : 40) != 0)
       return -1;
   return ret;
}

int LMS7_Device::GetGFIRCoef(bool tx, unsigned chan, lms_gfir_t filt, double* coef) const
{
    lime::LMS7002M* lms = SelectChannel(chan);
    int16_t coef16[120];

    if (lms->GetGFIRCoefficients(tx, filt, coef16, filt == LMS_GFIR3 ? 120 : 40) != 0)
        return -1;
    if (coef != NULL)
    {
        for (int i = 0; i < (filt==LMS_GFIR3 ? 120 : 40) ; i++)
        {
            coef[i] = coef16[i];
            coef[i] /= 32767.0;
        }
    }
    return (filt==LMS_GFIR3) ? 120 : 40;
}

int LMS7_Device::SetGFIR(bool tx, unsigned chan, lms_gfir_t filt, bool enabled)
{
    lime::LMS7002M* lms = SelectChannel(chan);
    if (tx)
    {
        if (filt ==LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
    }
    else
    {
        if (filt ==LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        bool sisoDDR = lms->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        if (chan%2)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXBLML), !(enabled|sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXBLML), enabled? 3 : 0);
        }
        else
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXALML), !(enabled|sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXALML),  enabled? 3 : 0);
        }
    }

   return 0;
}


int LMS7_Device::SetGain(bool dir_tx, unsigned chan, double value, const std::string &name)
{
    lime::LMS7002M* lms = SelectChannel(chan);

    if (name == "LNA")
        return lms->SetRFELNA_dB(value);
    else if (name == "LB_LNA")
        return lms->SetRFELoopbackLNA_dB(value);
    else if (name == "TIA")
        return lms->SetRFETIA_dB(value);
    else if (name == "PGA")
        return lms->SetRBBPGA_dB(value);
    else if (name == "PAD")
        return lms->SetTRFPAD_dB(value);
    else if (name == "IAMP")
        return lms->SetTBBIAMP_dB(value);
    else if (name == "LB_PAD")
        return lms->SetTRFLoopbackPAD_dB(value);

    if (dir_tx)
    {
        if (lms->SetTRFPAD_dB(value)!=0)
            return -1;
#ifdef NEW_GAIN_BEHAVIOUR
        if(value <= 0)
            return lms->Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),1);
        if(lms->GetTBBIAMP_dB() < 0.0) 
            return lms->CalibrateTxGain(0,nullptr);
#else
        value -= lms->GetTRFPAD_dB();
        if (lms->SetTBBIAMP_dB(value)!=0)
            return -1;
#endif
    }
    else
    {
#ifdef NEW_GAIN_BEHAVIOUR
        const int maxGain = 62; // gain table size
#else
        const int maxGain = 74;
#endif
        value += 12;           //pga offset
        if (value >= maxGain) //do not exceed gain table index
            value = maxGain-1;
        else if (value < 0)
            value = 0;
        unsigned lna = 0, pga = 0, tia = 0;
#ifdef NEW_GAIN_BEHAVIOUR
        //LNA table
        const unsigned lnaTbl[maxGain] = {
            0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,
            5,  5,  6,  6,  6,  7,  7,  7,  8,  9,  10, 11, 11, 11, 11, 11,
            11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 14,
            14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
        };
        //PGA table
        const unsigned pgaTbl[maxGain] = {
            0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
            1,  2,  0,  1,  2,  0,  1,  2,  0,  0,  0,  0,  1,  2,  3,  4,
            5,  6,  7,  8,  9,  10, 11, 12, 12, 12, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        };
#else
        //LNA table
        const unsigned lnaTbl[maxGain] = {
            0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,
            5,  5,  6,  6,  6,  7,  7,  7,  8,  9,  10, 11, 11, 11, 11, 11,
            11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 14,
            14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
            14, 14, 14, 14, 14, 14, 14, 14, 14, 14
        };
        //PGA table
        const unsigned pgaTbl[maxGain] = {
            0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
            1,  2,  0,  1,  2,  0,  1,  2,  0,  0,  0,  0,  1,  2,  3,  4,
            5,  6,  7,  8,  9,  10, 11, 12, 12, 12, 12, 4,  5,  6,  7,  8,
            9,  10, 11, 12, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
            22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        };
#endif
        lna = lnaTbl[int(value+0.5)];
        pga = pgaTbl[int(value+0.5)];

#ifdef NEW_GAIN_BEHAVIOUR
        if(value > 0) tia = 1;
#else
        //TIA table
        if (value > 51) tia = 2;
        else if (value > 42) tia = 1;
#endif

        int rcc_ctl_pga_rbb = (430*(pow(0.65,((double)pga/10)))-110.35)/20.4516+16; //from datasheet

        if ((lms->Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE),lna+1)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE),tia+1)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB),pga)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB),rcc_ctl_pga_rbb)!=0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetGain(bool dir_tx, unsigned chan, const std::string &name) const
{
    lime::LMS7002M* lms = SelectChannel(chan);

    if (name == "LNA")
        return lms->GetRFELNA_dB();
    else if (name == "LB_LNA")
        return lms->GetRFELoopbackLNA_dB();
    else if (name == "TIA")
        return lms->GetRFETIA_dB();
    else if (name == "PGA")
        return lms->GetRBBPGA_dB();
    else if (name == "PAD")
        return lms->GetTRFPAD_dB();
    else if (name == "IAMP")
        return lms->GetTBBIAMP_dB();
    else if (name == "LB_PAD")
        return lms->GetTRFLoopbackPAD_dB();

#ifdef NEW_GAIN_BEHAVIOUR
    if (dir_tx)
        return lms->GetTRFPAD_dB();
    else
        return lms->GetRFELNA_dB() + lms->GetRBBPGA_dB();
#else
    if (dir_tx)
        return lms->GetTRFPAD_dB() + lms->GetTBBIAMP_dB();
    else
        return lms->GetRFELNA_dB() + lms->GetRFETIA_dB() + lms->GetRBBPGA_dB();
#endif
}

LMS7_Device::Range LMS7_Device::GetGainRange(bool isTx, unsigned chan, const std::string &name) const
{
    if (name == "LNA") return Range(0.0, 30.0);
    if (name == "LB_LNA") return Range(0.0, 40.0);
    if (name == "TIA") return Range(0.0, 12.0);
    if (name == "PGA") return Range(-12.0, 19.0);
    if (name == "PAD") return Range(0.0, 52.0);
    if (name == "IAMP") return Range(-12.0, 12.0);
    if (name == "LB_PAD") return Range(-4.3, 0.0);
#ifdef NEW_GAIN_BEHAVIOUR
    if (name == "") return Range(isTx ? 0.0 : -12.0, isTx ? 52.0 : 49.0);
#else
    if (name == "") return Range(-12.0, isTx ? 64.0 : 61.0);
#endif
    return Range();
}

int LMS7_Device::SetTestSignal(bool dir_tx, unsigned chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{
    lime::LMS7002M* lms = SelectChannel(chan);

    if (dir_tx == false)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 2);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);

        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), sig == LMS_TESTSIG_DC);
    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), sig != LMS_TESTSIG_NONE) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 2);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 0);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 1);

        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), sig == LMS_TESTSIG_DC);

    }

    if (sig == LMS_TESTSIG_DC)
        return lms->LoadDC_REG_IQ(dir_tx, dc_i, dc_q);

    return 0;
}

int LMS7_Device::GetTestSignal(bool dir_tx, unsigned chan) const
{
    lime::LMS7002M* lms = SelectChannel(chan);

    if (dir_tx)
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP)) == 0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP)) != 0)
        {
            return LMS_TESTSIG_DC;
        }
        else return lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP)) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), true);
    }
    else
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_RXTSP)) == 0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP)) != 0)
        {
            return LMS_TESTSIG_DC;
        }
        else return lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP)) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), true);
    }
    return -1;
}

int LMS7_Device::SetNCOFreq(bool tx, unsigned ch, int ind, double freq)
{
    lime::LMS7002M* lms = SelectChannel(ch);

    bool enable = (ind>=0) && (freq != 0);

    if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP,!enable)!=0)
    || ( lms->Modify_SPI_Reg_bits( tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, enable)!=0))
        return -1;

    if ((ind>=0) && (lms->SetNCOFrequency(tx, ind, std::fabs(freq)) != 0))
        return -1;

    if (enable)
    {
        bool down = (freq < 0);
        if ((!tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
            down = !down;
        if ((lms->Modify_SPI_Reg_bits( tx ? LMS7_SEL_TX : LMS7_SEL_RX, ind) != 0)
        || ( lms->Modify_SPI_Reg_bits(tx ? LMS7_MODE_TX : LMS7_MODE_RX, 0) != 0)
        || ( lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP,down)!=0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetNCOFreq(bool tx, unsigned ch, int ind) const
{
    lime::LMS7002M* lms = SelectChannel(ch);
    double freq = lms->GetNCOFrequency(tx,ind,true);

    bool down = lms->Get_SPI_Reg_bits(tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP);
    if ((!tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
        down = !down;
    return down ? -freq : freq;
}

int LMS7_Device::SetNCOPhase(bool tx, unsigned ch, int ind, double phase)
{
    lime::LMS7002M* lms = SelectChannel(ch);

    bool enable = (ind>=0) && (phase != 0);

    if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP,!enable)!=0)
    || ( lms->Modify_SPI_Reg_bits( tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, enable)!=0))
        return -1;

    if ((ind>=0) && (lms->SetNCOPhaseOffset(tx, ind, phase)!=0))
        return -1;
    if (enable)
    {
        if ((lms->Modify_SPI_Reg_bits( tx ? LMS7_SEL_TX : LMS7_SEL_RX, ind) != 0)
        || ( lms->Modify_SPI_Reg_bits(tx ? LMS7_MODE_TX : LMS7_MODE_RX, 1) != 0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetNCOPhase(bool tx, unsigned ch, int ind) const
{
    lime::LMS7002M* lms = SelectChannel(ch);
    return lms->GetNCOPhaseOffset_Deg(tx, ind);
}

int LMS7_Device::Calibrate(bool dir_tx, unsigned chan, double bw, unsigned flags)
{
    lime::LMS7002M* lms = SelectChannel(chan);
    int ret;
    auto reg20 = lms->SPI_read(0x20);
    lms->SPI_write(0x20,reg20 | (20 << (chan%2)));
    if (dir_tx)
        ret = lms->CalibrateTx(bw, flags & 1);
    else
        ret = lms->CalibrateRx(bw, flags & 1);
    lms->SPI_write(0x20,reg20);
    return ret;
}

int LMS7_Device::SetFrequency(bool isTx, unsigned chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[chan / 2];

    int chA = chan&(~1);
    int chB = chan|1;

    std::vector<ChannelInfo>& channels = isTx ? tx_channels : rx_channels;

    auto setTDD = [=](double center)->int
    {
        std::vector<ChannelInfo>& other = isTx ? rx_channels : tx_channels;
        bool tdd =  fabs(other[chA].freq+other[chA].cF_offset_nco-center) > 0.1 ? false : true;
        lms->EnableSXTDD(tdd);
        if (isTx || (!tdd))
            if (lms->SetFrequencySX(isTx, center) != 0)
                return -1;
        return 0;
    };

    channels[chan].freq = f_Hz;
    if (channels[chA].freq > 0 && channels[chB].freq > 0)
    {
        double delta = fabs(channels[chA].freq - channels[chB].freq);
        if (delta > 0.1)
        {
            double rate = GetRate(isTx,chan);
            if ((delta <= rate*31) && (delta+rate <= 160e6))
            {
                double center = (channels[chA].freq+channels[chB].freq)/2;
                if (center < 30e6)
                    center = 30e6;
                channels[chA].cF_offset_nco = center-channels[chA].freq;
                channels[chB].cF_offset_nco = center-channels[chB].freq;
                if (setTDD(center) != 0)
                    return -1;
                if (SetRate(isTx,rate,2)!=0)
                    return -1;
                return 0;
            }
        }
    }

    if (f_Hz < 30e6)
    {
        if (setTDD(30e6) != 0)
            return -1;
        channels[chan].cF_offset_nco = 30e6-f_Hz;
        double rf_rate;
        double rate = GetRate(isTx, chan, &rf_rate);
        if (channels[chan].cF_offset_nco+rate/2.0 >= rf_rate/2.0)
        {
            if (SetRate(isTx, rate, 2)!=0)
                return -1;
            return 0;
        }
        else
            return SetNCOFreq(isTx, chan, 0, channels[chan].cF_offset_nco * (isTx ? -1.0 : 1.0));
    }

    if (channels[chan].cF_offset_nco != 0)
        SetNCOFreq(isTx, chan, -1, 0.0);
    channels[chA].cF_offset_nco = 0;
    channels[chB].cF_offset_nco = 0;
    if (setTDD(f_Hz) != 0)
        return -1;
    return 0;
}

double LMS7_Device::GetFrequency(bool tx, unsigned chan) const
{
   lime::LMS7002M* lms = lms_list[chan / 2];
   double offset = tx ? tx_channels[chan].cF_offset_nco : rx_channels[chan].cF_offset_nco;

   if (!tx)
   {
        lms->Modify_SPI_Reg_bits(LMS7_MAC, 1);
        if (lms->Get_SPI_Reg_bits(LMS7_PD_VCO)==1)
            tx = true; //assume that Tx PLL used for TX and RX
   }
   return lms->GetFrequencySX(tx) - offset;
}

LMS7_Device::Range LMS7_Device::GetFrequencyRange(bool tx) const
{
    return Range(100e3, 3.8e9);
}

int LMS7_Device::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x0525},
        {0x0089, 0x1078}, {0x008B, 0x218C}, {0x008C, 0x267B}, {0x00A6, 0x000F},
        {0x00A9, 0x8000}, {0x00AC, 0x2000}, {0x0108, 0x218C}, {0x0109, 0x57C1},
        {0x010A, 0x154C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010D, 0x011A},
        {0x010E, 0x0000}, {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000},
        {0x0112, 0x000C}, {0x0113, 0x03C2}, {0x0114, 0x01F0}, {0x0115, 0x000D},
        {0x0118, 0x418C}, {0x0119, 0x5292}, {0x011A, 0x3001}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0984}, {0x0120, 0xE6C0}, {0x0121, 0x3638},
        {0x0122, 0x0514}, {0x0123, 0x200F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB}
    };

    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        if (lms->ResetChip() != 0)
            return -1;
       
        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        for (auto i : initVals)
            lms->SPI_write(i.adr, i.val, true);
        
        if(lms->CalibrateTxGain(0,nullptr) != 0)
            return -1;

        EnableChannel(true, 2*i, false);
        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
        for (auto i : initVals)
            if (i.adr >= 0x100)
                lms->SPI_write(i.adr, i.val, true);

        if(lms->CalibrateTxGain(0,nullptr) != 0)
            return -1;

        EnableChannel(false, 2*i+1, false);
        EnableChannel(true, 2*i+1, false);

        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

        if(SetFrequency(true,2*i,GetFrequency(true,2*i))!=0)
            return -1;
        if(SetFrequency(false,2*i,GetFrequency(false,2*i))!=0)
            return -1;
    }

    if (SetRate(10e6,2)!=0)
        return -1;
    return 0;
}

int LMS7_Device::Reset()
{
    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        if (lms->ResetChip() != 0)
            return -1;
    }
    return LMS_SUCCESS;
}

int LMS7_Device::EnableChannel(bool dir_tx, unsigned chan, bool enabled)
{
    lime::LMS7002M* lms = SelectChannel(chan);

    lms->EnableChannel(dir_tx, enabled);
    if (!enabled)
    {
        if (dir_tx)
        {
            tx_channels[chan].freq = -1.0;
            tx_channels[chan].cF_offset_nco = 0.0;
        }
        else
        {
            rx_channels[chan].freq = -1.0;
            rx_channels[chan].cF_offset_nco = 0.0;
        }
    }
    return LMS_SUCCESS;
}

std::vector<std::string> LMS7_Device::GetProgramModes() const
{
    return {program_mode::fpgaRAM, program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3RAM, program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_Device::Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const
{
    if (connection == nullptr)
        return lime::ReportError(EINVAL, "Device not connected");

    if (mode == program_mode::autoUpdate)
        return this->connection->ProgramUpdate(true, true, callback);
    if (mode == program_mode::fx3Flash)
        return this->connection->ProgramWrite(data, len, 2, 1, callback);
    else if (mode == program_mode::fx3RAM)
        return this->connection->ProgramWrite(data, len, 1, 1, callback);
    else if (mode == program_mode::fx3Reset)
        return this->connection->ProgramWrite(nullptr, 0, 0, 1, callback);
    else if (mode == program_mode::fpgaFlash)
         return this->connection->ProgramWrite(data, len, 1, 2, callback);
    else if (mode == program_mode::fpgaRAM)
         return this->connection->ProgramWrite(data, len, 0, 2, callback);
    else if (mode == program_mode::fpgaReset)
         return this->connection->ProgramWrite(data, len, 2, 2, callback);
    else if (mode == program_mode::mcuReset) {
        auto lms = lms_list.at(lms_chip_id);
        lms->SPI_write(0x0002, 0x0000);
        return lms->SPI_write(0x0002, 0x0003);
    } else if (mode == program_mode::mcuRAM || mode == program_mode::mcuEEPROM){
        lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
        lime::IConnection::MCU_PROG_MODE prog_mode;
        uint8_t bin[lime::MCU_BD::cMaxFWSize];
        memcpy(bin,data,len>sizeof(bin) ? sizeof(bin) : len);

        if (mode == program_mode::mcuRAM)
            prog_mode = lime::IConnection::MCU_PROG_MODE::SRAM;
        else
            prog_mode = lime::IConnection::MCU_PROG_MODE::EEPROM_AND_SRAM;
        mcu->callback = callback;
        mcu->Program_MCU(bin,prog_mode);
        mcu->callback = nullptr;
        return 0;
    }

    lime::ReportError(ENOTSUP, "Unsupported programming target");
    return -1;
}

double LMS7_Device::GetClockFreq(unsigned clk_id, int channel) const
{
    int lmsInd = channel == -1 ? lms_chip_id : channel/2;
    switch (clk_id)
    {
    case LMS_CLOCK_REF:
        return lms_list.at(lmsInd)->GetReferenceClk_SX(lime::LMS7002M::Rx);
    case LMS_CLOCK_SXR:
        return lms_list.at(lmsInd)->GetFrequencySX(false);

    case LMS_CLOCK_SXT:
        return lms_list.at(lmsInd)->GetFrequencySX(true);

    case LMS_CLOCK_CGEN:
        return lms_list.at(lmsInd)->GetFrequencyCGEN();

    case LMS_CLOCK_RXTSP:
        return lms_list.at(lmsInd)->GetReferenceClk_TSP(false);

    case LMS_CLOCK_TXTSP:
        return lms_list.at(lmsInd)->GetReferenceClk_TSP(true);
    case LMS_CLOCK_EXTREF:
        lime::ReportError(ENOTSUP, "Reading external reference clock is not supported");
        return -1;
    default:
        lime::ReportError(EINVAL, "Invalid clock ID.");
        return -1;
    }
}

int LMS7_Device::SetClockFreq(unsigned clk_id, double freq, int channel)
{
    lms_chip_id = channel == -1 ? lms_chip_id : channel/2;
    lime::LMS7002M* lms = lms_list[lms_chip_id];
    switch (clk_id)
    {
    case LMS_CLOCK_REF:
        if (freq <= 0)
        {
            lime::ReportError(EINVAL, "Invalid frequency value.");
            return -1;
        }
        lms->SetReferenceClk_SX(lime::LMS7002M::Tx, freq);
        return 0;
    case LMS_CLOCK_SXR:
        if (freq <= 0)
            return lms->TuneVCO(lime::LMS7002M::VCO_SXR);
        if (channel != -1)
        {
            rx_channels[channel].cF_offset_nco = 0.0;
            rx_channels[channel].freq = freq;
        }
        return lms->SetFrequencySX(false, freq);
    case LMS_CLOCK_SXT:
        if (freq <= 0)
            return lms->TuneVCO(lime::LMS7002M::VCO_SXT);
        if (channel != -1)
        {
            tx_channels[channel].cF_offset_nco = 0.0;
            tx_channels[channel].freq = freq;
        }
        return lms->SetFrequencySX(true, freq);
    case LMS_CLOCK_CGEN:
    {
        int ret =0;
        lms->Modify_SPI_Reg_bits(LMS7param(MAC),1);
        if (freq <= 0)
        {
            ret = lms->TuneVCO(lime::LMS7002M::VCO_CGEN);
        }
        else
        {
            ret = lms->SetInterfaceFrequency(freq, lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP)), lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP)));
        }
        if (ret != 0)
            return -1;
        return SetFPGAInterfaceFreq();
    }
    case LMS_CLOCK_RXTSP:
        lime::ReportError(ENOTSUP, "Setting TSP clocks is not supported.");
        return -1;
    case LMS_CLOCK_TXTSP:
        lime::ReportError(ENOTSUP, "Setting TSP clocks is not supported.");
        return -1;
    case LMS_CLOCK_EXTREF:
        {
            if (freq <= 0)
            {
                lime::info("Disabling external reference clock");
                double val;
                uint8_t id = 0;
                connection->CustomParameterRead(&id, &val, 1, nullptr);
                connection->CustomParameterWrite(&id, &val, 1, "");
                return 0;
            }

            lime::ADF4002 module;
            module.SetDefaults();
            double fvco = lms->GetReferenceClk_SX(lime::LMS7002M::Rx);
            int dummy;
            module.SetFrefFvco(freq/1e6, fvco/1e6, dummy, dummy);
            unsigned char data[12];
            module.GetConfig(data);

            std::vector<uint32_t> dataWr;
            for(int i=0; i<12; i+=3)
                dataWr.push_back((uint32_t)data[i] << 16 | (uint32_t)data[i+1] << 8 | data[i+2]);
            return connection->TransactSPI(0x30, dataWr.data(), nullptr, 4);
        }
    default:
        lime::ReportError(EINVAL, "Invalid clock ID.");
        return -1;
    }
}

lms_dev_info_t* LMS7_Device::GetInfo()
{
    memset(&devInfo,0,sizeof(lms_dev_info_t));
    if (connection)
    {
        auto info = connection->GetDeviceInfo();
        strncpy(devInfo.deviceName,info.deviceName.c_str(),sizeof(devInfo.deviceName)-1);
        strncpy(devInfo.expansionName,info.expansionName.c_str(),sizeof(devInfo.expansionName)-1);
        strncpy(devInfo.firmwareVersion,info.firmwareVersion.c_str(),sizeof(devInfo.firmwareVersion)-1);
        strncpy(devInfo.hardwareVersion,info.hardwareVersion.c_str(),sizeof(devInfo.hardwareVersion)-1);
        strncpy(devInfo.protocolVersion,info.protocolVersion.c_str(),sizeof(devInfo.protocolVersion)-1);
        strncpy(devInfo.gatewareVersion, (info.gatewareVersion+"."+ info.gatewareRevision).c_str(), sizeof(devInfo.gatewareVersion) - 1);
        strncpy(devInfo.gatewareTargetBoard,info.gatewareTargetBoard.c_str(),sizeof(devInfo.gatewareTargetBoard)-1);
        info.boardSerialNumber = info.boardSerialNumber;
        devInfo.boardSerialNumber = info.boardSerialNumber;
    }
    return &devInfo;
}

int LMS7_Device::Synchronize(bool toChip)
{
    int ret=0;
    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        if (toChip)
        {
            if (lms->UploadAll()==0)
            {
                int tmp = lms_chip_id;
                lms_chip_id = i;
                lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
                ret = SetFPGAInterfaceFreq(-1, -1, -1000, -1000);
                lms_chip_id = tmp;
            }
        }
        else
            ret = lms->DownloadAll();
        if (ret != 0)
            break;
    }
    return ret;
}

int LMS7_Device::SetLogCallback(void(*func)(const char* cstr, const unsigned int type))
{
    for (unsigned i = 0; i < lms_list.size(); i++)
        lms_list[i]->SetLogCallback(func);
    return 0;
}

int LMS7_Device::EnableCache(bool enable)
{
    for (unsigned i = 0; i < lms_list.size(); i++)
        lms_list[i]->EnableValuesCache(enable);
    if (fpga)
        fpga->EnableValuesCache(enable);
    return 0;
}

double LMS7_Device::GetChipTemperature(int ind) const
{
    return lms_list.at(ind == -1 ? lms_chip_id : ind)->GetTemperature();
}

int LMS7_Device::LoadConfig(const char *filename, int ind)
{
    lime::LMS7002M* lms = lms_list.at(ind == -1 ? lms_chip_id : ind);
    if (lms->LoadConfig(filename)==0)
    {
        //tune PLLs as saved VCO settings may not work
        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        if (!lms->Get_SPI_Reg_bits(LMS7param(PD_VCO)))
            lms->SetFrequencySX(false, lms->GetFrequencySX(false));
        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
        if (!lms->Get_SPI_Reg_bits(LMS7param(PD_VCO)))
            lms->SetFrequencySX(true, lms->GetFrequencySX(true));
        if (!lms->Get_SPI_Reg_bits(LMS7param(PD_VCO_CGEN)))
        {
            lms->TuneVCO(lime::LMS7002M::VCO_CGEN);
            lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
            return SetFPGAInterfaceFreq(-1, -1, -1000, -1000);
        }
        return 0;
    }
    return -1;
}

int LMS7_Device::SaveConfig(const char *filename, int ind) const
{
    return lms_list.at(ind == -1 ? lms_chip_id : ind)->SaveConfig(filename);
}

int LMS7_Device::ReadLMSReg(uint16_t address, int ind) const
{
    return lms_list.at(ind == -1 ? lms_chip_id : ind)->SPI_read(address & 0xFFFF);
}

int LMS7_Device::WriteLMSReg(uint16_t address, uint16_t val, int ind) const
{
    return lms_list.at(ind == -1 ? lms_chip_id : ind)->SPI_write(address & 0xFFFF, val);
}

int LMS7_Device:: ReadFPGAReg(uint16_t address) const
{
    return fpga ? fpga->ReadRegister(address): 0;
}
int LMS7_Device::WriteFPGAReg(uint16_t address, uint16_t val) const
{
    return fpga ? fpga->WriteRegister(address, val): 0;
}

uint16_t LMS7_Device::ReadParam(const struct LMS7Parameter& param, int chan, bool fromChip) const
{
    int lmsChip;
    if (chan >= 0)
    {
        lmsChip = chan/2;
        if (param.address >= 0x100)
            lms_list.at(lmsChip)->Modify_SPI_Reg_bits(LMS7param(MAC),(chan%2) + 1);
    }
    else lmsChip = lms_chip_id;
    return lms_list.at(lmsChip)->Get_SPI_Reg_bits(param, fromChip);
}

int LMS7_Device::ReadParam(const std::string& name, int chan, bool fromChip) const
{
    const LMS7Parameter* param = lime::LMS7002M::GetParam(name);
    if (!param)
        return -1;

    int lmsChip;
    if (chan >= 0)
    {
        lmsChip = chan/2;
        if (param->address >= 0x100)
            lms_list.at(lmsChip)->Modify_SPI_Reg_bits(LMS7param(MAC),(chan%2) + 1);
    }
    else lmsChip = lms_chip_id;

    return lms_list.at(lmsChip)->Get_SPI_Reg_bits(param->address, param->msb, param->lsb, fromChip);
}

int LMS7_Device::WriteParam(const struct LMS7Parameter& param, uint16_t val, int chan)
{
    int lmsChip;
    if (chan >= 0)
    {
        lmsChip = chan/2;
        if (param.address >= 0x100)
            lms_list.at(lmsChip)->Modify_SPI_Reg_bits(LMS7param(MAC),(chan%2) + 1);
    }
    else lmsChip = lms_chip_id;

    return lms_list.at(lmsChip)->Modify_SPI_Reg_bits(param, val);
}

int LMS7_Device::WriteParam(const std::string& name, uint16_t val, int chan)
{
    const LMS7Parameter* param = lime::LMS7002M::GetParam(name);

    if (!param)
        return -1;

    int lmsChip;
    if (chan >= 0)
    {
        lmsChip = chan/2;
        if (param->address >= 0x100)
            lms_list.at(lmsChip)->Modify_SPI_Reg_bits(LMS7param(MAC),(chan%2) + 1);
    }
    else lmsChip = lms_chip_id;

    return lms_list.at(lmsChip)->Modify_SPI_Reg_bits(param->address, param->msb, param->lsb, val);
}

int LMS7_Device::SetActiveChip(unsigned ind)
{
    if (ind >= lms_list.size())
    {
        lime::ReportError("Invalid chip ID");
        return -1;
    }
    this->lms_chip_id = ind;
    return 0;
}

lime::LMS7002M* LMS7_Device::GetLMS(int index) const
{
    return lms_list.at( index < 0 ? lms_chip_id : index);
}

int LMS7_Device::UploadWFM(const void **samples, uint8_t chCount, int sample_count, lime::StreamConfig::StreamDataFormat fmt) const
{
    if (!fpga)
        return lime::ReportError("Device not connected");

    return fpga->UploadWFM(samples, chCount%2 ? 1 : 2, sample_count, fmt, (chCount-1)/2);
}

lime::StreamChannel* LMS7_Device::SetupStream(const lime::StreamConfig &config)
{
    if (config.channelID >= GetNumChannels())
        return nullptr;
    if (!connection)
        return nullptr;
    return mStreamers[config.channelID/2]->SetupStream(config);
}

int LMS7_Device::DestroyStream(lime::StreamChannel* streamID)
{
    reinterpret_cast<lime::StreamChannel*>(streamID)->Close();
    return 0;
}

uint64_t LMS7_Device::GetHardwareTimestamp(void) const
{
    return mStreamers[0]->GetHardwareTimestamp();
}

void LMS7_Device::SetHardwareTimestamp(const uint64_t now)
{
    mStreamers[0]->SetHardwareTimestamp(now);
}

int LMS7_Device::MCU_AGCStart(uint32_t wantedRSSI)
{
    lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x0006, 0, 0, 0);

    uint8_t mcuID = mcu->ReadMCUProgramID();
    lime::debug("Current MCU firmware: %i, expected %i", mcuID, MCU_ID_CALIBRATIONS_SINGLE_IMAGE);
    if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        lime::debug("Uploading MCU AGC firmware");
        int status = mcu->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, lime::IConnection::MCU_PROG_MODE::SRAM);
        lime::info("MCU AGC firmware uploaded");
        if(status != 0)
            return status;
    }

    long refClk = lms_list.at(lms_chip_id)->GetReferenceClk_SX(false);
    mcu->SetParameter(MCU_BD::MCU_REF_CLK, refClk);

    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x002D, 15, 0, wantedRSSI >> 2);
    mcu->RunProcedure(MCU_FUNCTION_AGC);
    return 0;
}

int LMS7_Device::MCU_AGCStop()
{
    lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
    mcu->RunProcedure(0);
    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x0006, 0, 0, 0);
    return 0;
}

RFE_Device* LMS7_Device::GetLimeRFE() const
{
    return limeRFE;
}
void LMS7_Device::SetLimeRFE(RFE_Device* dev)
{
    limeRFE = dev;
}

}//namespace lime
