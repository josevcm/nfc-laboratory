#ifndef FRAMESTORAGE_H
#define FRAMESTORAGE_H

#include <QObject>
#include <QList>
#include <decoder/NfcFrame.h>

class FrameStorage : public QObject
{
   public:

      enum Mode { Read = 1, Write = 2 };

   public:

      FrameStorage(const QString &file, Mode mode, QObject *parent = nullptr);
      virtual ~FrameStorage();

      bool open();
      void close();
      bool isOpen() const;

      const QString &source() const;
      void setSource(const QString &source);

      long frequency() const;
      void setFrequency(long frequency);

      long sampling() const;
      void setSampling(long sampling);

      bool readFrames(NfcStream *stream);
      bool writeFrames(NfcStream *stream);

   protected:

      bool readHeader();
      bool writeHeader();

};

#endif // FRAMESTORAGE_H
