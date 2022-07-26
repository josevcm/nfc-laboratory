#include "lms7002m_calibrations.h"
#include "LMS7002M_parameters_compact.h"
#include "spi.h"
#include "lms7002m_controls.h"
#include <math.h>
#include "mcu_defines.h"

#ifndef __cplusplus
#include "lms7002_regx51.h" //MCU timer sfr
#endif

#define ENABLE_EXTERNAL_LOOPBACK 1

#ifdef __cplusplus
#include <cstdlib>
#define VERBOSE 1
#define DRAW_GNU_PLOTS

#include <thread>
#include <vector>
#include <chrono>
#include <stdio.h>
#include <sstream>

#ifdef DRAW_GNU_PLOTS
#define PUSH_GMEASUREMENT_VALUES(value, rssi) gMeasurements.push_back({value, rssi})
#else
#define PUSH_GMEASUREMENT_VALUES(value, rssi)
#endif
#include <gnuPlotPipe.h>
GNUPlotPipe saturationPlot;
GNUPlotPipe IQImbalancePlot;
GNUPlotPipe txDCPlot;

typedef std::vector< std::pair<float, float> > MeasurementsVector;
MeasurementsVector gMeasurements;

void SortMeasurements(MeasurementsVector &vec)
{
    for(size_t i=0; i<vec.size(); ++i)
        for(size_t j=i; j<vec.size(); ++j)
            if(vec[i].first > vec[j].first)
            {
                auto temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            }
}

void DrawMeasurement(GNUPlotPipe &gp, const MeasurementsVector& vec)
{
    for(auto i : vec)
        gp.writef("%f %f\n", i.first, i.second);
    gp.write("e\n");
}

///APPROXIMATE conversion
float ChipRSSI_2_dBFS(uint32_t rssi)
{
    uint32_t maxRSSI = 0x15FF4;
    if(rssi == 0)
        rssi = 1;
    return 20*log10((float)(rssi)/maxRSSI);
}

int16_t toSigned(int16_t val, uint8_t msblsb)
{
    val <<= 15-((msblsb >> 4) & 0xF);
    val >>= 15-((msblsb >> 4) & 0xF);
    return val;
}
#else
#define VERBOSE 0
#define PUSH_GMEASUREMENT_VALUES(value, rssi)
#endif // __cplusplus

float bandwidthRF = 5e6; //Calibration bandwidth
uint16_t RSSIDelayCounter = 1; // MCU timer delay between RSSI measurements
#define calibrationSXOffset_Hz 1e6
#define offsetNCO 0.1e6
#define calibUserBwDivider 5

// external loopback selection
// [2] tx band, when calibrating Rx, 0-band1, 1-band2
// [1:0] SEL_PATH_RFE, when calibrating Tx
uint8_t extLoopbackPair = 0;

int16_t clamp(int16_t value, int16_t minBound, int16_t maxBound)
{
    if(value < minBound)
        return minBound;
    if(value > maxBound)
        return maxBound;
    return value;
}

static void FlipRisingEdge(const uint16_t addr, const uint8_t bits)
{
    Modify_SPI_Reg_bits(addr, bits, 0);
    Modify_SPI_Reg_bits(addr, bits, 1);
}

static bool IsPLLTuned()
{
    if( Get_SPI_Reg_bits(0x0123, MSB_LSB(13, 12)) == 2 )
        return true;
    return TuneVCO(true) == MCU_NO_ERROR;
}

void LoadDC_REG_TX_IQ()
{
    SPI_write(0x020C, 0x7FFF);
    FlipRisingEdge(TSGDCLDI_TXTSP);
    SPI_write(0x020C, 0x8000);
    FlipRisingEdge(TSGDCLDQ_TXTSP);
}


extern float_type RefClk;
void UpdateRSSIDelay()
{
    const uint16_t sampleCount = (2 << 7) << Get_SPI_Reg_bits(AGC_AVG_RXTSP);
    uint8_t decimation = Get_SPI_Reg_bits(HBD_OVR_RXTSP);
    if(decimation < 6)
        decimation = (2 << decimation);
    else
        decimation = 1; //bypass
    {
    float waitTime = sampleCount/((GetReferenceClk_TSP_MHz(false)/2) / decimation );
    RSSIDelayCounter = (0xFFFF) - (uint16_t)(waitTime*RefClk/12);
    }
}

uint32_t GetRSSI()
{
    uint32_t rssi;
#ifdef __cplusplus
    int waitTime = 1000000.0*(0xFFFF - RSSIDelayCounter)*12/RefClk;
    std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
#else
    TR0 = 0; //stop timer 0
    TH0 = (RSSIDelayCounter >> 8);
    TL0 = (RSSIDelayCounter & 0xFF);
    TF0 = 0; // clear overflow
    TR0 = 1; //start timer 0
    while( !TF0 ); // wait for timer overflow
#endif
    FlipRisingEdge(CAPTURE);
    rssi = SPI_read(0x040F);
    return (rssi << 2 | (SPI_read(0x040E) & 0x3));
}

static void SetRxGFIR3Coefficients()
{
    //FIR coefficients symmetrical, storing only one half
    ROM const int16_t firCoefs[] =
    {
        8,4,0,-6,-11,-16,-20,-22,-22,-20,-14,-5,6,20,34,46,
        56,61,58,48,29,3,-29,-63,-96,-123,-140,-142,-128,-94,-44,20,
        93,167,232,280,302,291,244,159,41,-102,-258,-409,-539,-628,-658,-614,
        -486,-269,34,413,852,1328,1814,2280,2697,3038,3277,3401,
    };
    uint8_t index = 0;
    for (; index < sizeof(firCoefs)/sizeof(int16_t); ++index)
        SPI_write(0x0500 + index + 24 * (index / 40), firCoefs[index]);
    for (; index < sizeof(firCoefs)/sizeof(int16_t)*2; ++index)
        SPI_write(0x0500 + index + 24 * (index / 40), firCoefs[119-index]);
}

