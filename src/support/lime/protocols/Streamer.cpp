#include <assert.h>
#include <ciso646>
#include <complex>

#include "Logger.h"
#include "Streamer.h"

#include "FPGA/FPGA_common.h"
#include "lms7002m/LMS7002M.h"
#include "ConnectionRegistry/IConnection.h"
#include "protocols/LMSBoards.h"

namespace lime
{

StreamChannel::StreamChannel(Streamer* streamer) :
    mActive(false)
{
    mStreamer = streamer;
    overflow = 0;
    underflow = 0;
    pktLost = 0;
    fifo = nullptr;
    used = false;
}

StreamChannel::~StreamChannel()
{
    if (fifo)
        delete fifo;
}

void StreamChannel::Setup(StreamConfig conf)
{
    used = true;
    config = conf;
    overflow = 0;
    underflow = 0;
    pktLost = 0;
    if (config.bufferLength == 0) //default size
        config.bufferLength = 1024*8*SamplesPacket::maxSamplesInPacket;
    else
    {
        size_t fifoSize = 64;
        while(fifoSize < conf.bufferLength/SamplesPacket::maxSamplesInPacket)
            fifoSize <<= 1;
        this->config.bufferLength = fifoSize*SamplesPacket::maxSamplesInPacket;
    }
    if (fifo)
        delete fifo;
    fifo = new RingFIFO(config.bufferLength);
}

void StreamChannel::Close()
{
    if (mActive)
        Stop();
    if (fifo)
        delete fifo;
    fifo = nullptr;
    used = false;
}

int StreamChannel::Write(const void* samples, const uint32_t count, const Metadata *meta, const int32_t timeout_ms)
{
    int pushed = 0;
    if(config.format == StreamConfig::FMT_FLOAT32 && config.isTx)
    {
        const float* samplesFloat = (const float*)samples;
        int16_t* samplesShort = new int16_t[2*count];
        for(size_t i=0; i<2*count; ++i)
            samplesShort[i] = samplesFloat[i]*32767.0f;
        const complex16_t* ptr = (const complex16_t*)samplesShort ;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
        delete[] samplesShort;
    }
    else
    {
        const complex16_t* ptr = (const complex16_t*)samples;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
    }
    return pushed;
}

int StreamChannel::Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms)
{
    int popped = 0;
    if(config.format == StreamConfig::FMT_FLOAT32 && !config.isTx)
    {
        //in place conversion
        complex16_t* ptr = (complex16_t*)samples;
        int16_t* samplesShort = (int16_t*)samples;
        float* samplesFloat = (float*)samples;
        popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
        for(int i=2*popped-1; i>=0; --i)
            samplesFloat[i] = (float)samplesShort[i]/32767.0f;
    }
    else
    {
        complex16_t* ptr = (complex16_t*)samples;
        popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
    }
    return popped;
}

StreamChannel::Info StreamChannel::GetInfo()
{
    Info stats;
    memset(&stats,0,sizeof(stats));
    RingFIFO::BufferInfo info = fifo->GetInfo();
    stats.fifoSize = info.size;
    stats.fifoItemsCount = info.itemsFilled;
    stats.active = mActive;
    stats.droppedPackets = pktLost;
    stats.overrun = overflow;
    stats.underrun = underflow;
    pktLost = 0;
    overflow = 0;
    underflow = 0;
    if(config.isTx)
    {
        stats.timestamp = mStreamer->txLastTimestamp.load(std::memory_order_relaxed);
        stats.linkRate = mStreamer->txDataRate_Bps.load(std::memory_order_relaxed);
    }
    else
    {
        stats.timestamp = mStreamer->rxLastTimestamp.load(std::memory_order_relaxed);
        stats.linkRate = mStreamer->rxDataRate_Bps.load(std::memory_order_relaxed);
    }
    return stats;
}

int StreamChannel::GetStreamSize()
{
    return mStreamer->GetStreamSize(config.isTx);
}

bool StreamChannel::IsActive() const
{
    return mActive;
}

int StreamChannel::Start()
{
    mActive = true;
    fifo->Clear();
    overflow = 0;
    underflow = 0;
    pktLost = 0;
    return mStreamer->UpdateThreads();
}

