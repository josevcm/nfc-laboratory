#include "LMS7002M.h"
#include <assert.h>
#include "MCU_BD.h"
#include "IConnection.h"
#include "mcu_programs.h"
#include <chrono>
#include <thread>
#include "Logger.h"
#include "LMSBoards.h"

#ifndef NDEBUG
#define LMS_VERBOSE_OUTPUT
#endif

#ifdef LMS_VERBOSE_OUTPUT
static const bool verboseEnabled = true;
#else
static const bool verboseEnabled = false;
#endif

using namespace std;
using namespace lime;

class BoardLoopbackStore
{
public:
    BoardLoopbackStore(lime::IConnection* port) : port(port)
    {
        if(port && port->ReadRegister(LoopbackCtrAddr, mLoopbackState) != 0)
            lime::ReportError(-1, "Failed to read Loopback controls");
    }
    ~BoardLoopbackStore()
    {
        if(port && port->WriteRegister(LoopbackCtrAddr, mLoopbackState) != 0)
            lime::ReportError(-1, "Failed to restore Loopback controls");
    }
private:
    lime::IConnection* port;
    static const uint32_t LoopbackCtrAddr = 0x0017;
    int mLoopbackState;
};

static uint8_t GetExtLoopPair(lime::LMS7002M &ctr, bool calibratingTx)
{
    uint8_t loopPair = 0;
    lime::IConnection* port = ctr.GetConnection();
    if(!port)
        return 0;

    auto devName = port->GetDeviceInfo().deviceName;
    uint8_t activeLNA = ctr.Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE);
    uint8_t activeBand = (ctr.Get_SPI_Reg_bits(LMS7_SEL_BAND2_TRF) << 1 | ctr.Get_SPI_Reg_bits(LMS7_SEL_BAND1_TRF))-1;

    if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR))
        loopPair = 1 << 2 | 0x1; // band2 -> LNAH
    else if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI))
        loopPair = activeBand << 2 | activeLNA;
    else if (devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2))
        loopPair = activeBand << 2 | activeLNA;
    return loopPair;
}

/*!
 * Convert the 12-bit twos compliment register into a signed integer
 */
static inline int16_t signextIqCorr(const uint16_t regVal)
{
    int16_t signedPhase = int16_t(regVal << 4);
    return int16_t(signedPhase) >> 4;
}

const double TrxCalib_RF_LimitLow = 2.5e6;
const double TrxCalib_RF_LimitHigh = 120e6;

static const char cSquaresLine[] =
    "############################################################\n";
static const char cDashLine[] =
    "------------------------------------------------------------\n";

#define verbose_printf(...) \
    do { if (verboseEnabled) {\
        fprintf(stderr, __VA_ARGS__);\
    }} while (0)

static int16_t ReadAnalogDC(lime::LMS7002M* lmsControl, const LMS7Parameter& param)
{
    uint16_t mask = param.address < 0x05C7 ? 0x03FF : 0x003F;

    lmsControl->SPI_write(param.address, 0);
    lmsControl->SPI_write(param.address, 0x4000);
    uint16_t value = lmsControl->SPI_read(param.address, true);
    lmsControl->SPI_write(param.address, value & ~0xC000);
    int16_t result = (value & mask);
    if(value& (mask+1))
        result *= -1;
    return result;
}

