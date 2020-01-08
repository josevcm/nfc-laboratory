#include <QFile>
#include <QTextStream>

#include "FrameCapture.h"

FrameCapture::FrameCapture(const QString &name, FrameCapture::Mode mode, QObject *parent) : FrameStorage(parent), mMode(mode)
{
   mDevice = new QFile(name);
}

FrameCapture::~FrameCapture()
{
   close();

   if (mDevice)
   {
      mDevice->deleteLater();
   }
}

bool FrameCapture::open()
{
   close();

   if (mMode == Write)
   {
      if (mDevice->open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
         return writeHeader();
      }
   }
   else if (mMode == Read)
   {
      if (mDevice->open(QIODevice::ReadOnly))
      {
         return readHeader();
      }
   }

   return false;
}

void FrameCapture::close()
{
   if (mDevice)
   {
      if (mMode == Write)
      {
         mWriter.writeEndElement();
         mWriter.writeEndDocument();
      }

      mDevice->close();
   }
}

bool FrameCapture::readHeader()
{
   mReader.setDevice(mDevice);

   if(mReader.readNextStartElement())
   {
      if (mReader.name() == "capture")
      {
         if (mReader.attributes().hasAttribute("source"))
            mSource = mReader.attributes().value("source").toString();

         if (mReader.attributes().hasAttribute("frequency"))
            mFrequency = mReader.attributes().value("frequency").toLong();

         if (mReader.attributes().hasAttribute("sampling"))
            mSampling = mReader.attributes().value("sampling").toLong();

         return true;
      }
   }

   return false;
}

bool FrameCapture::writeHeader()
{
   mWriter.setDevice(mDevice);
   mWriter.setAutoFormatting(true);
   mWriter.writeStartDocument();
   mWriter.writeStartElement("capture");
   mWriter.writeAttribute("device", mSource);
   mWriter.writeAttribute("frequency", QString("%1").arg(mFrequency));
   mWriter.writeAttribute("sampling", QString("%1").arg(mSampling));

   return true;
}

bool FrameCapture::readFrame(NfcFrame frame)
{
   if(mReader.readNextStartElement())
   {
      if (mReader.name() == "frame")
      {
         QXmlStreamAttributes attr = mReader.attributes();

         if (attr.hasAttribute("tech") && attr.hasAttribute("type") && attr.hasAttribute("flags"))
         {
            int tech = attr.value("tech").toInt();
            int type = attr.value("type").toInt();
            int flags = attr.value("flags").toInt();

            frame = NfcFrame(tech, type, flags);

            if (attr.hasAttribute("start"))
               frame.setSampleStart(mReader.attributes().value("start").toLong());

            if (attr.hasAttribute("end"))
               frame.setSampleEnd(mReader.attributes().value("end").toLong());

            if (attr.hasAttribute("stage"))
               frame.setFramePhase(mReader.attributes().value("stage").toInt());

            QStringRef data = mReader.text();

            for (int i = 0; i < data.length(); i += 2)
            {
               bool ok = false;

               if (data.length() >= i+2)
               {
                  int value = data.mid(i, i + 2).toInt(&ok, 16);

                  if (ok)
                  {
                     frame.put(value);
                  }
               }
            }

            return true;
         }
      }
   }

   return false;
}

bool FrameCapture::writeFrame(NfcFrame frame)
{
   mWriter.writeStartElement("frame");
   mWriter.writeAttribute("start", QString("%1").arg(frame.sampleStart()));
   mWriter.writeAttribute("end", QString("%1").arg(frame.sampleEnd()));
   mWriter.writeAttribute("tech", QString("%1").arg(frame.techType()));
   mWriter.writeAttribute("type", QString("%1").arg(frame.frameType()));
   mWriter.writeAttribute("flags", QString("%1").arg(frame.frameFlags()));
   mWriter.writeAttribute("stage", QString("%1").arg(frame.framePhase()));
   mWriter.writeCharacters(toHexString(frame.toByteArray()));
   mWriter.writeEndElement();

   return true;
}

bool FrameCapture::isOpen() const
{
   return mDevice->isOpen();
}

const QString &FrameCapture::source() const
{
   return mSource;
}

void FrameCapture::setSource(const QString &source)
{
   mSource = source;
}

long FrameCapture::frequency() const
{
   return mFrequency;
}

void FrameCapture::setFrequency(long frequency)
{
   mFrequency = frequency;
}

long FrameCapture::sampling() const
{
   return mSampling;
}

void FrameCapture::setSampling(long sampling)
{
   mSampling = sampling;
}

void FrameCapture::append(NfcFrame frame)
{
   if (mMode == Write && mDevice->isOpen())
   {
      writeFrame(frame);
   }

   FrameStorage::append(frame);
}

NfcFrame FrameCapture::at(int index) const
{
   if (mMode == Read && index < length())
   {
      NfcFrame frame;

      FrameCapture *self = const_cast<FrameCapture*>(this);

      while(((index + 1) < length()) && self->readFrame(frame))
      {
         self->append(frame);
      }
   }

   return FrameStorage::at(index);
}

QString FrameCapture::toHexString(const QByteArray &value) const
{
   QString text;

   for (QByteArray::ConstIterator it = value.begin(); it != value.end(); it++)
   {
      text.append(QString("%1").arg(*it & 0xff, 2, 16, QLatin1Char('0')));
   }

   return text;
}

QByteArray FrameCapture::toByteArray(const QString &value) const
{
   QByteArray data;

   return data;
}