int StreamChannel::Stop()
{
    mActive = false;
    return mStreamer->UpdateThreads();
}

Streamer::Streamer(FPGA* f, LMS7002M* chip, int id) : mRxStreams(2, this), mTxStreams(2, this)
{
    lms = chip,
    fpga = f;
    chipId = id;
    dataPort = f->GetConnection();
    mTimestampOffset = 0;
    rxLastTimestamp.store(0, std::memory_order_relaxed);
    terminateRx.store(false, std::memory_order_relaxed);
    terminateTx.store(false, std::memory_order_relaxed);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
    txDataRate_Bps.store(0, std::memory_order_relaxed);
    txBatchSize = 1;
    rxBatchSize = 1;
    streamSize = 1;
}

Streamer::~Streamer()
{
    terminateRx.store(true, std::memory_order_relaxed);
    terminateTx.store(true, std::memory_order_relaxed);
    if (txThread.joinable())
        txThread.join();
    if (rxThread.joinable())
        rxThread.join();
}


StreamChannel* Streamer::SetupStream(const StreamConfig& config)
{
    const int ch = config.channelID&1;

    if ((config.isTx && mTxStreams[ch].used) || (!config.isTx && mRxStreams[ch].used))
    {
        lime::error("Setup Stream: Channel already in use");
        return nullptr;
    }

    if ((!mTxStreams[ch].used) && (!mRxStreams[ch].used) && (txThread.joinable() || rxThread.joinable()))
    {
        lime::warning("Stopping data stream to set up a new stream");
        UpdateThreads(true);
    }


    if(config.isTx)
        mTxStreams[ch].Setup(config);
    else
        mRxStreams[ch].Setup(config);

    double rate = lms->GetSampleRate(config.isTx,LMS7002M::ChA)/1e6;
    streamSize = (mTxStreams[0].used||mRxStreams[0].used) + (mTxStreams[1].used||mRxStreams[1].used);

    rate = (rate + 5) * config.performanceLatency * streamSize;
    for (int batch = 1; batch < rate; batch <<= 1)
        if (config.isTx)
            txBatchSize = batch;
        else
            rxBatchSize = batch;

    return config.isTx ? &mTxStreams[ch] : &mRxStreams[ch]; //success
}

int Streamer::GetStreamSize(bool tx)
{
    int batchSize = (tx ? txBatchSize : rxBatchSize)/streamSize;
    for(auto &i : mRxStreams)
        if(i.used && i.config.format != StreamConfig::FMT_INT12)
            return samples16InPkt*batchSize;

    for(auto &i : mTxStreams)
        if(i.used && i.config.format != StreamConfig::FMT_INT12)
            return samples16InPkt*batchSize;

    return samples12InPkt*batchSize;
}

uint64_t Streamer::GetHardwareTimestamp(void)
{
    if(!(rxThread.joinable() || txThread.joinable()))
    {
        //stop streaming just in case the board has not been configured
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->StopStreaming();
        fpga->ResetTimestamp();
        mTimestampOffset = 0;
        return 0;
    }
    else
    {
        return rxLastTimestamp.load(std::memory_order_relaxed)+mTimestampOffset;
    }
}

void Streamer::SetHardwareTimestamp(const uint64_t now)
{
    mTimestampOffset = now - rxLastTimestamp.load(std::memory_order_relaxed);
}

