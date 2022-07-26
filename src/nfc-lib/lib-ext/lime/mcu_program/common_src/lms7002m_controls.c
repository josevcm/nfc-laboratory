#include "lms7002m_controls.h"
#include "spi.h"
#include "LMS7002M_parameters_compact.h"
#include "math.h"
#include "typedefs.h"
#include "mcu_defines.h"

#ifdef __cplusplus
#include <cmath>
#include <cstdlib>
using namespace std;
#include <chrono>
#include <thread>
#else
#include "lms7002_regx51.h" //MCU timer sfr
uint16_t gComparatorDelayCounter = 0xFF00; // ~100us @ ref 30.72MHz
#endif

#define VERBOSE 0

//TODO add functions to modify reference clock
float_type RefClk = 30.72e6; //board reference clock

uint16_t pow2(const uint8_t power)
{
    return 1 << power;
}

xdata uint16_t x0020state;
ROM const uint16_t chipStateAddr[] = {
    0x0021, 0x002F, //LimeLight
    0x0081, 0x0082, //EN_DIR Configuration + AFE
    SECTION_BIAS,
    SECTION_XBUF,
    SECTION_CGEN,
    SECTION_LDO,
    SECTION_BIST,
    SECTION_CDS,
    SECTION_TRF,
    SECTION_TBB,
    SECTION_RFE,
    SECTION_RBB,
    SECTION_TxTSP,
    SECTION_TxNCO,
    SECTION_RxTSP,
    SECTION_RxNCO,
    0x500, 0x5A7, //GFIR3
    0x5C0, 0x5C0 //DC Calibration Configuration
};
xdata uint16_t chipStateData[500];

void SaveChipState(bool wr)
{
    uint16_t dest=0;
    uint8_t i;
    uint16_t addr;
    uint16_t ch = SPI_read(0x0020);
    if(!wr)
        x0020state = ch;
    //for(i=0; i<sizeof(chipStateAddr)/sizeof(uint16_t); i+=2)
    for(i = sizeof(chipStateAddr)/sizeof(uint16_t); i; i-=2)
    {
        for(addr=chipStateAddr[i-2]; addr<=chipStateAddr[i-1]; ++addr)
        {
            if(wr)
                SPI_write(addr, chipStateData[dest]);
            else
                chipStateData[dest] = SPI_read(addr);
            ++dest;
        }
    }
    //sxr and sxt
    for(i=0; i<2; ++i)
    {
        SPI_write(0x0020, 0xFFFD+i);
        for(addr=0x011C; addr<=0x0123; ++addr)
        {
            if(wr)
                SPI_write(addr, chipStateData[dest]);
            else
                chipStateData[dest] = SPI_read(addr);
            ++dest;
        }
    }
    if(wr)
    {
        ClockLogicResets();
        SPI_write(0x0020, x0020state);
    }
    else
        SPI_write(0x0020, ch);
}