static int SetExtLoopback(IConnection* port, uint8_t ch, bool enable, bool tx)
{
    //enable external loopback switches
    const uint32_t LoopbackCtrAddr = 0x0017;
    uint32_t value = 0;
    int status;
    status = port->ReadRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return -1;
    auto devName = port->GetDeviceInfo().deviceName;

    if(devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDR))
    {
        const uint16_t mask = 0x7;
        const uint8_t shiftCount = (ch==2 ? 4 : 0);
        value &= ~(mask << shiftCount);
        value |= enable << shiftCount;   //EN_Loopback
        value |= enable << (shiftCount+1); //EN_Attenuator
        value |= !enable << (shiftCount+2); //EN_Shunt
    }
    else if (devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI) || devName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2))
    {
        //EN_Shunt
        value &= ~(1 << 2);
        value |= !enable << 2;

        if(tx)
        {
            uint32_t wr = 0x0103 << 16;
            uint32_t path;
            port->ReadLMS7002MSPI(&wr, &path, 1);
            path >>= 10;
            path &= 0x3;
            if (path==1)
            {
                value &= ~(1<<13);
                value |= 1<<12;

                value &= ~(1<<8); //LNA_H
                value |= 1<<9;
            }
            else if (path==2)
            {
                value &= ~(1<<12);
                value |= 1<<13;

                value &= ~(1<<9); //LNA_W
                value |= 1<<8;
            }
            //value |= enable << 1; //EN_Attenuator
        }
        else
        {
            uint32_t wr = 0x010D << 16;
            uint32_t path;
            port->ReadLMS7002MSPI(&wr, &path, 1);
            path &= ~0x0180;
            path >>= 7;
            if (path==1)
            {
                value &= ~(1<<8);
                value |= 1<<9;
            }
            else if (path==3)
            {
                value &= ~(1<<9);
                value |= 1<<8;
            }
        }
    }
    status = port->WriteRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return ReportError(status, "Failed to enable external loopback");
    return status;
}

/** @brief Flips the CAPTURE bit and returns digital RSSI value
*/
uint32_t LMS7002M::GetRSSI(RSSI_measurements *measurements)
{
    //delay to make sure RSSI gets enough samples to refresh before reading it
    this_thread::sleep_for(chrono::microseconds(50));
    Modify_SPI_Reg_bits(LMS7_CAPTURE, 0);
    Modify_SPI_Reg_bits(LMS7_CAPTURE, 1);
    uint32_t rssi = (Get_SPI_Reg_bits(0x040F, 15, 0, true) << 2) | Get_SPI_Reg_bits(0x040E, 1, 0, true);
    return rssi;
}

/** @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
@return 0-success, other-failure
*/
int LMS7002M::CalibrateTx(float_type bandwidth_Hz, bool useExtLoopback)
{
    if (TrxCalib_RF_LimitLow > bandwidth_Hz)
    {
        lime::warning("Calibrating Tx for %g MHz (requested %g MHz [out of range])", TrxCalib_RF_LimitLow/1e6, bandwidth_Hz/1e6);
        bandwidth_Hz = TrxCalib_RF_LimitLow;
    }
    else if (bandwidth_Hz > TrxCalib_RF_LimitHigh)
    {
        lime::warning("Calibrating Tx for %g MHz (requested %g MHz [out of range])", TrxCalib_RF_LimitHigh/1e6, bandwidth_Hz/1e6);
        bandwidth_Hz = TrxCalib_RF_LimitHigh;
    }
    if(controlPort == nullptr)
        return ReportError(EINVAL, "Tx Calibration: Device not connected");
#ifdef LMS_VERBOSE_OUTPUT
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
    int status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7_MAC);
    if(ch == 0 || ch == 3)
        return ReportError(EINVAL, "Tx Calibration: Incorrect channel selection MAC %i", ch);

    //caching variables
    DeviceInfo info = controlPort->GetDeviceInfo();
    double txFreq = GetFrequencySX(LMS7002M::Tx);
    uint8_t channel = ch == 1 ? 0 : 1;
    int band = Get_SPI_Reg_bits(LMS7_SEL_BAND1_TRF) ? 0 : 1;

    int dccorri(0), dccorrq(0), gcorri(0), gcorrq(0),phaseOffset(0);
    verbose_printf("Tx calibration using MCU %s loopback\n",
                    useExtLoopback ? "EXTERNAL" : "INTERNAL");
    verbose_printf("Tx ch.%s @ %4g MHz, BW: %g MHz, RF output: %s, Gain: %i\n",
                    channel ? "B" : "A",
                    txFreq/1e6,
                    bandwidth_Hz/1e6,
                    band ? "BAND2" : "BAND1",
                    Get_SPI_Reg_bits(LMS7_CG_IAMP_TBB));

    uint8_t mcuID = mcuControl->ReadMCUProgramID();
    verbose_printf("Current MCU firmware: %i, %s\n", mcuID,
        mcuID == MCU_ID_CALIBRATIONS_SINGLE_IMAGE ? "DC/IQ calibration full":"unknown");
    if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        verbose_printf("Uploading DC/IQ calibration firmware\n");
        status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
        if(status != 0)
            return status;
    }

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    verbose_printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
    //Tx Rx separation bandwidth while calibrating
    mcuControl->SetParameter(MCU_BD::MCU_BW, bandwidth_Hz);

    {
       //BoardLoopbackStore onBoardLoopbackRestoration(GetConnection());
        if(useExtLoopback)
        {
            status = SetExtLoopback(controlPort, ch, true, true);
            if(status != 0)
                return ReportError(EINVAL, "Tx Calibration: Failed to enable external loopback");
            uint8_t loopPair = GetExtLoopPair(*this, true);
            mcuControl->SetParameter(MCU_BD::MCU_EXT_LOOPBACK_PAIR, loopPair);
        }
        mcuControl->RunProcedure(useExtLoopback ? MCU_FUNCTION_CALIBRATE_TX_EXTLOOPB : MCU_FUNCTION_CALIBRATE_TX);
        status = mcuControl->WaitForMCU(1000);
        if(status != MCU_BD::MCU_NO_ERROR)
            return ReportError(EINVAL, "Tx Calibration: MCU error %i (%s)", status, MCU_BD::MCUStatusMessage(status));
    }

    //sync registers to cache
    const std::vector<uint16_t> regsToSync = {0x0208, 0x05C0};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    //need to read back calibration results
    dccorri = ReadAnalogDC(this, channel ? LMS7_DC_TXBI : LMS7_DC_TXAI);
    dccorrq = ReadAnalogDC(this, channel ? LMS7_DC_TXBQ : LMS7_DC_TXAQ);
    gcorri = Get_SPI_Reg_bits(LMS7_GCORRI_TXTSP, true);
    gcorrq = Get_SPI_Reg_bits(LMS7_GCORRQ_TXTSP, true);
    phaseOffset = signextIqCorr(Get_SPI_Reg_bits(LMS7_IQCORR_TXTSP, true));

    Log("Tx calibration finished", LOG_INFO);