int CheckSaturationTxRx(bool extLoopback)
{
    const uint16_t saturationLevel = 0x05000; //-3dBFS
    uint8_t g_pga;
    uint8_t g_rfe;
    uint16_t rssi;
#ifdef DRAW_GNU_PLOTS
    int index = 0;
    GNUPlotPipe &gp = saturationPlot;
    gp.write("set yrange [:0]\n");
    gp.write("set title 'Rx gains search'\n");
    gp.write("set key right bottom\n");
    gp.write("set xlabel 'measurement index'\n");
    gp.write("set ylabel 'RSSI dbFS'\n");
    gp.write("set grid xtics ytics\n");
    gp.writef("plot\
'-' u 1:2 with lines title 'target Level',\
'-' u 1:2 with lines title '%s',\
'-' u 1:2 with lines title 'PGA'\n", (extLoopback ? "LNA":"RXLOOPB"));
    gp.writef("%i %f\n%i %f\ne\n", 0, ChipRSSI_2_dBFS(saturationLevel),
                              20, ChipRSSI_2_dBFS(saturationLevel));
    gMeasurements.clear();
#endif
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO + (bandwidthRF / calibUserBwDivider) * 2, 0);

    g_pga = (uint8_t)Get_SPI_Reg_bits(G_PGA_RBB);
#if ENABLE_EXTERNAL_LOOPBACK
    if(extLoopback)
    {
        g_rfe = 0;
        Modify_SPI_Reg_bits(G_LNA_RFE, g_rfe);
    }
    else
#endif
        g_rfe = (uint8_t)Get_SPI_Reg_bits(G_RXLOOPB_RFE);
    rssi = GetRSSI();
    PUSH_GMEASUREMENT_VALUES(index, ChipRSSI_2_dBFS(rssi));

#if VERBOSE
    printf("Receiver saturation search, target level: %i (%2.3f dBFS)\n", saturationLevel, ChipRSSI_2_dBFS(saturationLevel));
    printf("initial  PGA: %2i, %s: %2i, %3.2f dbFS\n", g_pga, (extLoopback ? "LNA":"RXLOOPB"), g_rfe, ChipRSSI_2_dBFS(rssi));
#endif
    while(rssi < saturationLevel)
    {
        if(g_rfe < 15)
            ++g_rfe;
        else
            break;
#if ENABLE_EXTERNAL_LOOPBACK
        if(extLoopback)
            Modify_SPI_Reg_bits(G_LNA_RFE, g_rfe);
        else
#endif
            Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rfe);
        rssi = GetRSSI();
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
    }
#ifdef DRAW_GNU_PLOTS
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
#endif // DRAW_GNU_PLOTS

    PUSH_GMEASUREMENT_VALUES(index, ChipRSSI_2_dBFS(rssi));
    {
    uint16_t rssi_prev = rssi;
    while(g_pga < 25 && g_rfe == 15 && rssi < saturationLevel)
    {
        if(g_pga < 25)
            ++g_pga;
        else
            break;
        Modify_SPI_Reg_bits(G_PGA_RBB, g_pga);
        rssi = GetRSSI();
        if((float)rssi/rssi_prev < 1.05) // pga should give ~1dB change
            break;
        rssi_prev = rssi;
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
    }
    }
#ifdef DRAW_GNU_PLOTS
    DrawMeasurement(gp, gMeasurements);
    gp.flush();
#endif // DRAW_GNU_PLOTS
#if VERBOSE
    printf("adjusted PGA: %2i, %s: %2i, %3.2f dbFS\n", Get_SPI_Reg_bits(G_PGA_RBB), (extLoopback ? "LNA":"RXLOOPB"), g_rfe, ChipRSSI_2_dBFS(rssi));
#endif
    if( rssi < 0xB21 ) // ~(-30 dbFS)
    {
#if VERBOSE
        printf("Signal strength (%3.1f dBFS) very low, loopback not working?\n", ChipRSSI_2_dBFS(rssi));
#endif // VERBOSE
        return MCU_LOOPBACK_SIGNAL_WEAK;
    }
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
    return MCU_NO_ERROR;
}

typedef struct
{
    LMS7Parameter param;
    int16_t result;
    int16_t minValue;
    int16_t maxValue;
} BinSearchParam;

void BinarySearch(BinSearchParam bdata* args)
{
    uint16_t rssiLeft = ~0;
    uint16_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;
    const uint16_t addr = args->param.address;
    const uint8_t msblsb = args->param.msblsb;

    Modify_SPI_Reg_bits(addr, msblsb, right);
    rssiRight = GetRSSI();
    PUSH_GMEASUREMENT_VALUES(right, ChipRSSI_2_dBFS(rssiRight));
    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            Modify_SPI_Reg_bits(addr, msblsb, right);
            rssiRight = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(right, ChipRSSI_2_dBFS(rssiRight));
        }
        else
        {
            Modify_SPI_Reg_bits(addr, msblsb, left);
            rssiLeft = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(left, ChipRSSI_2_dBFS(rssiLeft));
        }
        if(step <= 0)
            break;
        if(rssiLeft < rssiRight)
            right -= step;
        else
            left += step;
    }
    args->result = rssiLeft < rssiRight ? left : right;
    Modify_SPI_Reg_bits(addr, msblsb, args->result);
}

int16_t ReadAnalogDC(const uint16_t addr)
{
    const uint16_t mask = addr < 0x05C7 ? 0x03FF : 0x003F;
    uint16_t value;
    int16_t result;
    SPI_write(addr, 0);
    SPI_write(addr, 0x4000);
    value = SPI_read(addr);
    SPI_write(addr, value & ~0xC000);
    result = (value & mask);
    if(value & (mask+1))
        result *= -1;
    return result;
}

static void WriteAnalogDC(const uint16_t addr, int16_t value)
{
    const uint16_t mask = addr < 0x05C7 ? 0x03FF : 0x003F;
    int16_t regValue = 0;
    if(value < 0)
    {
        regValue |= (mask+1);
        regValue |= (abs(value+mask) & mask);
    }
    else
        regValue |= (abs(value+mask+1) & mask);
    SPI_write(addr, regValue);
    SPI_write(addr, regValue | 0x8000);
}

static void TxDcBinarySearch(BinSearchParam* args)
{
    uint16_t rssiLeft = ~0;
    uint16_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;

    WriteAnalogDC(args->param.address, right);
    rssiRight = GetRSSI();
    PUSH_GMEASUREMENT_VALUES(right, ChipRSSI_2_dBFS(rssiRight));

    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            WriteAnalogDC(args->param.address, right);
            rssiRight = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(right, ChipRSSI_2_dBFS(rssiRight));
        }
        else
        {
            WriteAnalogDC(args->param.address, left);
            rssiLeft = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(left, ChipRSSI_2_dBFS(rssiLeft));
        }
        if(step == 0)
            break;
        if(rssiLeft < rssiRight)
            right -= step;
        else
            left += step;
    }

    args->result = rssiLeft < rssiRight ? left : right;
    WriteAnalogDC(args->param.address, args->result);
}