#ifdef __cplusplus
#include <vector>
void SetDefaults(uint16_t start, uint16_t end)
{
    ROM const uint16_t defaultAddrs[] = {
0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,0x0028,0x0029,0x002A,0x002B,0x002C,0x002E,0x002F,0x0081,0x0082,0x0084,0x0085,0x0086,0x0087,0x0088,0x0089,0x008A,0x008B,0x008C,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,0x00A8,0x00AA,0x00AB,0x00AD,0x00AE,0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x010B,0x010C,0x010D,0x010E,0x010F,0x0110,0x0111,0x0112,0x0113,0x0114,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x011C,0x011D,0x011E,0x011F,0x0120,0x0121,0x0122,0x0123,0x0124,0x0125,0x0126,0x0200,0x0201,0x0202,0x0203,0x0204,0x0205,0x0206,0x0207,0x0208,0x0209,0x020A,0x020C,0x0240,0x0242,0x0243,0x0244,0x0245,0x0246,0x0247,0x0248,0x0249,0x024A,0x024B,0x024C,0x024D,0x024E,0x024F,0x0250,0x0251,0x0252,0x0253,0x0254,0x0255,0x0256,0x0257,0x0258,0x0259,0x025A,0x025B,0x025C,0x025D,0x025E,0x025F,0x0260,0x0261,0x0400,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,0x0408,0x0409,0x040A,0x040B,0x040C,0x040E,0x0440,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F,0x0450,0x0451,0x0452,0x0453,0x0454,0x0455,0x0456,0x0457,0x0458,0x0459,0x045A,0x045B,0x045C,0x045D,0x045E,0x045F,0x0460,0x0461,0x05C0,0x05C1,0x05C2,0x05C3,0x05C4,0x05C5,0x05C6,0x05C7,0x05C8,0x05C9,0x05CA,0x05CB,0x05CC,0x0600,0x0601,0x0602,0x0603,0x0604,0x0605,0x0606,0x0640,0x0641
};
    ROM const uint16_t defaultValues[] = {
0xFFFF,0x0E9F,0x07DF,0x5559,0xE4E4,0x0101,0x0101,0xE4E4,0x0101,0x0101,0x0086,0x0010,0xFFFF,0x0000,0x3840,0x0000,0x800B,0x0400,0x0001,0x4901,0x0400,0x0780,0x0020,0x0514,0x2100,0x067B,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x6565,0x658C,0x6565,0x658C,0x6565,0x658C,0x658C,0x6565,0x6565,0x6565,0x6565,0x6565,0x6565,0x000F,0x6565,0x0000,0x0000,0x0000,0x03FF,0xF000,0x3409,0x7800,0x3180,0x0A12,0x0088,0x0007,0x318C,0x318C,0x958C,0x61C1,0x104C,0x0000,0x88FD,0x009E,0x2040,0x30C6,0x0994,0x0083,0xC0E6,0x03C3,0x008D,0x0009,0x8180,0x280C,0x018C,0x528B,0x2E02,0xAD43,0x0400,0x0780,0x3640,0xB9FF,0x3404,0x033F,0x067B,0x0000,0x9400,0x12FF,0x0081,0x07FF,0x07FF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0070,0x0000,0x0000,0x0000,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0081,0x07FF,0x07FF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1000,0x0000,0x0038,0x0000,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x2020,0x0000,0x0F00,0x0000,0x2000,0x0000,0x0000,0x0000,0x0000,0x00A0,0x1020
};
#ifdef __cplusplus
    //int status = 0;
    std::vector<uint16_t> addrs;
    std::vector<uint16_t> values;
    for(uint32_t address = start; address <= end; ++address)
    {
        unsigned int i=0;
        for(i=0; i<sizeof(defaultAddrs)/sizeof(uint16_t); ++i)
        {
            if(defaultAddrs[i] == address)
            {
                addrs.push_back(defaultAddrs[i]);
                values.push_back(defaultValues[i]);
                break;
            }
        }
    }
    SPI_write_batch(addrs.data(), values.data(), addrs.size());
#else

#endif
}
#endif
void SetDefaultsSX()
{
    ROM const uint16_t SXAddr[]=    {0x011C, 0x011D, 0x011E, 0x011F, 0x0121, 0x0122, 0x0123};
    ROM const uint16_t SXdefVals[]= {0xAD43, 0x0400, 0x0780, 0x3640, 0x3404, 0x033F, 0x067B};

    uint8_t i;
    for(i=sizeof(SXAddr)/sizeof(uint16_t); i; --i)
        SPI_write(SXAddr[i-1], SXdefVals[i-1]);
    //keep 0x0120[7:0]ICT_VCO bias value intact
    Modify_SPI_Reg_bits(0x0120, MSB_LSB(15, 8), 0xB9FF);
}

void ClockLogicResets()
{
    //MCLK2 toggle
    uint16_t reg = SPI_read(0x002B);
    SPI_write(0x002B, reg ^ (1<<9));
    SPI_write(0x002B, reg);

    //TSP logic reset
    reg = SPI_read(0x0020);
    SPI_write(0x0020, reg & ~0xAA00);
    SPI_write(0x0020, reg);
}

