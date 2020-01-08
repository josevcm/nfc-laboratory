#include <QDebug>
#include <QThread>

#include <support/lime/LimeSuite.h>

#include "LimeDevice.h"

typedef lms_stream_t* lms_stream;
typedef lms_device_t* lms_device;

LimeDevice::LimeDevice(QObject *parent) :
   LimeDevice("", parent)
{
}

LimeDevice::LimeDevice(const QString &name, QObject *parent) :
   RadioDevice(parent), mDevice(nullptr), mName(name), mSampleRate(-1), mCenterFrequency(-1), mTunerGain(-1), mBandWidth(8e6), mGainMode(0)
{
   qDebug() << "created LimeDevice" << mName;
}

LimeDevice::~LimeDevice()
{
   qDebug() << "destroy LimeDevice" << mName;

   close();
}

QStringList LimeDevice::listDevices()
{
   QStringList result;

   lms_info_str_t list[16];

   int count = LMS_GetDeviceList(list);

   for (int i = 0; i < count; i++)
   {
      result.append(QString("lime://%1").arg(list[i]));
   }

   return result;
}

const QString &LimeDevice::name()
{
   return mName;
}

bool LimeDevice::open(OpenMode mode)
{
   return open(mName, mode);
}

bool LimeDevice::open(const QString &name, OpenMode mode)
{
   void *device;

   close();

   // check device name
   if (!name.startsWith("lime://"))
   {
      qWarning() << "invalid device name" << name;
      return false;
   }

   QString id = name.mid(7);

   if (LMS_Open(&device, id.toLocal8Bit().data(), nullptr) == LMS_SUCCESS)
   {
      qInfo() << "open device" << name;

      // initialize device with default configuration
      if (LMS_Init(lms_device(device)) != LMS_SUCCESS)
         qWarning() << "failed LMS_Init:" << LMS_GetLastErrorMessage();

      //Enable RX channel
      if (LMS_EnableChannel(lms_device(device), LMS_CH_RX, 0, true) != LMS_SUCCESS)
         qWarning() << "failed LMS_EnableChannel:" << LMS_GetLastErrorMessage();

      // configure frequency
      if (mCenterFrequency != -1)
      {
         qInfo() << "set frequency to" << mCenterFrequency << "Hz";

         if (LMS_SetLOFrequency(lms_device(device), LMS_CH_RX, 0, mCenterFrequency) != LMS_SUCCESS)
            qWarning() << "failed LMS_SetLOFrequency:" << LMS_GetLastErrorMessage();
      }

      // configure sample rate
      if (mSampleRate != -1)
      {
         qInfo() << "set samplerate to" << mSampleRate;

         if (LMS_SetSampleRate(lms_device(device), mSampleRate, 1) != LMS_SUCCESS)
            qWarning() << "failed LMS_SetSampleRate:" << LMS_GetLastErrorMessage();
      }

      // configure gain
      if (mTunerGain >= 0)
      {
         qInfo() << "set normalized gain to" << mSampleRate;

         if (LMS_SetNormalizedGain(lms_device(device), LMS_CH_RX, 0, float_type(mTunerGain / 100)) != LMS_SUCCESS)
            qWarning() << "failed LMS_SetNormalizedGain:" << LMS_GetLastErrorMessage();
      }

      //Configure LPF bandwidth
      if (LMS_SetLPFBW(lms_device(device), LMS_CH_RX, 0, double(mBandWidth)) != LMS_SUCCESS)
         qWarning() << "failed LMS_SetLPFBW:" << LMS_GetLastErrorMessage();

      // perform calibration
      if (LMS_Calibrate(lms_device(device), LMS_CH_RX, 0, double(mBandWidth), 0) != LMS_SUCCESS)
         qWarning() << "failed LMS_Calibrate:" << LMS_GetLastErrorMessage();


      // enable test signal
//      if (LMS_SetTestSignal(lms_device(device), LMS_CH_RX, 0, LMS_TESTSIG_NCODIV8, 0, 0) != LMS_SUCCESS)
//         qWarning() << "failed LMS_SetTestSignal:" << LMS_GetLastErrorMessage();

      // initialize stream
      lms_stream stream = new lms_stream_t;

      stream->channel = 0; //channel number
      stream->fifoSize = 1024 * 1024; //fifo size in samples
      stream->throughputVsLatency = 1.0; //optimize for max throughput
      stream->dataFmt = lms_stream_t::LMS_FMT_F32; // 32-bit floating point
      stream->isTx = false; //RX channel

      // setup stream
      if (LMS_SetupStream(lms_device(device), lms_stream(stream)) != LMS_SUCCESS)
         qWarning() << "failed LMS_SetupStream:" << LMS_GetLastErrorMessage();

      // enable stream
      if (LMS_StartStream(lms_stream(stream)) != LMS_SUCCESS)
         qWarning() << "failed LMS_StartStream:" << LMS_GetLastErrorMessage();

      mName = name;
      mDevice = device;
      mStream = stream;
   }
   else
   {
      qWarning() << "failed LMS_Open:" << LMS_GetLastErrorMessage();
      return false;
   }

   return QIODevice::open(mode);
}

