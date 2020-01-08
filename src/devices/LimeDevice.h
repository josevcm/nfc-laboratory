#ifndef LIB_SIO_LIMEDEVICE_H
#define LIB_SIO_LIMEDEVICE_H

#include <QMutex>
#include <QIODevice>

#include "RadioDevice.h"

class LimeDevice: public RadioDevice
{
   public:

      static const int SampleSize = 16;

   public:

      LimeDevice(QObject *parent = nullptr);
      LimeDevice(const QString &name, QObject *parent = nullptr);

      virtual ~LimeDevice();

      static QStringList listDevices();

   public:
      // from QIODevice

      void close();

   public:
      // from RadioDevice

      const QString &name();

      bool open(OpenMode mode);

      bool open(const QString &name, OpenMode mode);

      int sampleSize() const;

      void setSampleSize(int sampleSize);

      long sampleRate() const;

      void setSampleRate(long sampleRate);

      int sampleType() const;

      void setSampleType(int sampleType);

      long centerFrequency() const;

      void setCenterFrequency(long frequency);

      int agcMode() const;

      void setAgcMode(int gainMode);

      float receiverGain() const;

      void setReceiverGain(float tunerGain);

      QVector<int> supportedSampleRates() const;

      QVector<float> supportedReceiverGains() const;

      bool waitForReadyRead(int msecs);

      int read(SampleBuffer<float> signal);

      int write(SampleBuffer<float> signal);

      using SignalDevice::read;

      using SignalDevice::write;

   protected:

      // from QIODevice
      qint64 readData(char* data, qint64 maxSize);

      qint64 writeData(const char* data, qint64 maxSize);

   private:

      // device control
      void *mDevice;
      void *mStream;
      QString mName;
      long mSampleRate;
      long mCenterFrequency;
      float mTunerGain;
      float mBandWidth;
      int mGainMode;
      QMutex mMutex;
};

#endif // LIB_SIO_LIMEDEVICE_H