void AdjustAutoDC(const uint16_t address, bool tx)
{
    uint8_t i;
    uint16_t rssi;
    uint16_t minRSSI;
    int16_t minValue;
    int16_t initVal;
    int8_t valChange;
    uint16_t range = tx ? 1023 : 63;

    minValue = initVal = ReadAnalogDC(address);
    minRSSI = rssi = GetRSSI();
    WriteAnalogDC(address, clamp(initVal+1, -range, range));
    valChange = GetRSSI() < rssi ? 1 : -1;

    for(i = 8; i; --i)
    {
        initVal = clamp(initVal+valChange, -range, range);
        WriteAnalogDC(address, initVal);
        rssi = GetRSSI();
        if(rssi < minRSSI)
        {
            minRSSI = rssi;
            minValue = initVal;
        }
    }
    WriteAnalogDC(address, minValue);
}

void CalibrateRxDCAuto()
{
    uint16_t dcRegAddr = 0x5C7;
    const uint8_t ch = Get_SPI_Reg_bits(MAC);
    Modify_SPI_Reg_bits(EN_G_TRF, 0);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);

    //auto calibration
    Modify_SPI_Reg_bits(DCMODE, 1);
    if(ch == 1)
    {
        Modify_SPI_Reg_bits(PD_DCDAC_RXA, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_RXA, 0);
        SPI_write(0x05C2, 0xFF30);
    }
    else
    {
        Modify_SPI_Reg_bits(PD_DCDAC_RXB, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_RXB, 0);
        SPI_write(0x05C2, 0xFFC0);
        dcRegAddr += 2;
    }

    {
        while(SPI_read(0x05C1) & 0xF000);
    }
#if VERBOSE
    {
        int16_t dci = ReadAnalogDC(dcRegAddr);
        int16_t dcq = ReadAnalogDC(dcRegAddr+1);
        uint32_t rssi = GetRSSI();
        printf("Rx DC auto   I: %3i, Q: %3i, %3.1f dBFS\n", dci, dcq, ChipRSSI_2_dBFS(rssi));
    }
#endif // VERBOSE

    //manual adjustments
    Modify_SPI_Reg_bits(GCORRQ_RXTSP, 0);
    AdjustAutoDC(dcRegAddr, false);
    Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);
    AdjustAutoDC(dcRegAddr+1, false);

#if VERBOSE
    {
        int16_t dci = ReadAnalogDC(dcRegAddr);
        int16_t dcq = ReadAnalogDC(dcRegAddr+1);
        uint32_t rssi = GetRSSI();
        printf("Rx DC manual I: %3i, Q: %3i, %3.1f dBFS\n", dci, dcq, ChipRSSI_2_dBFS(rssi));
    }
#endif

    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0); // DC_BYP 0
#if VERBOSE
    printf("RxTSP DC corrector enabled %3.1f dBFS\n", ChipRSSI_2_dBFS(GetRSSI()));
#endif
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
}

void CalibrateTxDCAuto()
{
#ifdef DRAW_GNU_PLOTS
    GNUPlotPipe &gp = txDCPlot;
    std::vector<MeasurementsVector> data;
    gMeasurements.clear();
#endif // DRAW_GNU_PLOTS
    BinSearchParam iparams;
    BinSearchParam qparams;
    const uint8_t ch = Get_SPI_Reg_bits(MAC);
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);

    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 1);
    //auto calibration
    Modify_SPI_Reg_bits(DCMODE, 1);
    //Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, 0);
    //Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, 0);
    iparams.param.msblsb = 10<<4 | 0;
    qparams.param.msblsb = 10<<4 | 0;
    if(ch == 1)
    {
        iparams.param.address = 0x5C3;// DC_TXAI;
        qparams.param.address = 0x5C4;// DC_TXAQ;
        Modify_SPI_Reg_bits(PD_DCDAC_TXA, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_TXA, 0);
        //SPI_write(0x05C2, 0x0F03);
    }
    else
    {
        iparams.param.address = 0x5C5;// DC_TXBI;
        qparams.param.address = 0x5C6;// DC_TXBQ;
        Modify_SPI_Reg_bits(PD_DCDAC_TXB, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_TXB, 0);
        //SPI_write(0x05C2, 0x0F0C);
    }
    WriteAnalogDC(iparams.param.address, 0);
    WriteAnalogDC(qparams.param.address, 0);

    //wait until finished
    //while(SPI_read(0x05C1) & 0x0F00);

    {
    ROM const int16_t offset[3] = {1023, 128, 8};
    uint8_t i;
    iparams.result = 0; //ReadAnalogDC(iparams.param.address);
    qparams.result = 0; //ReadAnalogDC(qparams.param.address);
    for(i=0; i<3; ++i)
    {
        iparams.minValue = clamp(iparams.result-offset[i], -1024, 1023);
        iparams.maxValue = clamp(iparams.result+offset[i], -1024, 1023);
        qparams.minValue = clamp(qparams.result-offset[i], -1024, 1023);
        qparams.maxValue = clamp(qparams.result+offset[i], -1024, 1023);

        TxDcBinarySearch(&iparams);
#ifdef DRAW_GNU_PLOTS
        data.push_back(gMeasurements);
        gMeasurements.clear();
#endif // DRAW_GNU_PLOTS
        TxDcBinarySearch(&qparams);
#ifdef DRAW_GNU_PLOTS
        data.push_back(gMeasurements);
        gMeasurements.clear();
#endif // DRAW_GNU_PLOTS
#if VERBOSE
    {
        int16_t dci = ReadAnalogDC(iparams.param.address);
        int16_t dcq = ReadAnalogDC(qparams.param.address);
        uint32_t rssi = GetRSSI();
        printf("#%i Tx DC manual I: %4i, Q: %4i, %3.1f dBFS\n", i, dci, dcq, ChipRSSI_2_dBFS(rssi));
    }
#endif // VERBOSE
    }
    }

