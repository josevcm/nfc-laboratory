/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef NFCSERVICE_H
#define NFCSERVICE_H

#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QMutex>

class NfcStream;
class SignalDevice;
class ProtocolParser;
class LiveViewStatusEvent;
class DecoderControlEvent;
class GainControlEvent;
class StreamFrameEvent;

class NfcService: public QObject
{
		Q_OBJECT

		enum Status
		{
          Stopped = 0, Decoding = 1, Capture = 2
		};

	protected:

      void decoderControlEvent(DecoderControlEvent *event);
      void gainControlEvent(GainControlEvent *event);

      void decodeStart(const QString &source);
      void decodeRecord(const QString &source);
      void decodeStop();

      void scannerHandler();
      void decoderHandler();
      void captureHandler();

	public:

      NfcService(QSettings &settings, NfcStream *stream);
      virtual ~NfcService();

      void searchDevices();

      void customEvent(QEvent * event);

   private:

      // configuration
      QSettings &mSettings;

      // underline frame stream
      NfcStream *mStream;

      // device list
      QStringList mDeviceList;

      // frequency list
      QStringList mFrequencyList;

      // signal source
      QString mSignalSource;

      // signal recorder
      QString mSignalRecord;

      // recorder channels
      int mRecordChannels;

      // device check mutex
      QMutex mScannerMutex;

      // decoder mutex
      QMutex mServiceMutex;

      // decoder working status
      QAtomicInteger<int> mServiceStatus;

      // check working status
      QAtomicInteger<int> mScannerStatus;

      // selected frequency
      QAtomicInteger<int> mFrequency;

      // selected samplerate
      QAtomicInteger<int> mSampleRate;

      // selected gain
      QAtomicInteger<int> mTunerGain;

      // capture time limit
      QAtomicInteger<int> mTimeLimit;
};

#endif /* NFCSERVICE_H */