#ifdef LMS_VERBOSE_OUTPUT
    verbose_printf("Tx | DC  | GAIN | PHASE\n");
    verbose_printf("---+-----+------+------\n");
    verbose_printf("I: | %3i | %4i | %i\n", dccorri, gcorri, phaseOffset);
    verbose_printf("Q: | %3i | %4i |\n", dccorrq, gcorrq);
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now()-beginTime).count();
    verbose_printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

/** @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateRx(float_type bandwidth_Hz, bool useExtLoopback)
{
    if (TrxCalib_RF_LimitLow > bandwidth_Hz)
    {
        lime::warning("Calibrating Rx for %g MHz (requested %g MHz [out of range])", TrxCalib_RF_LimitLow/1e6, bandwidth_Hz/1e6);
        bandwidth_Hz = TrxCalib_RF_LimitLow;
    }
    else if (bandwidth_Hz > TrxCalib_RF_LimitHigh)
    {
        lime::warning("Calibrating Rx for %g MHz (requested %g MHz [out of range])", TrxCalib_RF_LimitHigh/1e6, bandwidth_Hz/1e6);
        bandwidth_Hz = TrxCalib_RF_LimitHigh;
    }
    if(controlPort == nullptr)
        return ReportError(ENODEV, "Rx Calibration: Device not connected");
#ifdef LMS_VERBOSE_OUTPUT
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif

    DeviceInfo info = controlPort->GetDeviceInfo();
    int status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7_MAC);
    if(ch == 0 || ch == 3)
        return ReportError(EINVAL, "Rx Calibration: Incorrect channel selection MAC %i", ch);
    uint8_t channel = ch == 1 ? 0 : 1;
    uint8_t lna = (uint8_t)Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE);
    double rxFreq = GetFrequencySX(LMS7002M::Rx);

    const char* lnaName;
    switch(lna)
    {
        case 0: lnaName = "none"; break;
        case 1: lnaName = "LNAH"; break;
        case 2: lnaName = "LNAL"; break;
        case 3: lnaName = "LNAW"; break;
        default: lnaName = "none"; break;
    }
    verbose_printf("Rx calibration using %s loopback\n",
        (useExtLoopback ? "EXTERNAL" : "INTERNAL"));
    verbose_printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
                ch == Channel::ChA ? "A" : "B", rxFreq/1e6,
                bandwidth_Hz/1e6, lnaName,
                Get_SPI_Reg_bits(LMS7_G_PGA_RBB),
                Get_SPI_Reg_bits(LMS7_G_LNA_RFE),
                Get_SPI_Reg_bits(LMS7_G_TIA_RFE));

    int dcoffi(0), dcoffq(0), gcorri(0), gcorrq(0), phaseOffset(0);
    //check if MCU has correct firmware
    uint8_t mcuID = mcuControl->ReadMCUProgramID();
    verbose_printf("Current MCU firmware: %i, %s\n", mcuID,
        mcuID == MCU_ID_CALIBRATIONS_SINGLE_IMAGE ? "DC/IQ calibration full" : "unknown");
    if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        verbose_printf("Uploading DC/IQ calibration firmware\n");
        status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
        if(status != 0)
            return status;
    }

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    verbose_printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
    //Tx Rx separation bandwidth while calibrating
    mcuControl->SetParameter(MCU_BD::MCU_BW, bandwidth_Hz);

    {
        //BoardLoopbackStore onBoardLoopbackRestoration(GetConnection());
        if(useExtLoopback)
        {
            status = SetExtLoopback(controlPort, ch, true, false);
            if(status != 0)
                return ReportError(EINVAL, "Rx Calibration: Failed to enable external loopback");
            uint8_t loopPair = GetExtLoopPair(*this, false);
            mcuControl->SetParameter(MCU_BD::MCU_EXT_LOOPBACK_PAIR, loopPair);
        }

        mcuControl->RunProcedure(useExtLoopback ? MCU_FUNCTION_CALIBRATE_RX_EXTLOOPB : MCU_FUNCTION_CALIBRATE_RX);
        status = mcuControl->WaitForMCU(1000);
        if(status != MCU_BD::MCU_NO_ERROR)
            return ReportError(EINVAL, "Rx calibration: MCU error %i (%s)", status, MCU_BD::MCUStatusMessage(status));
    }

    //sync registers to cache
    const std::vector<uint16_t> regsToSync = {0x040C, 0x05C0};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    //read back for cache input and print
    dcoffi = ReadAnalogDC(this, channel ? LMS7_DC_RXBI : LMS7_DC_RXAI);
    dcoffq = ReadAnalogDC(this, channel ? LMS7_DC_RXBQ : LMS7_DC_RXAQ);
    gcorri = Get_SPI_Reg_bits(LMS7_GCORRI_RXTSP, true);
    gcorrq = Get_SPI_Reg_bits(LMS7_GCORRQ_RXTSP, true);
    phaseOffset = signextIqCorr(Get_SPI_Reg_bits(LMS7_IQCORR_RXTSP, true));

    Log("Rx calibration finished", LOG_INFO);
#ifdef LMS_VERBOSE_OUTPUT
    verbose_printf("RX | DC  | GAIN | PHASE\n");
    verbose_printf("---+-----+------+------\n");
    verbose_printf("I: | %3i | %4i | %i\n", dcoffi, gcorri, phaseOffset);
    verbose_printf("Q: | %3i | %4i |\n", dcoffq, gcorrq);
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now()-beginTime).count();
    verbose_printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

/** @brief Loads given DC_REG values into registers
    @param tx TxTSP or RxTSP selection
    @param I DC_REG I value
    @param Q DC_REG Q value
*/
int LMS7002M::LoadDC_REG_IQ(bool tx, int16_t I, int16_t Q)
{
    if(tx)
    {
        Modify_SPI_Reg_bits(LMS7_DC_REG_TXTSP, I);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_TXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_TXTSP, 1);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_TXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_DC_REG_TXTSP, Q);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_TXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_TXTSP, 1);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_TXTSP, 0);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7_DC_REG_RXTSP, I);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_RXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_RXTSP, 1);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDI_RXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_DC_REG_RXTSP, Q);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_RXTSP, 0);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_RXTSP, 1);
        Modify_SPI_Reg_bits(LMS7_TSGDCLDQ_RXTSP, 0);
    }
    return 0;
}