#ifdef DRAW_GNU_PLOTS
    gp.writef("set title 'TxDC search'\n");
    gp.write("set xlabel 'offset'\n");
    gp.write("set ylabel 'RSSI dBFS'\n");
    gp.write("set grid ytics xtics\n");
    std::stringstream ss;
    for(unsigned i=0; i<data.size()/2; ++i)
    {
        if(i>0)
            ss << ", ";
        ss << "'-' w l t '#" << i << " I', ";
        ss << "'-' w l t '#" << i << " Q'";
    }
    gp.writef("plot %s\n", ss.str().c_str());
    for(auto i : data)
    {
        SortMeasurements(i);
        DrawMeasurement(gp, i);
    }
#endif // DRAW_GNU_PLOTS

    //Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, 2047);
    //Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, 2047);
}

void CalibrateIQImbalance(bool tx)
{
#if defined(VERBOSE) || defined(DRAW_GNU_PLOTS)
    const char *dirName = tx ? "Tx" : "Rx";
#endif
#ifdef DRAW_GNU_PLOTS
    GNUPlotPipe &gp = IQImbalancePlot;
    std::vector<MeasurementsVector> data;
    gMeasurements.clear();
    gp.writef("set title '%s IQ imbalance'\n", dirName);
    gp.write("set xlabel 'parameter value'\n");
    gp.write("set ylabel 'RSSI dBFS'\n");
    gp.write("set grid ytics xtics\n");
    gp.write("plot\
'-' w l t '#0 phase',\
'-' w l t '#1 gain',\
'-' w l t '#2 phase'\
\n");
#endif // DRAW_GNU_PLOTS
    uint16_t gcorriAddress;
    uint16_t gcorrqAddress;
    BinSearchParam argsPhase;
    BinSearchParam argsGain;
    argsGain.param.msblsb = MSB_LSB(10,0);
    argsPhase.param.msblsb = MSB_LSB(11,0);
    if(tx)
    {
        gcorrqAddress = 0x0201;
        gcorriAddress = 0x0202;
        argsPhase.param.address = 0x0203;
    }
    else
    {
        gcorrqAddress = 0x0401;
        gcorriAddress = 0x0402;
        argsPhase.param.address = 0x0403;
    }

    argsPhase.maxValue = 128;
    argsPhase.minValue = -128;
    BinarySearch(&argsPhase);
#if VERBOSE
    printf("#0 %s IQCORR: %i, %3.1f dBFS\n", dirName, argsPhase.result, ChipRSSI_2_dBFS(GetRSSI()));
#endif // VERBOSE
#ifdef DRAW_GNU_PLOTS
    SortMeasurements(gMeasurements);
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
#endif

    //coarse gain
    {
        uint16_t rssiIgain;
        uint16_t rssiQgain;
        SPI_write(gcorriAddress, 2047 - 64);
        SPI_write(gcorrqAddress, 2047);
        rssiIgain = GetRSSI();
        SPI_write(gcorriAddress, 2047);
        SPI_write(gcorrqAddress, 2047 - 64);
        rssiQgain = GetRSSI();

        if(rssiIgain < rssiQgain)
            argsGain.param.address = gcorriAddress;
        else
            argsGain.param.address = gcorrqAddress;
        SPI_write(gcorrqAddress, 2047);
    }
    argsGain.maxValue = 2047;
    argsGain.minValue = 2047-512;
    BinarySearch(&argsGain);
#if VERBOSE
    const char* chName = (argsGain.param.address == gcorriAddress ? "I" : "Q");
    printf("#1 %s GAIN_%s: %i, %3.1f dBFS\n", dirName, chName, argsGain.result, ChipRSSI_2_dBFS(GetRSSI()));
#endif // VERBOSE
#ifdef DRAW_GNU_PLOTS
    SortMeasurements(gMeasurements);
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
#endif

    argsPhase.maxValue = argsPhase.result+16;
    argsPhase.minValue = argsPhase.result-16;
    BinarySearch(&argsPhase);
#if VERBOSE
    printf("#2 %s IQCORR: %i, %3.1f dBFS\n", dirName, argsPhase.result, ChipRSSI_2_dBFS(GetRSSI()));
#endif // VERBOSE
#ifdef DRAW_GNU_PLOTS
    SortMeasurements(gMeasurements);
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
#endif
    SPI_write(argsGain.param.address, argsGain.result);
    Modify_SPI_Reg_bits(argsPhase.param.address, argsPhase.param.msblsb, argsPhase.result);
}

uint8_t SetupCGEN()
{
    uint8_t cgenMultiplier;
    uint8_t gfir3n;
    cgenMultiplier = clamp((GetFrequencyCGEN() / 46.08e6) + 0.5, 2, 13);
    gfir3n = 4 * cgenMultiplier;
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 1)
        gfir3n /= pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));

    { //gfir3n = pow2((uint8_t)log2(gfir3n))-1
        uint8_t power;
        for(power = 0x3F; power; power >>= 1)
        {
            if(gfir3n >= power)
                break;
        }
        Modify_SPI_Reg_bits(GFIR3_N_RXTSP, power);
    }
    //CGEN VCO is powered up in SetFrequencyCGEN/Tune
    return SetFrequencyCGEN(46.08e6 * cgenMultiplier);
}