float_type GetFrequencyCGEN()
{
    const float_type dMul = (RefClk/2.0)/(Get_SPI_Reg_bits(DIV_OUTCH_CGEN)+1); //DIV_OUTCH_CGEN
    const uint16_t gINT = Get_SPI_Reg_bits(0x0088, MSB_LSB(13, 0)); //read whole register to reduce SPI transfers
    const uint32_t gFRAC = ((uint32_t)(gINT & 0xF) << 16) | Get_SPI_Reg_bits(0x0087, MSB_LSB(15, 0));
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

uint8_t SetFrequencyCGEN(float_type freq)
{
    float_type dFvco;
    float_type intpart;
    //VCO frequency selection according to F_CLKH
    {
        uint8_t iHdiv_high = (2.94e9/2 / freq)-1;
        uint8_t iHdiv_low = (1.93e9/2 / freq);
        uint8_t iHdiv = (iHdiv_low + iHdiv_high)/2;
        dFvco = 2 * (iHdiv+1) * freq;
        Modify_SPI_Reg_bits(DIV_OUTCH_CGEN, iHdiv);
    }
    //Integer division
    intpart = dFvco/RefClk;
    Modify_SPI_Reg_bits(INT_SDM_CGEN, intpart - 1); //INT_SDM_CGEN
    //Fractional division
    {
        const float_type dFrac = intpart - (uint32_t)(dFvco/RefClk);
        const uint32_t gFRAC = (uint32_t)(dFrac * 1048576);
        Modify_SPI_Reg_bits(0x0087, MSB_LSB(15, 0), gFRAC&0xFFFF); //INT_SDM_CGEN[15:0]
        Modify_SPI_Reg_bits(0x0088, MSB_LSB(3, 0), gFRAC>>16); //INT_SDM_CGEN[19:16]
    }

#if VERBOSE
    //printf("CGEN: Freq=%g MHz, VCO=%g GHz, INT=%i, FRAC=%i, DIV_OUTCH_CGEN=%i\n", freq/1e6, dFvco/1e9, gINT, gFRAC, iHdiv);
#endif // NDEBUG
    if(TuneVCO(VCO_CGEN) != 0)
        return MCU_CGEN_TUNE_FAILED;
    return 0;
}

float_type GetReferenceClk_TSP_MHz(bool tx)
{
    const float_type cgenFreq = GetFrequencyCGEN();
    const float_type clklfreq = cgenFreq/pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 0)
        return tx ? clklfreq : cgenFreq/4;
    else
        return tx ? cgenFreq : clklfreq/4;
}

void SetNCOFrequency(const bool tx, const float freq, uint8_t index)
{
    const uint16_t addr = (tx ? 0x0242 : 0x0442)+index*2;
    const uint32_t fcw = (uint32_t)((freq/GetReferenceClk_TSP_MHz(tx))*4294967296.0);
    SPI_write(addr, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+1, fcw); //NCO frequency control word register LSB part.
}

float_type GetFrequencySX(const bool Tx)
{
    const uint16_t ch = SPI_read(0x0020);//(uint8_t)Get_SPI_Reg_bits(MAC); //remember previously used channel
    Modify_SPI_Reg_bits(MAC, Tx ? 2 : 1); // Rx mac = 1, Tx mac = 2
    {
        const uint16_t gINT = Get_SPI_Reg_bits(0x011E, MSB_LSB(13, 0));    // read whole register to reduce SPI transfers
        const uint32_t gFRAC = ((uint32_t)(gINT&0xF) << 16) | SPI_read(0x011D);
        const uint8_t enDiv2 = Get_SPI_Reg_bits(EN_DIV2_DIVPROG)+1;
        const uint8_t divLoch = Get_SPI_Reg_bits(DIV_LOCH) + 1;
        SPI_write(0x0020, ch);
        //Calculate real frequency according to the calculated parameters
        return (enDiv2) * (RefClk / pow2(divLoch)) * ((gINT >> 4) + 4 + (gFRAC / 1048576.0));
    }
}

