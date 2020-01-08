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

#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QObject>
#include <QSettings>

#include <decoder/NfcFrame.h>

class ProtocolFrame;

class ProtocolParser: public QObject
{
		Q_OBJECT

      enum CrcType
      {
         A = 1, B = 2
      };

	private:

      int mCount;
      int mChaining;
      int mRequest;

      NfcFrame mLast;

	private:

      // request frames PDC->PICC
      ProtocolFrame *parseRequestREQA(const NfcFrame &frame); // Request Command, Type A
      ProtocolFrame *parseRequestHLTA(const NfcFrame &frame); // HALT Command, Type A
      ProtocolFrame *parseRequestWUPA(const NfcFrame &frame); // Wake-UP Command, Type A
      ProtocolFrame *parseRequestSELn(const NfcFrame &frame); // Select Command, Type A
      ProtocolFrame *parseRequestRATS(const NfcFrame &frame); // Request for Answer to Select
      ProtocolFrame *parseRequestPPSr(const NfcFrame &frame); // Protocol Parameter Selection
      ProtocolFrame *parseRequestAUTH(const NfcFrame &frame); // Mifare Classic Authentication
      ProtocolFrame *parseRequestIBlock(const NfcFrame &frame); // I-block
      ProtocolFrame *parseRequestRBlock(const NfcFrame &frame); // R-block
      ProtocolFrame *parseRequestSBlock(const NfcFrame &frame); // S-block
      ProtocolFrame *parseRequestUnknow(const NfcFrame &frame); // no know command

      // response frames PICC->PDC
      ProtocolFrame *parseResponseREQA(const NfcFrame &frame); // Request Command, Type A
      ProtocolFrame *parseResponseHLTA(const NfcFrame &frame); // HALT Command, Type A
      ProtocolFrame *parseResponseWUPA(const NfcFrame &frame); // Wake-UP Command, Type A
      ProtocolFrame *parseResponseSELn(const NfcFrame &frame); // Select Command, Type A
      ProtocolFrame *parseResponseRATS(const NfcFrame &frame); // Request for Answer to Select
      ProtocolFrame *parseResponsePPSr(const NfcFrame &frame); // Protocol Parameter Selection
      ProtocolFrame *parseResponseAUTH(const NfcFrame &frame); // Mifare Classic Authentication
      ProtocolFrame *parseResponseIBlock(const NfcFrame &frame); // I-block
      ProtocolFrame *parseResponseRBlock(const NfcFrame &frame); // R-block
      ProtocolFrame *parseResponseSBlock(const NfcFrame &frame); // S-block
      ProtocolFrame *parseResponseUnknow(const NfcFrame &frame); // no know command

      bool isApdu(const QByteArray &apdu);

      ProtocolFrame *parseAPDU(const QString &name, const QByteArray &apdu); // no know command

      ProtocolFrame *buildFrameInfo(int rate, QVariant info, double time, double end, int flags, int type);

      ProtocolFrame *buildFrameInfo(const QString &name, int rate, QVariant info, double time, double end, int flags, int type);

      ProtocolFrame *buildFieldInfo(const QString &name, QVariant info);

      ProtocolFrame *buildFieldInfo(QVariant info);

      ProtocolFrame *buildInfo(const QString &name, int rate, QVariant info, double time, double end, float diff, int flags, int type);

      bool verifyCrc(const QByteArray &data, int type = CrcType::A);

	public:

      ProtocolParser(QObject *parent = nullptr);

      virtual ~ProtocolParser();

      ProtocolFrame *parse(const NfcFrame &frame);

      void reset();
};

#endif /* PROTOCOLPARSER_H */