uint8_t CalibrateTxSetup(bool extLoopback)
{
    uint8_t status;
    const uint16_t x0020val = SPI_read(0x0020); //remember used channel
    /*BeginBatch("TxSetup");
    //rfe
    //reset RFE to defaults
    SetDefaults(SECTION_RFE);
    Modify_SPI_Reg_bits(G_RXLOOPB_RFE, 7);
    Modify_SPI_Reg_bits(0x010C, 4 << 4 | 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(CCOMP_TIA_RFE, 4);
    Modify_SPI_Reg_bits(CFB_TIA_RFE, 50);

    //RBB
    //reset RBB to defaults
    SetDefaults(SECTION_RBB);
    Modify_SPI_Reg_bits(PD_LPFH_RBB, 0);
    Modify_SPI_Reg_bits(PD_LPFL_RBB, 1);
    Modify_SPI_Reg_bits(G_PGA_RBB, 0);
    Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 1);
    Modify_SPI_Reg_bits(ICT_PGA_OUT_RBB, 12);
    Modify_SPI_Reg_bits(ICT_PGA_IN_RBB, 12);

    //TXTSP
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(GC_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(PH_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
    Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);
    Modify_SPI_Reg_bits(CMIX_SC_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP_R3, 0);

    //RXTSP
    SetDefaults(SECTION_RxTSP);
    //SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 0);
    Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4); //Decimation HBD ratio
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);



    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 0x1);
    Modify_SPI_Reg_bits(GFIR3_L_RXTSP, 7);


    //AFE
    Modify_SPI_Reg_bits(PD_RX_AFE1, 0);
    Modify_SPI_Reg_bits(PD_RX_AFE2, 0);

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2 << 4 | 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CDS
    Modify_SPI_Reg_bits(CDS_TXATSP, 3);
    Modify_SPI_Reg_bits(CDS_TXBTSP, 3);

    //TRF
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);

    //BIAS
    {
        uint16_t backup = Get_SPI_Reg_bits(RP_CALIB_BIAS);
        SetDefaults(SECTION_BIAS);
        Modify_SPI_Reg_bits(RP_CALIB_BIAS, backup);
    }

    EndBatch();*/
    if((x0020val & 0x3) == 1)
        Modify_SPI_Reg_bits(PD_RX_AFE1, 0);
    else
        Modify_SPI_Reg_bits(PD_RX_AFE2, 0);
    {
        ROM const uint16_t TxSetupAddr[] = {0x0084, 0x0085,0x00AE,0x0101,0x0200,0x0201,0x0202,0x0208};
        ROM const uint16_t TxSetupData[] = {0x0400, 0x0001,0xF000,0x0001,0x000C,0x07FF,0x07FF,0x0000};
        ROM const uint16_t TxSetupMask[] = {0xF8FF, 0x0007,0xF000,0x1801,0x000C,0x07FF,0x07FF,0xF10B};
        ROM const uint16_t TxSetupWrOnlyAddr[] = {0x010C,0x010D,0x010E,0x010F,0x0110,0x0111,0x0112,0x0113,0x0114,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x0400,0x0401,0x0402,0x0403,0x0407,0x040A,0x040C,0x0440, 0x0441, 0x0442, 0x0443 ,0x0409,0x0408,0x0406,0x0405,0x0404,0x0081};
        ROM const uint16_t TxSetupWrOnlyData[] = {0x88E5,0x009E,0x2040,0x30C6,0x0994,0x0083,0x4032,0x03DF,0x008D,0x0005,0x8180,0x280C,0x218C,0x3180,0x2E02,0x0081,0x07FF,0x07FF,0x4000,0x0700,0x1001,0x2098}; // rest of values will be written as zeros
        ROM const RegisterBatch batch = {
            TxSetupAddr, TxSetupData, TxSetupMask, sizeof(TxSetupAddr)/sizeof(uint16_t),
            TxSetupWrOnlyAddr, TxSetupWrOnlyData, sizeof(TxSetupWrOnlyAddr)/sizeof(uint16_t), sizeof(TxSetupWrOnlyData)/sizeof(uint16_t)};
        WriteMaskedRegs(&batch);
    }
    SetRxGFIR3Coefficients();
    status = SetupCGEN();
    if(status != MCU_NO_ERROR)
        return status;
    //SXR
    Modify_SPI_Reg_bits(MAC, 1); //switch to ch. A
    SetDefaultsSX();
    Modify_SPI_Reg_bits(ICT_VCO, 255);
    {
        const float_type SXRfreq = GetFrequencySX(LMS7002M_Tx) - bandwidthRF/ calibUserBwDivider - calibrationSXOffset_Hz;
        //SX VCO is powered up in SetFrequencySX/Tune
        status = SetFrequencySX(LMS7002M_Rx, SXRfreq);
        if(status != MCU_NO_ERROR)
        {
            SPI_write(0x0020, x0020val); //restore used channel
            return status;
        }
    }

    //SXT{
    Modify_SPI_Reg_bits(MAC, 2); //switch to ch. B
    Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
    //check if Tx is tuned
    if( !IsPLLTuned() )
    {
        SPI_write(0x0020, x0020val); //restore used channel
        return MCU_SXT_TUNE_FAILED;
    }

    SPI_write(0x0020, x0020val); //restore used channel

    LoadDC_REG_TX_IQ();
    SetNCOFrequency(LMS7002M_Tx, bandwidthRF/ calibUserBwDivider, 0);
    {
        const uint8_t sel_band1_2_trf = (uint8_t)Get_SPI_Reg_bits(0x0103, MSB_LSB(11, 10));
#if ENABLE_EXTERNAL_LOOPBACK
        if(extLoopback)
        {
            uint8_t lnaPath;
            Modify_SPI_Reg_bits(PD_LNA_RFE, 0);
            if(sel_band1_2_trf == 1 || sel_band1_2_trf == 2)
            {
                //activate selected lna path for external loopback
                lnaPath = extLoopbackPair & 0x3;
                Modify_SPI_Reg_bits(SEL_PATH_RFE, lnaPath);
                Modify_SPI_Reg_bits(0x010D, MSB_LSB(2, 1), ~(lnaPath-1)); //EN_INSHSW_*_RFE

                //check if correct tx band for external loop
                if(extLoopbackPair >> 2 != !(sel_band1_2_trf-1))
                    return MCU_INVALID_TX_BAND;
            }
            else
            {
#if VERBOSE
                printf("Tx Calibration: external calibration is not supported on selected Tx Band");
#endif
                return MCU_INVALID_TX_BAND;
            }
        }
        else
#endif
        {
            if(sel_band1_2_trf != 0x1 && sel_band1_2_trf != 0x2) //BAND1
            {
                //printf("Tx Calibration: band not selected");
                return MCU_INVALID_TX_BAND;
            }
            Modify_SPI_Reg_bits(SEL_PATH_RFE, sel_band1_2_trf+1);
            //Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, 0);
            //Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 1);
            Modify_SPI_Reg_bits(0x010C, MSB_LSB(6, 5), sel_band1_2_trf ^ 0x3);
            //Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, 0);
            //Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 1);
            Modify_SPI_Reg_bits(0x010D, MSB_LSB(4, 3), sel_band1_2_trf ^ 0x3);
        }
    }
    //if calibrating ch. B enable buffers
    EnableMIMOBuffersIfNecessary();
    EnableChannelPowerControls();
    return MCU_NO_ERROR;
}