void LimeDevice::close()
{
   if (mDevice)
   {
      qInfo() << "close device" << mName;

      // stop stream
      if (LMS_StopStream(lms_stream(mStream)) != LMS_SUCCESS)
         qWarning() << "failed LMS_StopStream:" << LMS_GetLastErrorMessage();

      // destroy stream
      if (LMS_DestroyStream(lms_device(mDevice), lms_stream(mStream)) != LMS_SUCCESS)
         qWarning() << "failed LMS_DestroyStream:" << LMS_GetLastErrorMessage();

      // close device
      if (LMS_Close(lms_device(mDevice)) != LMS_SUCCESS)
         qWarning() << "failed LMS_Close:" << LMS_GetLastErrorMessage();

      mDevice = nullptr;
      mStream = nullptr;
      mName = "";
   }

   QIODevice::close();
}

int LimeDevice::sampleSize() const
{
   return LimeDevice::SampleSize;
}

void LimeDevice::setSampleSize(int sampleSize)
{
   qWarning() << "setSampleSize has no effect!";
}

long LimeDevice::sampleRate() const
{
   return mSampleRate;
}

void LimeDevice::setSampleRate(long sampleRate)
{
   mSampleRate = sampleRate;

   if (mDevice)
   {
      /*
       * Set sampling rate for all RX/TX channels. Sample rate is in complex samples (1 sample = I + Q).
       * The function sets sampling rate that is used for data exchange with the host.
       * It also allows to specify higher sampling rate to be used in RF by setting oversampling ratio.
       * Valid oversampling values are 1, 2, 4, 8, 16, 32 or 0 (use device default oversampling value).
       */

      if (LMS_SetSampleRate(lms_device(mDevice), mSampleRate, 2) != LMS_SUCCESS)
         qWarning() << "failed LMS_SetSampleRate:" << LMS_GetLastErrorMessage();
   }
}

int LimeDevice::sampleType() const
{
   return INTEGER;
}

void LimeDevice::setSampleType(int sampleType)
{
   qWarning() << "setSampleType has no effect!";
}

long LimeDevice::centerFrequency() const
{
   return mCenterFrequency;
}

void LimeDevice::setCenterFrequency(long frequency)
{
   mCenterFrequency = frequency;

   if (mDevice)
   {
      if (LMS_SetLOFrequency(lms_device(mDevice), LMS_CH_RX, 0, mCenterFrequency) != LMS_SUCCESS)
         qWarning() << "failed LMS_SetLOFrequency:" << LMS_GetLastErrorMessage();
   }
}

int LimeDevice::agcMode() const
{
   return mGainMode;
}

void LimeDevice::setAgcMode(int gainMode)
{
   qWarning() << "setAgcMode has no effect!";

   mGainMode = gainMode;
}

float LimeDevice::receiverGain() const
{
   return mTunerGain;
}

void LimeDevice::setReceiverGain(float tunerGain)
{
   mTunerGain = tunerGain;

   if (mDevice)
   {
      if (LMS_SetNormalizedGain(lms_device(mDevice), LMS_CH_RX, 0, float_type(mTunerGain / 100)) != LMS_SUCCESS)
         qWarning() << "failed LMS_SetNormalizedGain:" << LMS_GetLastErrorMessage();
   }
}

QVector<int> LimeDevice::supportedSampleRates() const
{
   QVector<int> result;

   lms_range_t range;

   if (mDevice)
   {
      if (LMS_GetSampleRateRange(lms_device(mDevice), LMS_CH_RX, &range) == LMS_SUCCESS)
      {
         for (float_type value = range.min; value <= range.max; value += 1E5)
         {
            result.append(int(value));
         }
      }
   }

   return result;
}

QVector<float> LimeDevice::supportedReceiverGains() const
{
   QVector<float> result;

   for (int i = 0; i < 100; i++)
   {
      result.append(i);
   }

   return result;
}

qint64 LimeDevice::readData(char* data, qint64 maxSize)
{
   int samples = LMS_RecvStream(lms_stream(mStream), data, size_t(maxSize / 4), nullptr, 100);

   return samples * sizeof(float) * 2;
}

qint64 LimeDevice::writeData(const char* data, qint64 maxSize)
{
   qWarning() << "writeData not supported on this device!";

   return -1;
}

bool LimeDevice::waitForReadyRead(int msecs)
{
   if (!isOpen())
      return false;

   return true;
}

int LimeDevice::read(SampleBuffer<float> signal)
{
   int samples = LMS_RecvStream(lms_stream(mStream), signal.data(), size_t(signal.available()), nullptr, 100);

   signal.wrap(samples).flip();

   return signal.limit();
}

int LimeDevice::write(SampleBuffer<float> signal)
{
   qWarning() << "write not supported on this device!";

   return -1;
}