void Streamer::RstRxIQGen()
{
    uint32_t data[16];
    uint32_t reg20;
    uint32_t reg11C;
    uint32_t reg10C;
    data[0] = (uint32_t(0x0020) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg20, 1, chipId);
    data[0] = (uint32_t(0x010C) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg10C, 1, chipId);
    data[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;
    dataPort->WriteLMS7002MSPI(data, 1, chipId);
    data[0] = (uint32_t(0x011C) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg11C, 1, chipId);
    data[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;             //SXR
    data[1] = (1 << 31) | (uint32_t(0x011C) << 16) | (reg11C | 0x10);    //PD_FDIV
    data[2] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF;             // mac 3 - both channels
    data[3] = (1 << 31) | (uint32_t(0x0124) << 16) | 0x001F;             //direct control of powerdowns
    data[4] = (1 << 31) | (uint32_t(0x010C) << 16) | (reg10C | 0x8);     // PD_QGEN_RFE
    data[5] = (1 << 31) | (uint32_t(0x010C) << 16) | reg10C;             //restore value
    data[6] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;             //SXR
    data[7] = (1 << 31) | (uint32_t(0x011C) << 16) | reg11C;             //restore value
    data[8] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20;              //restore value
    dataPort->WriteLMS7002MSPI(data, 9, chipId);
}

void Streamer::AlignRxTSP()
{
    uint32_t reg20;
    uint32_t regsA[2];
    uint32_t regsB[2];
    //backup values
    {
        const std::vector<uint32_t> bakAddr = { (uint32_t(0x0400) << 16), (uint32_t(0x040C) << 16) };
        uint32_t data = (uint32_t(0x0020) << 16);
        dataPort->ReadLMS7002MSPI(&data, &reg20, 1, chipId);
        data = (uint32_t(0x0020) << 16) | 0xFFFD;
        dataPort->WriteLMS7002MSPI(&data, 1, chipId);
        dataPort->ReadLMS7002MSPI(bakAddr.data(), regsA, bakAddr.size(), chipId);
        data = (uint32_t(0x0020) << 16) | 0xFFFE;
        dataPort->WriteLMS7002MSPI(&data, 1, chipId);
        dataPort->ReadLMS7002MSPI(bakAddr.data(), regsB, bakAddr.size(), chipId);
    }

    //alignment search
    {
        uint32_t dataWr[4];
        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF;
        dataWr[1] = (1 << 31) | (uint32_t(0x0400) << 16) | 0x8085;
        dataWr[2] = (1 << 31) | (uint32_t(0x040C) << 16) | 0x01FF;
        dataPort->WriteLMS7002MSPI(dataWr, 3, chipId);
        uint32_t* buf = new uint32_t[sizeof(FPGA_DataPacket) / sizeof(uint32_t)];

        fpga->StopStreaming();
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->WriteRegister(0x0008, 0x0100);
        fpga->WriteRegister(0x0007, 3);

        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0x55FE;
        dataWr[1] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;

        for (int i = 0; i < 100; i++)
        {
            dataPort->WriteLMS7002MSPI(&dataWr[0], 2, chipId);
            dataPort->ResetStreamBuffers();
            fpga->StartStreaming();
            if (dataPort->ReceiveData((char*)buf, sizeof(FPGA_DataPacket), chipId, 50) != sizeof(FPGA_DataPacket))
            {
                lime::warning("Channel alignment failed");
                break;
            }
            fpga->StopStreaming();
            dataPort->AbortReading(chipId);
            if (buf[4] == buf[5])
                break;
        }
        delete[] buf;
    }

    //restore values
    {
        uint32_t dataWr[7];
        dataWr[0] = (uint32_t(0x0020) << 16) | 0xFFFD;
        dataWr[1] = (uint32_t(0x0400) << 16) | regsA[0];
        dataWr[2] = (uint32_t(0x040C) << 16) | regsA[1];
        dataWr[3] = (uint32_t(0x0020) << 16) | 0xFFFE;
        dataWr[4] = (uint32_t(0x0400) << 16) | regsB[0];
        dataWr[5] = (uint32_t(0x040C) << 16) | regsB[1];
        dataWr[6] = (uint32_t(0x0020) << 16) | reg20;
        dataPort->WriteLMS7002MSPI(dataWr, 7, chipId);
    }
}

double Streamer::GetPhaseOffset(int bin)
{
    int16_t* buf = new int16_t[sizeof(FPGA_DataPacket)/sizeof(int16_t)];

    dataPort->ResetStreamBuffers();
    fpga->StartStreaming();
    if (dataPort->ReceiveData((char*)buf, sizeof(FPGA_DataPacket), chipId, 50)!=sizeof(FPGA_DataPacket))
    {
        lime::warning("Channel alignment failed");
        delete [] buf;
        return -1000;
    }
    fpga->StopStreaming();
    dataPort->AbortReading(chipId);
    //calculate DFT bin of interest and check channel phase difference
    const std::complex<double> iunit(0, 1);
    const double pi = std::acos(-1);
    const int N = 512;
    std::complex<double> xA(0,0);
    std::complex<double> xB(0, 0);
    for (int n = 0; n < N; n++)
    {
        const std::complex<double> xAn(buf[8+4*n], buf[9+4*n]);
        const std::complex<double> xBn(buf[10+4*n],buf[11+4*n]);
        const std::complex<double> mult = std::exp(-2.0*iunit*pi* double(bin)* double(n)/double(N));
        xA += xAn * mult;
        xB += xBn * mult;
    }
    double phaseA = std::arg(xA) * 180.0 / pi;
    double phaseB = std::arg(xB) * 180.0 / pi;
    double phasediff = phaseB - phaseA;
    if (phasediff < -180.0) phasediff +=360.0;
    if (phasediff > 180.0) phasediff -=360.0;
    delete [] buf;
    return phasediff;
}

void Streamer::AlignRxRF(bool restoreValues)
{
    uint32_t addr = 0, val =0;
    fpga->ReadRegisters(&addr,&val,1);
    if (val!= LMS_DEV_LIMESDR && val != LMS_DEV_LIMESDR_PCIE)
        return;
    uint32_t reg20 = lms->SPI_read(0x20);
    auto regBackup = lms->BackupRegisterMap();
    lms->SPI_write(0x20, 0xFFFF);
    lms->SetDefaults(LMS7002M::RFE);
    lms->SetDefaults(LMS7002M::RBB);
    lms->SetDefaults(LMS7002M::TBB);
    lms->SetDefaults(LMS7002M::TRF);
    lms->SPI_write(0x10C, 0x88C5);
    lms->SPI_write(0x10D, 0x0117);
    lms->SPI_write(0x113, 0x024A);
    lms->SPI_write(0x118, 0x418C);
    lms->SPI_write(0x100, 0x4039);
    lms->SPI_write(0x101, 0x7801);
    lms->SPI_write(0x103, 0x0612);
    lms->SPI_write(0x108, 0x318C);
    lms->SPI_write(0x082, 0x8001);
    lms->SPI_write(0x200, 0x008D);
    lms->SPI_write(0x208, 0x01FB);
    lms->SPI_write(0x400, 0x8081);
    lms->SPI_write(0x40C, 0x01FF);
    lms->SPI_write(0x404, 0x0006);
    lms->LoadDC_REG_IQ(true, 0x3FFF, 0x3FFF);
    double srate = lms->GetSampleRate(false, LMS7002M::ChA);
    lms->SetFrequencySX(false,450e6);
    int dec = lms->Get_SPI_Reg_bits(LMS7_HBD_OVR_RXTSP);
    if (dec > 4) dec = 0;

    double offsets[] = {1.15/60.0, 1.1/40.0, 0.55/20.0, 0.2/10.0, 0.18/5.0};
    double tolerance[] = {0.9, 0.45, 0.25, 0.14, 0.06};
    double offset = offsets[dec]*srate/1e6;
    std::vector<uint32_t>  dataWr;
    dataWr.resize(16);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0x0008, 0x0100);
    fpga->WriteRegister(0x0007, 3);
    bool found = false;
    for (int i = 0; i < 200; i++){
        lms->Modify_SPI_Reg_bits(LMS7_PD_FDIV_O_CGEN, 1);
        lms->Modify_SPI_Reg_bits(LMS7_PD_FDIV_O_CGEN, 0);
        AlignRxTSP();

        lms->SetFrequencySX(true, 450e6+srate/16.0);
        double offset1 = GetPhaseOffset(32);
        if (offset1 < -360)
            break;
        lms->SetFrequencySX(true, 450e6+srate/8.0);
        double offset2 = GetPhaseOffset(64);
        if (offset2 < -360)
            break;
        double diff = offset1-offset2;
        if (abs(diff-offset) < tolerance[dec])
        {
            found = true;
            break;
        }
    }
    if (restoreValues)
        lms->RestoreRegisterMap(regBackup);
    if (found)
        AlignQuadrature(restoreValues);
    else
        lime::warning("Channel alignment failed");
    lms->SPI_write(0x20, reg20);
}

void Streamer::AlignQuadrature(bool restoreValues)
{
    auto regBackup = lms->BackupRegisterMap();

    lms->SPI_write(0x20, 0xFFFF);
    lms->SetDefaults(LMS7002M::RBB);
    lms->SetDefaults(LMS7002M::TBB);
    lms->SetDefaults(LMS7002M::TRF);
    lms->SPI_write(0x113, 0x0046);
    lms->SPI_write(0x118, 0x418C);
    lms->SPI_write(0x100, 0x4039);
    lms->SPI_write(0x101, 0x7801);
    lms->SPI_write(0x108, 0x318C);
    lms->SPI_write(0x082, 0x8001);
    lms->SPI_write(0x200, 0x008D);
    lms->SPI_write(0x208, 0x01FB);
    lms->SPI_write(0x400, 0x8081);
    lms->SPI_write(0x40C, 0x01FF);
    lms->SPI_write(0x404, 0x0006);
    lms->LoadDC_REG_IQ(true, 0x3FFF, 0x3FFF);
    lms->SPI_write(0x20, 0xFFFE);
    lms->SPI_write(0x105, 0x0006);
    lms->SPI_write(0x100, 0x4038);
    lms->SPI_write(0x113, 0x007F);
    lms->SPI_write(0x119, 0x529B);
    auto val = lms->Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE, true);
    lms->SPI_write(0x10D, val==3 ? 0x18F : val==2 ? 0x117 : 0x08F);
    lms->SPI_write(0x10C, val==2 ? 0x88C5 : 0x88A5);
    lms->SPI_write(0x20, 0xFFFD);
    lms->SPI_write(0x103, val==2 ? 0x612 : 0xA12);
    val = lms->Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE, true);
    lms->SPI_write(0x10D, val==3 ? 0x18F : val==2 ? 0x117 : 0x08F);
    lms->SPI_write(0x10C, val==2 ? 0x88C5 : 0x88A5);
    lms->SPI_write(0x119, 0x5293);
    double srate = lms->GetSampleRate(false, LMS7002M::ChA);
    double freq = lms->GetFrequencySX(false);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0x0008, 0x0100);
    fpga->WriteRegister(0x0007, 3);
    lms->SetFrequencySX(true, freq+srate/16.0);
    bool found = false;
    for (int i = 0; i < 100; i++){

        double offset = GetPhaseOffset(32);
        if (offset < -360)
            break;
        if (fabs(offset) <= 90.0)
        {
            found = true;
            break;
        }
        RstRxIQGen();
    }

    if (restoreValues)
        lms->RestoreRegisterMap(regBackup);
    if (!found)
        lime::warning("Channel alignment failed");
}