uint8_t CalibrateTx(bool extLoopback)
{
    const uint16_t x0020val = SPI_read(0x0020);
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
#if VERBOSE

    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(SEL_BAND1_TRF);
    printf("Tx ch.%s , BW: %g MHz, RF output: %s, Gain: %i, loopb: %s\n",
           (x0020val & 3) == 0x1 ? "A" : "B",
           bandwidthRF/1e6,
           sel_band1_trf==1 ? "BAND1" : "BAND2",
           Get_SPI_Reg_bits(CG_IAMP_TBB),
           extLoopback ? "external" : "internal");
#endif
    uint8_t status;
#if !ENABLE_EXTERNAL_LOOPBACK
    if(extLoopback)
        return MCU_PROCEDURE_DISABLED;
#endif
    SaveChipState(0);
    status = CalibrateTxSetup(extLoopback);
    if(status != MCU_NO_ERROR)
        goto TxCalibrationEnd; //go to ending stage to restore registers
    UpdateRSSIDelay();
    CalibrateRxDCAuto();
    status = CheckSaturationTxRx(extLoopback);
    if(status != MCU_NO_ERROR)
        goto TxCalibrationEnd;
    CalibrateRxDCAuto();

    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO + (bandwidthRF/ calibUserBwDivider), 0);
    CalibrateTxDCAuto();
    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Tx);
TxCalibrationEnd:
    {
        //analog dc is not overwritten by chip state restore
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_TXTSP);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_TXTSP);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_TXTSP);
        SaveChipState(1);
        SPI_write(0x0020, x0020val);
        if(status != MCU_NO_ERROR)
        {
#if VERBOSE
            printf("Tx calibration failed");
#endif
            return status;
        }
        Modify_SPI_Reg_bits(GCORRI_TXTSP, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_TXTSP, phaseOffset);
#if VERBOSE
        int16_t dcI = ReadAnalogDC((x0020val & 1) ? 0x5C3 : 0x5C5);
        int16_t dcQ = ReadAnalogDC((x0020val & 1) ? 0x5C4 : 0x5C6);
        printf("Tx | DC   | GAIN | PHASE\n");
        printf("---+------+------+------\n");
        printf("I: | %4i | %4i | %i\n", dcI, gcorri, toSigned(phaseOffset, MSB_LSB(11, 0)));
        printf("Q: | %4i | %4i |\n", dcQ, gcorrq);
#endif
    }

    Modify_SPI_Reg_bits(DCMODE, 1);
    if((x0020val & 1) == 1)
        Modify_SPI_Reg_bits(PD_DCDAC_TXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_TXB, 0);
    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, 1<<4 | 0, 0); //GC_BYP PH_BYP
    //LoadDC_REG_TX_IQ(); //not necessary, just for testing convenience
#if VERBOSE
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

uint8_t CalibrateRxSetup(bool extLoopback)
{
    uint8_t status;
    const uint16_t x0020val = SPI_read(0x0020);
    //rfe
    {
        ROM const uint16_t RxSetupAddr[] = {0x0084, 0x0085,0x00AE,0x010C,0x010D,0x0113,0x0115,0x0119};
        ROM const uint16_t RxSetupData[] = {0x0400, 0x0001,0xF000,0x0000,0x0046,0x000C,0x0000,0x0000};
        ROM const uint16_t RxSetupMask[] = {0xF8FF, 0x0007,0xF000,0x001A,0x0046,0x003C,0xC000,0x8000};
        ROM const uint16_t RxSetupWrOnlyAddr[] = {0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x0200,0x0201,0x0202,0x0208,0x0240,0x0400,0x0401,0x0402,0x0403,0x0407,0x040A,0x040C,0x0440,0x05C0,0x05CB,0x0203,0x0204,0x0205,0x0206,0x0207,0x0241,0x0404,0x0405,0x0406,0x0408,0x0409,0x0441,0x05C1,0x05C2,0x05C3,0x05C4,0x05C5,0x05C6,0x05C7,0x05C8,0x05C9,0x05CA,0x05CC, 0x0081};
        ROM const uint16_t RxSetupWrOnlyData[] = {0x3408,0x6001,0x3180,0x0A12,0x0088,0x0007,0x318C,0x318C,0x0426,0x61C1,0x104C,0x008D,0x07FF,0x07FF,0x2070,0x0020,0x0081,0x07FF,0x07FF,0x4000,0x0700,0x1000,0x2098,0x0020,0x00FF,0x2020};
        ROM const RegisterBatch batch = {
            RxSetupAddr, RxSetupData, RxSetupMask, sizeof(RxSetupAddr)/sizeof(uint16_t),
            RxSetupWrOnlyAddr, RxSetupWrOnlyData, sizeof(RxSetupWrOnlyAddr)/sizeof(uint16_t), sizeof(RxSetupWrOnlyData)/sizeof(uint16_t)};
        WriteMaskedRegs(&batch);
    }
    /*
    BeginBatch("RxSetup.txt");
    Modify_SPI_Reg_bits(EN_DCOFF_RXFE_RFE, 1);
    Modify_SPI_Reg_bits(G_RXLOOPB_RFE, 3);
    Modify_SPI_Reg_bits(0x010C, 4 << 4 | 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(0x010C, 1 << 4 | 1, 0); //PD_TIA 0
    if(extLoopback)
        Modify_SPI_Reg_bits(0x010C, 7 << 4 | 7, 0); //PD_LNA 0

    //RBB
    Modify_SPI_Reg_bits(0x0115, MSB_LSB(15, 14), 0); //Loopback switches disable
    Modify_SPI_Reg_bits(0x0119, MSB_LSB(15, 15), 0); //OSW_PGA 0

    //TRF
    //reset TRF to defaults
    SetDefaults(SECTION_TRF);
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);
    Modify_SPI_Reg_bits(EN_G_TRF, 0);
    if(extLoopback)
    {
        if(Get_SPI_Reg_bits(SEL_PATH_RFE) == 1)
        {
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);
        }
        Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, 15);
    }

    //TBB
    //reset TBB to defaults
    SetDefaults(SECTION_TBB);
    Modify_SPI_Reg_bits(CG_IAMP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_FRP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_GG_FRP_TBB, 6);

    //XBUF
    Modify_SPI_Reg_bits(0x0085, MSB_LSB(2, 0), 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //TXTSP
    SetDefaults(SECTION_TxTSP);
    //SetDefaults(SECTION_TxNCO);
    Modify_SPI_Reg_bits(TSGFCW_TXTSP, 1);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 0x1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, MSB_LSB(6, 4), 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_SC_TXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);

    //RXTSP
    SetDefaults(SECTION_RxTSP);
    //SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(0x040C, MSB_LSB(5, 3), 0x3); //GFIR2_BYP, GFIR1_BYP
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4);

    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(CAPSEL, 0);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_GAIN_RXTSP, 0);
    Modify_SPI_Reg_bits(GFIR3_L_RXTSP, 7);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);

    //CDS
    Modify_SPI_Reg_bits(CDS_TXATSP, 3);
    Modify_SPI_Reg_bits(CDS_TXBTSP, 3);

    //RSSI_DC_CALIBRATION
    SetDefaults(SECTION_RSSI_DC_CALIBRATION);
    EndBatch();
    //BIAS
    {
        uint16_t rp_calib_bias = Get_SPI_Reg_bits(0x0084, MSB_LSB(10, 6));
        SetDefaults(SECTION_BIAS);
        Modify_SPI_Reg_bits(0x0084, MSB_LSB(10, 6), rp_calib_bias);
    }

    /*if(!extLoopback)
    {
        Modify_SPI_Reg_bits(ICT_IAMP_FRP_TBB, 1);
        Modify_SPI_Reg_bits(ICT_IAMP_GG_FRP_TBB, 6);
    }*/

    //AFE
    if((x0020val & 0x3) == 1)
        Modify_SPI_Reg_bits(PD_TX_AFE1, 0);
    else
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);

