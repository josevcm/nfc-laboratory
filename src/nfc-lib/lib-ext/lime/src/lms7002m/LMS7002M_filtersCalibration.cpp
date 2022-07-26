/**
@file	LMS7002M_filtersCalibration.cpp
@author Lime Microsystems (www.limemicro.com)
@brief	Implementation of LMS7002M transceiver filters calibration algorithms
*/

#include "LMS7002M.h"
#include "IConnection.h"
#include "Logger.h"
#include "LMS7002M_RegistersMap.h"
#include <cmath>
#include <iostream>
#include <assert.h>
#include "MCU_BD.h"
#include "mcu_programs.h"
#include "Logger.h"

#ifdef _MSC_VER
#include <ciso646>
#endif
using namespace lime;

//rx lpf range limits
static const float_type RxLPF_RF_LimitLow = 1.4e6;
static const float_type RxLPF_RF_LimitHigh = 130e6;

//tx lpf range limits
const float_type TxLPF_RF_LimitLow = 5e6;
const float_type TxLPF_RF_LimitLowMid = 40e6;
const float_type TxLPF_RF_LimitMidHigh = 50e6;
const float_type TxLPF_RF_LimitHigh = 130e6;

LMS7002M_RegistersMap *LMS7002M::BackupRegisterMap(void)
{
    //BackupAllRegisters(); return NULL;
    auto backup = new LMS7002M_RegistersMap();
    Channel chBck = this->GetActiveChannel();
    this->SetActiveChannel(ChA);
    *backup = *mRegistersMap;
    this->SetActiveChannel(chBck);
    return backup;
}

void LMS7002M::RestoreRegisterMap(LMS7002M_RegistersMap *backup)
{
    //RestoreAllRegisters(); return;
    Channel chBck = this->GetActiveChannel();

    for (int ch = 0; ch < 2; ch++)
    {
        //determine addresses that have been changed
        //and restore backup to the main register map
        std::vector<uint16_t> restoreAddrs, restoreData;
        for (const uint16_t addr : mRegistersMap->GetUsedAddresses(ch))
        {
            uint16_t original = backup->GetValue(ch, addr);
            uint16_t current = mRegistersMap->GetValue(ch, addr);
            mRegistersMap->SetValue(ch, addr, original);

            if (ch == 1 and addr < 0x0100) continue;
            if (original == current) continue;
            restoreAddrs.push_back(addr);
            restoreData.push_back(original);
        }

        //bulk write the original register values from backup
        this->SetActiveChannel((ch==0)?ChA:ChB);
        SPI_write_batch(restoreAddrs.data(), restoreData.data(), restoreData.size(), true);
    }

    //cleanup
    delete backup;
    backup = nullptr;
    this->SetActiveChannel(chBck);
}

int LMS7002M::TuneRxFilter(float_type rx_lpf_freq_RF)
{
    int status;
    if(RxLPF_RF_LimitLow > rx_lpf_freq_RF || rx_lpf_freq_RF > RxLPF_RF_LimitHigh)
        return ReportError(ERANGE, "RxLPF frequency out of range, available range from %g to %g MHz", RxLPF_RF_LimitLow/1e6, RxLPF_RF_LimitHigh/1e6);

    uint8_t g_tia = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
    if(g_tia == 1 && rx_lpf_freq_RF < 4e6)
    {
        rx_lpf_freq_RF = 4e6;
        Log(LOG_WARNING, "Rx LPF min bandwidth is 4MHz when TIA gain is set to -12 dB");
    }

    if(mcuControl->ReadMCUProgramID() != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        if((status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM)))
            return ReportError(status, "Tune Rx Filter: failed to program MCU");
    }

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    lime::debug("MCU Ref. clock: %g MHz", refClk / 1e6);
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    mcuControl->SetParameter(MCU_BD::MCU_BW, rx_lpf_freq_RF);
    mcuControl->RunProcedure(5);

    status = mcuControl->WaitForMCU(1000);
    if(status != MCU_BD::MCU_NO_ERROR)
    {
        lime::error("Tune Rx Filter: MCU error %i (%s)", status, MCU_BD::MCUStatusMessage(status));
        return -1;
    }
    //sync registers to cache
    std::vector<uint16_t> regsToSync = {0x0112, 0x0117, 0x011A, 0x0116, 0x0118, 0x0114, 0x0019, 0x0115};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    return status;
}

int LMS7002M::TuneTxFilter(const float_type tx_lpf_freq_RF)
{
    int status;

    if(tx_lpf_freq_RF < TxLPF_RF_LimitLow || tx_lpf_freq_RF > TxLPF_RF_LimitHigh)
        return ReportError(ERANGE, "Tx lpf(%g MHz) out of range %g-%g MHz and %g-%g MHz", tx_lpf_freq_RF/1e6,
                        TxLPF_RF_LimitLow/1e6, TxLPF_RF_LimitLowMid/1e6,
                        TxLPF_RF_LimitMidHigh/1e6, TxLPF_RF_LimitHigh/1e6);
    //calculate intermediate frequency
    float_type tx_lpf_IF = tx_lpf_freq_RF/2;
    if(tx_lpf_freq_RF > TxLPF_RF_LimitLowMid && tx_lpf_freq_RF < TxLPF_RF_LimitMidHigh)
    {
        Log(LOG_WARNING, "Tx lpf(%g MHz) out of range %g-%g MHz and %g-%g MHz. Setting to %g MHz", tx_lpf_freq_RF/1e6,
                        TxLPF_RF_LimitLow/1e6, TxLPF_RF_LimitLowMid/1e6,
                        TxLPF_RF_LimitMidHigh/1e6, TxLPF_RF_LimitHigh/1e6,
                        TxLPF_RF_LimitMidHigh/1e6);
        tx_lpf_IF = TxLPF_RF_LimitMidHigh/2;
    }

    if (!controlPort){
        lime::error("Tune Tx Filter: No device connected");
        return -1;
    }

    if(mcuControl->ReadMCUProgramID() != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        if((status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM)))
            return ReportError(status, "Tune Tx Filter: failed to program MCU");
    }

    int ind = this->GetActiveChannelIndex()%2;
    opt_gain_tbb[ind] = -1;

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    lime::debug("MCU Ref. clock: %g MHz", refClk / 1e6);
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    mcuControl->SetParameter(MCU_BD::MCU_BW, tx_lpf_freq_RF);
    mcuControl->RunProcedure(6);

    status = mcuControl->WaitForMCU(1000);
    if(status != MCU_BD::MCU_NO_ERROR)
    {
        lime::error("Tune Tx Filter: MCU error %i (%s)", status, MCU_BD::MCUStatusMessage(status));
        return -1;
    }
    //sync registers to cache
    std::vector<uint16_t> regsToSync = {0x0105, 0x0106, 0x0109, 0x010A, 0x010B};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
        Log(LOG_INFO, "Filter calibrated. Filter order-4th, filter bandwidth set to %g MHz."
            "Real pole 1st order filter set to 2.5 MHz. Preemphasis filter not active", tx_lpf_IF/1e6 * 2);
    else
        Log(LOG_INFO, "Filter calibrated. Filter order-2nd, set to %g MHz", tx_lpf_IF/1e6 * 2);
    return 0;
}