uint8_t SetFrequencySX(const bool tx, const float_type freq_Hz)
{
    int16_t tuneScore[3] = {255, 255, 255}; // best is closest to 0
    const uint16_t macBck = SPI_read(0x0020);
    bool canDeliverFrequency = false;

    Modify_SPI_Reg_bits(MAC, tx?2:1);
    //find required VCO frequency
    {
        float_type VCOfreq;
        float_type temp;
        {
            uint8_t div_loch;
            for (div_loch = 7; div_loch; --div_loch)
            {
                VCOfreq = pow2(div_loch) * freq_Hz;
                if ((VCOfreq >= 3800e6) && (VCOfreq <= 7714e6))
                    break;
            }
            Modify_SPI_Reg_bits(DIV_LOCH, div_loch-1);
        }
        {
            uint32_t fractionalPart;
            const uint8_t enDiv2 = (VCOfreq > 5500e6) ? 1 : 0;
            Modify_SPI_Reg_bits(EN_DIV2_DIVPROG, enDiv2); //EN_DIV2_DIVPROG
            temp = VCOfreq / (RefClk * (1 + enDiv2));
            fractionalPart = (uint32_t)((temp - (uint32_t)(temp)) * 1048576);

            Modify_SPI_Reg_bits(INT_SDM, (uint16_t)(temp - 4)); //INT_SDM
            Modify_SPI_Reg_bits(0x011E, MSB_LSB(3, 0), (fractionalPart >> 16)); //FRAC_SDM[19:16]
            SPI_write(0x011D, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
        }
    }
    while(1)
    {
        uint8_t sel_vco;
        uint8_t bestvco = 0;
        for (sel_vco = 0; sel_vco < 3; ++sel_vco)
        {
            Modify_SPI_Reg_bits(SEL_VCO, sel_vco);
            if( TuneVCO(1) == MCU_NO_ERROR)
            {
                tuneScore[sel_vco] = Get_SPI_Reg_bits(CSW_VCO)-128;
                canDeliverFrequency = true;
            }
            if(abs(tuneScore[sel_vco]) < abs(tuneScore[bestvco]))
                bestvco = sel_vco;
        }
        if(canDeliverFrequency)
        {
            Modify_SPI_Reg_bits(SEL_VCO, bestvco);
            Modify_SPI_Reg_bits(CSW_VCO, tuneScore[bestvco] + 128);
            break;
        }
        {
            uint16_t bias = Get_SPI_Reg_bits(ICT_VCO);
            if(bias == 255)
                break;
            Modify_SPI_Reg_bits(ICT_VCO, bias + 32 > 255 ? 255 : bias + 32);
        }
    }
    SPI_write(0x0020, macBck);
    if (canDeliverFrequency == false)
        return tx ? MCU_SXT_TUNE_FAILED : MCU_SXR_TUNE_FAILED;
    return MCU_NO_ERROR;
}

/** @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
    @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
    @return 0-success, other-failure
*/
static uint8_t ReadCMP(const bool SX)
{
#ifdef __cplusplus
    std::this_thread::sleep_for(std::chrono::microseconds(100));
#else
    TR0 = 0; //stop timer 0
    TH0 = (gComparatorDelayCounter >> 8);
    TL0 = (gComparatorDelayCounter & 0xFF);
    TF0 = 0; // clear overflow
    TR0 = 1; //start timer 0
    while( !TF0 ); // wait for timer overflow
#endif
    return (uint8_t)Get_SPI_Reg_bits(SX ? 0x0123 : 0x008C, MSB_LSB(13, 12));
}

uint8_t TuneVCO(bool SX) // 0-cgen, 1-SXR, 2-SXT
{
    typedef struct
    {
        uint8_t high;
        uint8_t low;
        uint8_t hasLock;
    } CSWInteval;

    uint16_t addrCSW_VCO;
    uint8_t msblsb;

    CSWInteval cswSearch[2];
    cswSearch[0].high = 0; //search interval lowest value
    cswSearch[0].low = 127;
    cswSearch[1].high = 128;
    cswSearch[1].low = 255;
    cswSearch[1].hasLock = cswSearch[0].hasLock = false;

    if(SX)
    {
        addrCSW_VCO = 0x0121;
        msblsb = MSB_LSB(10, 3); //CSW msb lsb
        //assuming the active channel is already correct
        Modify_SPI_Reg_bits(0x011C, MSB_LSB(2, 1), 0); //activate VCO and comparator
    }
    else //CGEN
    {
        addrCSW_VCO = 0x008B;
        msblsb = MSB_LSB(8, 1); //CSW msb lsb
        Modify_SPI_Reg_bits(0x0086, MSB_LSB(2, 1), 0); //activate VCO and comparator
    }
#ifndef __cplusplus
    gComparatorDelayCounter = 0xFFFF - (uint16_t)((0.0009/12)*RefClk); // ~900us
#endif
    //check if lock is within VCO range
    Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, 0);
    if(ReadCMP(SX) == 3) //VCO too high
        return MCU_ERROR;
    Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, 255);
    if(ReadCMP(SX) == 0) //VCO too low
        return MCU_ERROR;

    //search intervals [0-127][128-255]
    {
        uint8_t t;
        for(t=0; t<2; ++t)
        {
            uint8_t mask;
            for(mask = (1 << 6); mask; mask >>= 1)
            {
                uint8_t cmpValue;
                cswSearch[t].high |= mask; // CSW_VCO<i>=1
                Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswSearch[t].high);
                cmpValue = ReadCMP(SX);
                if(cmpValue == 0x03) // reduce CSW
                    cswSearch[t].high ^= mask; // CSW_VCO<i>=0
                else if(cmpValue == 0x02 && cswSearch[t].high <= cswSearch[t].low)
                {
                    cswSearch[t].hasLock = true;
                    cswSearch[t].low = cswSearch[t].high;
                }
            }
            for(; cswSearch[t].low; --cswSearch[t].low)
            {
                Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswSearch[t].low);
                if(ReadCMP(SX) != 0x2)
                {
                    if(cswSearch[t].low < cswSearch[t].high)
                        ++cswSearch[t].low;
                    break;
                }
            }
        }
    }
    {
        uint8_t cswValue;
        //compare which interval is wider
        {
            if(cswSearch[1].hasLock && (cswSearch[1].high-cswSearch[1].low >= cswSearch[0].high-cswSearch[0].low))
                cswValue = cswSearch[1].low +((cswSearch[1].high-cswSearch[1].low) >> 1);
            else
                cswValue = cswSearch[0].low +((cswSearch[0].high-cswSearch[0].low) >> 1);
        }

        Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswValue);
        if(ReadCMP(SX) != 0x2) //just in case high-low==1, if low fails, check if high locks
            Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, ++cswValue);
    }
    if(ReadCMP(SX) == 0x2)
        return MCU_NO_ERROR;
    return MCU_ERROR;
}