int Streamer::UpdateThreads(bool stopAll)
{
    bool needTx = false;
    bool needRx = false;

    //check which threads are needed
    if (!stopAll)
    {
        for(auto &i : mRxStreams)
            if(i.used && i.IsActive())
            {
                needRx = true;
                break;
            }
        for(auto &i : mTxStreams)
            if(i.used && i.IsActive())
            {
                needTx = true;
                break;
            }
    }

    //stop threads if not needed
    if((!needTx) && txThread.joinable())
    {
        terminateTx.store(true, std::memory_order_relaxed);
        txThread.join();
    }
    if((!needRx) && rxThread.joinable())
    {
        terminateRx.store(true, std::memory_order_relaxed);
        rxThread.join();
    }

    //configure FPGA on first start, or disable FPGA when not streaming
    if((needTx || needRx) && (!txThread.joinable()) && (!rxThread.joinable()))
    {
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        if (mRxStreams[0].used && mRxStreams[1].used)
            AlignRxRF(true);
        //enable FPGA streaming
        fpga->StopStreaming();
        fpga->ResetTimestamp();
        rxLastTimestamp.store(0, std::memory_order_relaxed);
        //Clear device stream buffers
        dataPort->ResetStreamBuffers();

        //enable MIMO mode, 12 bit compressed values
        dataLinkFormat = StreamConfig::FMT_INT12;
        //by default use 12 bit compressed, adjust link format for stream

        for(auto &i : mRxStreams)
            if(i.used && i.config.format != StreamConfig::FMT_INT12)
            {
                dataLinkFormat = StreamConfig::FMT_INT16;
                break;
            }

        for(auto &i : mTxStreams)
            if(i.used && i.config.format != StreamConfig::FMT_INT12)
            {
                dataLinkFormat = StreamConfig::FMT_INT16;
                break;
            }

        for(auto &i : mRxStreams)
            if (i.used)
                i.config.linkFormat = dataLinkFormat;
        for(auto &i : mTxStreams)
            if (i.used)
                i.config.linkFormat = dataLinkFormat;

        const uint16_t smpl_width = dataLinkFormat == StreamConfig::FMT_INT12 ? 2 : 0;
        uint16_t mode = 0x0100;

        if (lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)))
            mode = 0x0040;
        else if (lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)))
            mode = 0x0180;

        fpga->WriteRegister(0x0008, mode | smpl_width);

        const uint16_t channelEnables = (mRxStreams[0].used||mTxStreams[0].used) + 2 * (mRxStreams[1].used||mTxStreams[1].used);
        fpga->WriteRegister(0x0007, channelEnables);

        uint32_t reg9 = fpga->ReadRegister(0x0009);
        const uint32_t addr[] = {0x0009, 0x0009};
        const uint32_t data[] = {reg9 | (5 << 1), reg9 & ~(5 << 1)};
        fpga->StartStreaming();
        fpga->WriteRegisters(addr, data, 2);
    }
    else if(not needTx and not needRx)
    {
        //disable FPGA streaming
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->StopStreaming();
    }

    //FPGA should be configured and activated, start needed threads
    if(needRx && (!rxThread.joinable()))
    {
        terminateRx.store(false, std::memory_order_relaxed);
        auto RxLoopFunction = std::bind(&Streamer::ReceivePacketsLoop, this);
        rxThread = std::thread(RxLoopFunction);
    }
    if(needTx && (!txThread.joinable()))
    {
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->WriteRegister(0xD, 0); //stop WFM
        terminateTx.store(false, std::memory_order_relaxed);
        auto TxLoopFunction = std::bind(&Streamer::TransmitPacketsLoop, this);
        txThread = std::thread(TxLoopFunction);
    }
    return 0;
}