#if ENABLE_EXTERNAL_LOOPBACK
    if(extLoopback) // external looback
    {
        const uint8_t band1_band2 = 2-((extLoopbackPair >> 2) & 1);
        Modify_SPI_Reg_bits(0x0103, MSB_LSB(11, 10), band1_band2);
        if(Get_SPI_Reg_bits(SEL_PATH_RFE) != (extLoopbackPair&0x3))
            return MCU_INVALID_RX_PATH;
    }
    else //chip internal loopbacks
#endif
    {
        switch(Get_SPI_Reg_bits(SEL_PATH_RFE))
        {
        case 2: //LNA_L
            //Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);
            //Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
            Modify_SPI_Reg_bits(0x0103, MSB_LSB(11, 10), 1);
            break;
        case 3: //LNA_W
        case 1: //LNA_H
            //Modify_SPI_Reg_bits(SEL_BAND2_TRF, 0);
            //Modify_SPI_Reg_bits(SEL_BAND1_TRF, 1);
            Modify_SPI_Reg_bits(0x0103, MSB_LSB(11, 10), 2);
            break;
        default:
            return MCU_INVALID_RX_PATH;
        }
    }

    Modify_SPI_Reg_bits(MAC, 2); //Get freq already changes/restores ch

    if(Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == 0) //isTDD
    {
        //in TDD do nothing
        Modify_SPI_Reg_bits(MAC, 1);
        SetDefaultsSX();
        Modify_SPI_Reg_bits(ICT_VCO, 255);
        status = SetFrequencySX(LMS7002M_Rx, GetFrequencySX(LMS7002M_Tx) - bandwidthRF/ calibUserBwDivider - 9e6);
    }
    else
    {
        //SXR
        float_type SXRfreqHz;
        Modify_SPI_Reg_bits(MAC, 1);
        //check if Rx is tuned
        if(!IsPLLTuned())
            return MCU_SXR_TUNE_FAILED;
        SXRfreqHz = GetFrequencySX(LMS7002M_Rx);

        //SXT
        Modify_SPI_Reg_bits(MAC, 2);
        SetDefaultsSX();
        Modify_SPI_Reg_bits(ICT_VCO, 255);
        status = SetFrequencySX(LMS7002M_Tx, SXRfreqHz + bandwidthRF/ calibUserBwDivider + 9e6);
    }
    SPI_write(0x0020, x0020val);
    if(status != MCU_NO_ERROR)
        return status;

    LoadDC_REG_TX_IQ();

    //CGEN
    status = SetupCGEN();
    if(status != MCU_NO_ERROR)
        return status;
    SetRxGFIR3Coefficients();
    SetNCOFrequency(LMS7002M_Tx, 9e6, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidthRF/calibUserBwDivider - offsetNCO, 0);
    //modifications when calibrating channel B
    EnableMIMOBuffersIfNecessary();
    EnableChannelPowerControls();
    return MCU_NO_ERROR;
}

uint8_t CheckSaturationRx(const float_type bandwidth_Hz, bool extLoopback)
{
    ROM const uint16_t target_rssi = 0x07000; //0x0B000 = -3 dBFS
    uint16_t rssi;
    uint8_t cg_iamp = (uint8_t)Get_SPI_Reg_bits(CG_IAMP_TBB);
#ifdef DRAW_GNU_PLOTS
    int index = 0;
    GNUPlotPipe &gp = saturationPlot;
    gp.write("set yrange [:0]\n");
    gp.write("set ylabel 'RSSI dbFS'\n");
    gp.write("set ylabel 'measurement index'\n");
    gp.write("set title 'Rx saturation check'\n");
    gp.write("set key right bottom\n");
    gp.write("set grid ytics xtics\n");
    gp.write("plot ");
    gp.writef(
        "'-' title '%s' with lines\
, '-' title 'CG IAMP' with lines\
, '-' title 'target Level' with lines\n", extLoopback?"LOSS MAIN TXPAD":"G RXLOOPB RFE");
#endif
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidth_Hz / calibUserBwDivider - offsetNCO, 0);

#if ENABLE_EXTERNAL_LOOPBACK
    if(extLoopback)
    {
        int8_t g_lossmain = 15;
        Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, g_lossmain);
        rssi = GetRSSI();
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
#if VERBOSE
        printf("Initial gains:\tLOSS_MAIN_TXPAD: %2i, CG_IAMP: %2i | %2.3f dbFS\n", g_lossmain, cg_iamp, ChipRSSI_2_dBFS(rssi));
#endif
        while (rssi < target_rssi)
        {
            g_lossmain -= 1;
            if(g_lossmain < 0)
                break;
            Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, g_lossmain);
            rssi = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
        }
    }
    else
