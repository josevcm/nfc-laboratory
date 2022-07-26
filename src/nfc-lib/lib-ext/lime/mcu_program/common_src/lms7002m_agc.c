#include "lms7002m_controls.h"
#include "spi.h"
#include "LMS7002M_parameters_compact.h"
#include "typedefs.h"
#include "mcu_defines.h"
#include "lms7002m_calibrations.h"


#include <math.h>

#ifdef __cplusplus
bool hasStopped = false;
bool stopProcedure = false;
#else
extern bool hasStopped;
extern bool stopProcedure;
#endif

static uint16_t ROM stateAddresses[] = {0x0081, 0x010F, 0x0126, 0x040A, 0x040C};
//double space to store both channels
static uint16_t xdata stateStorage[2*sizeof(stateAddresses)/sizeof(uint16_t)];

static void StoreState(bool write)
{
    uint16_t x0020 = SPI_read(0x0020);
    uint8_t storageIndex = 0;
    uint8_t ch;
    for(ch = 2; ch>0; --ch)
    {
        uint8_t i = 0;
        SPI_write(0x0020, (x0020 & 0xFFFC) | i);
        for(; i < sizeof(stateAddresses)/sizeof(uint16_t); ++i)
        {
            if(write)
                SPI_write(stateAddresses[i], stateStorage[storageIndex]);
            else
                stateStorage[storageIndex] = SPI_read(stateAddresses[i]);
            ++storageIndex;
        }
    }
    SPI_write(0x0020, x0020);
}

#define TABLE_ENTRY(gain_setting, gainLNA, gainPGA) (gainPGA << 4 | gainLNA)
#define GET_PGA_GAIN(value) ((value >> 4) & 0x1F)
#define GET_LNA_GAIN(value) ((value) & 0xF)
static ROM const uint16_t AGC_gain_table[] = {
TABLE_ENTRY(-12,1,0),
TABLE_ENTRY(-11,1,1),
TABLE_ENTRY(-10,1,2),
TABLE_ENTRY(-9,2,0),
TABLE_ENTRY(-8,2,1),
TABLE_ENTRY(-7,2,2),
TABLE_ENTRY(-6,2,3),
TABLE_ENTRY(-5,3,1),
TABLE_ENTRY(-4,3,2),
TABLE_ENTRY(-3,3,3),
TABLE_ENTRY(-2,3,4),
TABLE_ENTRY(-1,4,2),
TABLE_ENTRY(0,4,3),
TABLE_ENTRY(1,4,4),
TABLE_ENTRY(2,4,5),
TABLE_ENTRY(3,5,3),
TABLE_ENTRY(4,5,4),
TABLE_ENTRY(5,5,5),
TABLE_ENTRY(6,5,6),
TABLE_ENTRY(7,6,4),
TABLE_ENTRY(8,6,5),
TABLE_ENTRY(9,6,6),
TABLE_ENTRY(10,6,7),
TABLE_ENTRY(11,7,5),
TABLE_ENTRY(12,7,6),
TABLE_ENTRY(13,7,7),
TABLE_ENTRY(14,7,8),
TABLE_ENTRY(15,8,6),
TABLE_ENTRY(16,8,7),
TABLE_ENTRY(17,8,8),
TABLE_ENTRY(18,8,9),
TABLE_ENTRY(19,9,7),
TABLE_ENTRY(20,9,8),
TABLE_ENTRY(21,9,9),
TABLE_ENTRY(22,9,10),
TABLE_ENTRY(23,10,10),
TABLE_ENTRY(24,10,11),
TABLE_ENTRY(25,10,12),
TABLE_ENTRY(26,10,13),
TABLE_ENTRY(27,11,13),
TABLE_ENTRY(28,11,14),
TABLE_ENTRY(29,11,15),
TABLE_ENTRY(30,11,16),
TABLE_ENTRY(31,11,17),
TABLE_ENTRY(32,11,18),
TABLE_ENTRY(33,11,19),
TABLE_ENTRY(34,11,20),
TABLE_ENTRY(35,11,21),
TABLE_ENTRY(36,11,22),
TABLE_ENTRY(37,11,23),
TABLE_ENTRY(38,11,24),
TABLE_ENTRY(39,11,25),
TABLE_ENTRY(40,11,26),
TABLE_ENTRY(41,11,27),
TABLE_ENTRY(42,11,28),
TABLE_ENTRY(43,11,29),
TABLE_ENTRY(44,11,30),
TABLE_ENTRY(45,11,31),
TABLE_ENTRY(46,12,31),
TABLE_ENTRY(47,13,31),
TABLE_ENTRY(48,14,31),
TABLE_ENTRY(49,15,31)
};