void Streamer::TransmitPacketsLoop()
{
    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = streamSize;
    const bool packed = dataLinkFormat == StreamConfig::FMT_INT12;
    const int epIndex = chipId;
    const uint8_t buffersCount = dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch*sizeof(FPGA_DataPacket);
    const uint32_t popTimeout_ms = 500;

    const int maxSamplesBatch = (packed ? samples12InPkt:samples16InPkt)/chCount;
    std::vector<int> handles(buffersCount, 0);
    std::vector<bool> bufferUsed(buffersCount, 0);
    std::vector<uint32_t> bytesToSend(buffersCount, 0);
    std::vector<complex16_t> samples[maxChannelCount];
    std::vector<char> buffers;
    try
    {
        for(int i=0; i<chCount; ++i)
            samples[i].resize(maxSamplesBatch);
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc& ex) //not enough memory for buffers
    {
        return lime::error("Error allocating Tx buffers, not enough memory");
    }

    long totalBytesSent = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    bool end_burst = false;
    uint8_t bi = 0; //buffer index
    while (terminateTx.load(std::memory_order_relaxed) != true)
    {
        if (bufferUsed[bi])
        {
            if (dataPort->WaitForSending(handles[bi], 1000) == true)
            {
                unsigned bytesSent = dataPort->FinishDataSending(&buffers[bi*bufferSize], bytesToSend[bi], handles[bi]);

                if (bytesSent != bytesToSend[bi])
                {
                    for (auto &value : mTxStreams)
                        if (value.used && value.mActive)
                            value.overflow++;
                }
                else
                    totalBytesSent += bytesSent;
                bufferUsed[bi] = false;
            }
            else
            {
                txDataRate_Bps.store(totalBytesSent, std::memory_order_relaxed);
                totalBytesSent = 0;
                continue;
            }
        }
        bytesToSend[bi] = 0;
        FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(&buffers[bi*bufferSize]);
        int i=0;
        do
        {
            bool has_samples = false;
            int payloadSize = sizeof(FPGA_DataPacket::data);
            StreamChannel::Metadata meta = {0, 0};
            for(int ch=0; ch<maxChannelCount; ++ch)
            {
                if (!mTxStreams[ch].used)
                    continue;
                const int ind = chCount == maxChannelCount ? ch : 0;
                if (mTxStreams[ch].mActive==false)
                {
                    memset(&samples[ind][0],0,maxSamplesBatch*sizeof(complex16_t));
                    continue;
                }
                int samplesPopped = mTxStreams[ch].Read(samples[ind].data(), maxSamplesBatch, &meta, popTimeout_ms);
                if (samplesPopped != maxSamplesBatch)
                {
                    if ((!end_burst) && !(meta.flags & RingFIFO::END_BURST))
                    {
                        mTxStreams[ch].underflow++;
                        lime::warning("popping from TX, samples popped %i/%i", samplesPopped, maxSamplesBatch);
                        continue;
                    }
                    payloadSize = samplesPopped * sizeof(FPGA_DataPacket::data) / maxSamplesBatch;
                    int q = packed ? 48 : 16;
                    payloadSize = (1 + (payloadSize - 1) / q) * q;
                    memset(&samples[ind][samplesPopped], 0, (maxSamplesBatch - samplesPopped)*sizeof(complex16_t));
                }
                has_samples = true;
            }

            if (!has_samples)
                break;

            end_burst = (meta.flags & RingFIFO::END_BURST);
            pkt[i].counter = meta.timestamp;
            pkt[i].reserved[0] = 0;
            //by default ignore timestamps
            const int ignoreTimestamp = !(meta.flags & RingFIFO::SYNC_TIMESTAMP);
            pkt[i].reserved[0] |= ((int)ignoreTimestamp << 4); //ignore timestamp
            pkt[i].reserved[1] = payloadSize & 0xFF;
            pkt[i].reserved[2] = (payloadSize >> 8) & 0xFF;
            std::vector<complex16_t*> src(chCount);
            for(uint8_t c=0; c<chCount; ++c)
                src[c] = (samples[c].data());
            uint8_t* const dataStart = (uint8_t*)pkt[i].data;
            FPGA::Samples2FPGAPacketPayload(src.data(), maxSamplesBatch, chCount==2, packed, dataStart);
            bytesToSend[bi] += 16+payloadSize;
        }while(++i<packetsToBatch && end_burst == false);

        if(terminateTx.load(std::memory_order_relaxed) == true) //early termination
            break;

        if (i)
        {
            handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi], epIndex);
            txLastTimestamp.store(pkt[i-1].counter+maxSamplesBatch-1, std::memory_order_relaxed); //timestamp of the last sample that was sent to HW
            bufferUsed[bi] = true;
            bi = (bi + 1) & (buffersCount-1);
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            float dataRate = 1000.0*totalBytesSent / timePeriod;
            txDataRate_Bps.store(dataRate, std::memory_order_relaxed);
            totalBytesSent = 0;
            t1 = t2;
#ifndef NDEBUG
            lime::log(LOG_LEVEL_DEBUG, "Tx: %.3f MB/s\n", dataRate / 1000000.0);
#endif
        }
    }

    // Wait for all the queued requests to be cancelled
    dataPort->AbortSending(epIndex);
    txDataRate_Bps.store(0, std::memory_order_relaxed);
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void Streamer::ReceivePacketsLoop()
{
    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = streamSize;
    const bool packed = dataLinkFormat == StreamConfig::FMT_INT12;
    const uint32_t samplesInPacket = (packed  ? samples12InPkt : samples16InPkt)/chCount;

    const int epIndex = chipId;
    const uint8_t buffersCount = dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch*sizeof(FPGA_DataPacket);
    std::vector<int> handles(buffersCount, 0);
    std::vector<char>buffers(buffersCount*bufferSize, 0);
    std::vector<StreamChannel::Frame> chFrames;
    try
    {
        chFrames.resize(chCount);
    }
    catch (const std::bad_alloc &ex)
    {
        lime::error("Error allocating Rx buffers, not enough memory");
        return;
    }

    for (int i = 0; i<buffersCount; ++i)
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize, epIndex);

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

    int resetFlagsDelay = 0;
    uint64_t prevTs = 0;
    while (terminateRx.load(std::memory_order_relaxed) == false)
    {
        int32_t bytesReceived = 0;
        if(handles[bi] >= 0)
        {
            if (dataPort->WaitForReading(handles[bi], 1000) == true)
            {
                bytesReceived = dataPort->FinishDataReading(&buffers[bi*bufferSize], bufferSize, handles[bi]);
                totalBytesReceived += bytesReceived;
                if (bytesReceived != int32_t(bufferSize)) //data should come in full sized packets
                    for(auto &value: mRxStreams)
                        if (value.used && value.mActive)
                            value.underflow++;
            }
            else
            {
                rxDataRate_Bps.store(totalBytesReceived, std::memory_order_relaxed);
                totalBytesReceived = 0;
                continue;
            }
        }
        for (uint8_t pktIndex = 0; pktIndex < bytesReceived / sizeof(FPGA_DataPacket); ++pktIndex)
        {
            const FPGA_DataPacket* pkt = (FPGA_DataPacket*)&buffers[bi*bufferSize];
            const uint8_t byte0 = pkt[pktIndex].reserved[0];
            if ((byte0 & (1 << 3)) != 0)
            {
                if(resetFlagsDelay > 0)
                    --resetFlagsDelay;
                else
                {
                    lime::warning("L");
                    resetFlagsDelay = buffersCount*2;
                }
                for(auto &value: mTxStreams)
                    if (value.used && value.mActive)
                        value.pktLost++;
            }
            uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
            if(pkt[pktIndex].counter - prevTs != samplesInPacket && pkt[pktIndex].counter != prevTs)
            {
                int packetLoss = ((pkt[pktIndex].counter - prevTs)/samplesInPacket)-1;
                for(auto &value: mRxStreams)
                    if (value.used && value.mActive)
                        value.pktLost += packetLoss;
            }
            prevTs = pkt[pktIndex].counter;
            rxLastTimestamp.store(prevTs, std::memory_order_relaxed);
            //parse samples
            std::vector<complex16_t*> dest(chCount);
            for(uint8_t c=0; c<chCount; ++c)
                dest[c] = (chFrames[c].samples);
            int samplesCount = FPGA::FPGAPacketPayload2Samples(pktStart, 4080, chCount==2, packed, dest.data());

            for(int ch=0; ch<maxChannelCount; ++ch)
            {
                if (mRxStreams[ch].used==false || mRxStreams[ch].mActive==false)
                    continue;
                const int ind = chCount == maxChannelCount ? ch : 0;
                StreamChannel::Metadata meta;
                meta.timestamp = pkt[pktIndex].counter;
                meta.flags = RingFIFO::OVERWRITE_OLD | RingFIFO::SYNC_TIMESTAMP;
                int samplesPushed = mRxStreams[ch].Write((const void*)chFrames[ind].samples, samplesCount, &meta, 100);
                if(samplesPushed != samplesCount)
                    mRxStreams[ch].overflow++;
            }
        }
        // Re-submit this request to keep the queue full
        handles[bi] = dataPort->BeginDataReading(&buffers[bi*bufferSize], bufferSize, epIndex);
        bi = (bi + 1) & (buffersCount-1);

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            t1 = t2;
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
#ifndef NDEBUG
            lime::log(LOG_LEVEL_DEBUG, "Rx: %.3f MB/s\n", dataRate / 1000000.0);
#endif
            totalBytesReceived = 0;
            rxDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
        }
    }
    dataPort->AbortReading(epIndex);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
}

}