#endif
    {
        uint8_t g_rxloopb_rfe = 2;
        Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
        rssi = GetRSSI();
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
#if VERBOSE
        printf("Initial gains:\tG_RXLOOPB: %2i, CG_IAMP: %2i | %2.3f dbFS\n", g_rxloopb_rfe, cg_iamp, ChipRSSI_2_dBFS(rssi));
#endif
        while (rssi < target_rssi)
        {
            g_rxloopb_rfe += 2;
            if(g_rxloopb_rfe > 15)
                break;
            Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
            rssi = GetRSSI();
            PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
        }
    }
#ifdef DRAW_GNU_PLOTS
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
#endif // DRAW_GNU_PLOTS

    PUSH_GMEASUREMENT_VALUES(index, ChipRSSI_2_dBFS(rssi));
    while(rssi < 0x01000)
    {
        cg_iamp += 2;
        if(cg_iamp > 63-6)
            break;
        Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
        rssi = GetRSSI();
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
    }

    while(rssi < target_rssi)
    {
        cg_iamp += 1;
        if(cg_iamp > 62)
            break;
        Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
        rssi = GetRSSI();
        PUSH_GMEASUREMENT_VALUES(++index, ChipRSSI_2_dBFS(rssi));
    }
#if VERBOSE
    if(extLoopback)
        printf("Initial gains:\tLOSS_MAIN_TXPAD: %2i, CG_IAMP: %2i | %2.3f dbFS\n", Get_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF), Get_SPI_Reg_bits(CG_IAMP_TBB), ChipRSSI_2_dBFS(rssi));
    else
        printf("Adjusted gains: G_RXLOOPB: %2i, CG_IAMP: %2i | %2.3f dbFS\n", Get_SPI_Reg_bits(G_RXLOOPB_RFE), Get_SPI_Reg_bits(CG_IAMP_TBB), ChipRSSI_2_dBFS(rssi));
#endif
#ifdef DRAW_GNU_PLOTS
    DrawMeasurement(gp, gMeasurements);
    gMeasurements.clear();
    gp.writef("%i %f\n%i %f\ne\n", 0, ChipRSSI_2_dBFS(target_rssi), index, ChipRSSI_2_dBFS(target_rssi));
    gp.flush();
#endif
    if( rssi < 0xB21 ) // ~(-30 dbFS)
    {
#if VERBOSE
        printf("Signal strength (%3.1f dBFS) very low, loopback not working?\n", ChipRSSI_2_dBFS(rssi));
#endif // VERBOSE
        return MCU_LOOPBACK_SIGNAL_WEAK;
    }
    return MCU_NO_ERROR;
}

uint8_t CalibrateRx(bool extLoopback, bool dcOnly)
{
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
    uint8_t status;
    const uint16_t x0020val = SPI_read(0x0020); //remember used channel

#if !ENABLE_EXTERNAL_LOOPBACK
    if(extLoopback)
        return MCU_PROCEDURE_DISABLED;
#endif
#if VERBOSE
    double rxFreq = GetFrequencySX(LMS7002M_Rx);
    const char* lnaName;
    switch(Get_SPI_Reg_bits(SEL_PATH_RFE))
    {
    case 0:
        lnaName = "none";
        break;
    case 1:
        lnaName = "LNAH";
        break;
    case 2:
        lnaName = "LNAW";
        break;
    case 3:
        lnaName = "LNAL";
        break;
    default:
        lnaName = "none";
        break;
    }
    printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
           (x0020val & 0x3) == 1 ? "A" : "B", rxFreq/1e6,
           bandwidthRF/1e6, lnaName,
           Get_SPI_Reg_bits(G_PGA_RBB),
           Get_SPI_Reg_bits(G_LNA_RFE),
           Get_SPI_Reg_bits(G_TIA_RFE));
    printf("Rx calibration started\n");
#endif
    SaveChipState(0);
    status = CalibrateRxSetup(extLoopback);
    if(status != 0)
        goto RxCalibrationEndStage;
    UpdateRSSIDelay();
    CalibrateRxDCAuto();
    if(dcOnly)
        goto RxCalibrationEndStage;
    if(!extLoopback)
    {
        if ((uint8_t)Get_SPI_Reg_bits(SEL_PATH_RFE) == 2)
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 0);
        }
        else
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, 0);
        }
    }

    Modify_SPI_Reg_bits(MAC, 2);
    if (Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == false)
    {
        Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
        //TDD MODE
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(PD_VCO, 0);
    }
    SPI_write(0x0020, x0020val);
    status = CheckSaturationRx(bandwidthRF, extLoopback);
    if(status != MCU_NO_ERROR)
        goto RxCalibrationEndStage;
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidthRF/calibUserBwDivider + offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Rx);
RxCalibrationEndStage:
    {
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_RXTSP);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_RXTSP);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_RXTSP);
        SaveChipState(1);
        SPI_write(0x0020, x0020val);
        if (status != MCU_NO_ERROR)
        {
#if VERBOSE
            printf("Rx calibration failed");
#endif
            return status;
        }
        // dc corrector values not overwritten by chip state restore
        if(!dcOnly)
        {
            Modify_SPI_Reg_bits(GCORRI_RXTSP, gcorri);
            Modify_SPI_Reg_bits(GCORRQ_RXTSP, gcorrq);
            Modify_SPI_Reg_bits(IQCORR_RXTSP, phaseOffset);
        }
#if VERBOSE
        int16_t dcI = ReadAnalogDC((x0020val & 1) ? 0x5C7 : 0x5C8);
        int16_t dcQ = ReadAnalogDC((x0020val & 1) ? 0x5C9 : 0x5CA);
        int16_t phaseSigned = toSigned(phaseOffset, MSB_LSB(11, 0));
        printf("Tx | DC   | GAIN | PHASE\n");
        printf("---+------+------+------\n");
        printf("I: | %4i | %4i | %i\n", dcI, gcorri, phaseSigned);
        printf("Q: | %4i | %4i |\n", dcQ, gcorrq);
#endif
    }
    Modify_SPI_Reg_bits(DCMODE, 1);
    if(x0020val & 0x1)
        Modify_SPI_Reg_bits(PD_DCDAC_RXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_RXB, 0);
    Modify_SPI_Reg_bits(0x040C, MSB_LSB(2, 0), 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Modify_SPI_Reg_bits(0x040C, MSB_LSB(8, 8), 0); //DCLOOP_STOP
    //Log("Rx calibration finished", LOG_INFO);
#if VERBOSE
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return MCU_NO_ERROR;
}
