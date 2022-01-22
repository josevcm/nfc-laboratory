/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#ifndef APP_PROTOCOLFRAME_H
#define APP_PROTOCOLFRAME_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QVariant>
#include <QSharedPointer>

#include <nfc/NfcFrame.h>

class ProtocolFrame : public QObject
{
      struct Impl;

   public:

      enum Flags
      {
         RequestFrame = 0x0001,
         ResponseFrame = 0x0002,
         SenseFrame = 0x0004,
         SelectionFrame = 0x0008,
         ApplicationFrame = 0x0010,
         AuthFrame = 0x0020,
         FrameField = 0x040,
         FieldInfo = 0x0080,
         ParityError = 0x0100,
         CrcError = 0x0200,
         SyncError = 0x0200
      };

      enum Fields
      {
         Name = 0,
         Flags = 1,
         Data = 2
      };

   public:

      ProtocolFrame(const QVector<QVariant> &data, int flags, const nfc::NfcFrame &frame);

      ProtocolFrame(const QVector<QVariant> &data, int flags, ProtocolFrame *parent, int start = -1, int end = -1);

      ~ProtocolFrame() override;

      void clearChilds();

      ProtocolFrame *child(int row);

      int childDeep() const;

      int childCount() const;

      int columnCount() const;

      ProtocolFrame *appendChild(ProtocolFrame *child);

      ProtocolFrame *prependChild(ProtocolFrame *child);

      bool insertChilds(int position, int count, int columns);

      nfc::NfcFrame &frame() const;

      QVariant data(int column) const;

      void set(int column, const QVariant &value);

      int row() const;

      int repeated() const;

      int addRepeated(int value);

      ProtocolFrame *parent() const;

      void setParent(ProtocolFrame *parent);

      /*
       * frame range
       */
      int rangeStart() const;

      int rangeEnd() const;

      /*
       * frame phase
       */
      bool isSenseFrame() const;

      bool isSelectionFrame() const;

      bool isInformationFrame() const;

      bool isAuthFrame() const;

      /*
       * information type
       */
      bool isRequestFrame() const;

      bool isResponseFrame() const;

      bool isFrameField() const;

      bool isFieldInfo() const;

      bool hasParityError() const;

      bool hasCrcError() const;

      bool hasSyncError() const;

   private:

      QSharedPointer<Impl> impl;

};

#endif /* PROTOCOLFRAME_H_ */