void WriteMaskedRegs(const RegisterBatch ROM* regs)
{
    uint8_t i;
    uint8_t index;
    for(i=regs->cnt; i; --i)
    {
        index = i-1;
        SPI_write(regs->addr[index], ( SPI_read(regs->addr[index]) & ~regs->mask[index] ) | regs->val[index]);
    }
    for(i=regs->wrOnlyAddrCnt; i; --i)
    {
        index = i-1;
        SPI_write(regs->wrOnlyAddr[index], i > regs->wrOnlyDataCnt ? 0 : regs->wrOnlyData[index]);
    }
}

uint8_t GetValueOf_c_ctl_pga_rbb(uint8_t g_pga_rbb)
{
    if(g_pga_rbb < 21)
        return 1;
    if(g_pga_rbb < 13)
        return 2;
    if(g_pga_rbb < 8)
        return 3;
    return 0;
}

void EnableChannelPowerControls()
{
    uint16_t afe = SPI_read(0x0082);
    uint16_t value = SPI_read(0x0020);
    if((value & 3) == 1)
    {
        value = value | 0x0014;
        afe &= ~0x14;
    }
    else
    {
        value = value | 0x0028;
        afe &= ~0x0A;
    }
    SPI_write(0x0020, value);
    SPI_write(0x0082, afe);
}

void EnableMIMOBuffersIfNecessary()
{
//modifications when calibrating channel B
    uint16_t x0020val = SPI_read(0x0020);
    if( (x0020val&0x3) == 2)
    {
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(EN_NEXTRX_RFE, 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        SPI_write(0x0020, x0020val);
    }
}