uint8_t RunAGC(uint32_t wantedRSSI)
{
    xdata uint32_t lastRSSI[2];
    uint16_t x0400[2];
    uint8_t status;
    uint8_t gainLNA[2];
    uint8_t gainPGA[2];
    uint8_t ch;
    uint16_t x0020 = SPI_read(0x0020);
    gainLNA[1] = gainLNA[0] = 11;
    gainPGA[1] = gainPGA[0] = 31;
    hasStopped = false;
    StoreState(false);
    //Setup
    if(Get_SPI_Reg_bits(TRX_GAIN_SRC) == false)
    {
        for(ch=0; ch<2; ++ch)
        {
            SPI_write(0x0020, (x0020 & 0xFFFC) | (ch+1));
            Modify_SPI_Reg_bits(CG_IAMP_TBB_R3, Get_SPI_Reg_bits(CG_IAMP_TBB));
            Modify_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF_R3, Get_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF));
            Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF_R3, Get_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF));
        }
    }
    Modify_SPI_Reg_bits(TRX_GAIN_SRC, 1);
    for(ch=0; ch<2; ++ch)
    {
        SPI_write(0x0020, (x0020 & 0xFFFC) | (ch+1));
        Modify_SPI_Reg_bits(ICT_TIAMAIN_RFE, 31);
        Modify_SPI_Reg_bits(ICT_TIAOUT_RFE, 4);
        //C_CTL_PGA_RBB 0, TIA 2
        SPI_write(0x0126, (gainPGA[ch] << 6) | (gainLNA[ch] << 2) | 2);

        status = CalibrateRx(false, false);
        //if(status != MCU_NO_ERROR)
            //goto AGC_END;
        //Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
        //Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 0);
        SPI_write(0x040A, 0x1000);
        Modify_SPI_Reg_bits(AGC_BYP_RXTSP, 0);

        if(Get_SPI_Reg_bits(CMIX_BYP_RXTSP) == 0)
        {
            Modify_SPI_Reg_bits(CMIX_GAIN_RXTSP, 1);
            Modify_SPI_Reg_bits(CMIX_GAIN_RXTSP_R3, 0);
        }
        x0400[ch] = SPI_read(0x0400) & ~0xF000; //CAPTURE 0, CAPSEL 0
    }
    UpdateRSSIDelay();

    while(!stopProcedure)
    {
        //Need to wait only
        for(ch=0; ch<2; ++ch)
        {
            float dBdiff;
            uint8_t LNA_gain_available;
            uint32_t rssi;
            bool needUpdate = false;
            SPI_write(0x0020, (x0020 & 0xFFFC) | (ch+1));
            //CAPTURE RSSI
            SPI_write(0x0400, x0400[ch]);
            SPI_write(0x0400, x0400[ch] | 0x8000);
            rssi = SPI_read(0x040F);
            rssi = (rssi << 2 | (SPI_read(0x040E) & 0x3));

            if(rssi == lastRSSI[ch] || rssi == 0)
                continue;
            lastRSSI[ch] = rssi;

            if(gainLNA[ch] <= 9)
                LNA_gain_available = 3*(11-gainLNA[ch]); //G_LNA=0 not allowed
            else
                LNA_gain_available = 15-gainLNA[ch];

            dBdiff = 20*log10((float)wantedRSSI/rssi);
            if (dBdiff < 0 && rssi > 0x14000)
            {
                gainPGA[ch] = clamp(gainPGA[ch] - 12, 0, 31);
                gainLNA[ch] = clamp(gainLNA[ch] -  3, 0, 15);
                needUpdate = true;
            }
            else if(dBdiff < 0 || (dBdiff > 0 && LNA_gain_available+31-gainPGA[ch] > 0))
            {
                int8_t total_gain_current = 30-LNA_gain_available + gainPGA[ch];
                const uint16_t gains = AGC_gain_table[clamp(total_gain_current+dBdiff, 0, 61)];
                uint8_t newLNA = GET_LNA_GAIN(gains);
                uint8_t newPGA = GET_PGA_GAIN(gains);
                if(newPGA != gainPGA[ch] || newLNA != gainLNA[ch])
                {
                    gainLNA[ch] = GET_LNA_GAIN(gains);
                    gainPGA[ch] = GET_PGA_GAIN(gains);
                    needUpdate = true;
                }
            }
            if(needUpdate)
            {
                uint16_t reg126;
                //set C_CTL_PGA_RBB
                reg126 = GetValueOf_c_ctl_pga_rbb(gainPGA[ch]) << 11;
                SPI_write(0x0126, reg126 | (gainPGA[ch] << 6) | (gainLNA[ch] << 2) | 2);
            }
        }
    }
    StoreState(true);
    SPI_write(0x0020, x0020);
    ClockLogicResets();
    hasStopped = true;
    return status;
}
