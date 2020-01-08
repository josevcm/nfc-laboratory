#ifndef FILEFRAMESTORAGE_H
#define FILEFRAMESTORAGE_H

#include <QPointer>
#include <QIODevice>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include <storage/FrameStorage.h>

class FrameCapture: public FrameStorage
{
   public:

      enum Mode { Read = 1, Write = 2 };

   public:

      FrameCapture(const QString &file, Mode mode, QObject *parent = nullptr);
      virtual ~FrameCapture();

      bool open();
      void close();
      bool isOpen() const;

      const QString &source() const;
      void setSource(const QString &source);

      long frequency() const;
      void setFrequency(long frequency);

      long sampling() const;
      void setSampling(long sampling);

      void append(NfcFrame frame);
      NfcFrame at(int index) const;

   protected:

      bool readHeader();
      bool writeHeader();

      bool readFrame(NfcFrame frame);
      bool writeFrame(NfcFrame frame);

      QString toHexString(const QByteArray &value) const;
      QByteArray toByteArray(const QString &value) const;

   private:

      Mode mMode;
      QIODevice *mDevice;
      QXmlStreamWriter mWriter;
      QXmlStreamReader mReader;

      QString mSource;
      long mFrequency;
      long mSampling;
};

#endif // FILEFRAMESTORAGE_H
