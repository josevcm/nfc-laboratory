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

class ProtocolFrame : public QObject
{
      struct Impl;

   public:

      enum Type
      {
         SenseFrame = 1,
         SelectionFrame = 2,
         InformationFrame = 3,
         AuthFrame = 4
      };

      enum Flags
      {
         RequestFrame = 0x0001,
         ResponseFrame = 0x0002,
         FrameField = 0x0004,
         FieldInfo = 0x0008,
         Encrypted = 0x0080,
         ParityError = 0x0100,
         CrcError = 0x0200
      };

      enum Fields
      {
         Id = 0,
         Start = 1,
         End = 2,
         Rate = 3,
         Type = 4,
         Flags = 5,
         Data = 6
      };

   public:

      ProtocolFrame(const QVector<QVariant> &data, QObject *parent);

      ProtocolFrame(const QVector<QVariant> &data, int flags = 0, int type = 0, ProtocolFrame *parent = nullptr);

      virtual ~ProtocolFrame();

      void clearChilds();

      ProtocolFrame *child(int row);

      int childDeep() const;

      int childCount() const;

      int columnCount() const;

      void appendChild(ProtocolFrame *child);

      void prependChild(ProtocolFrame *child);

      bool insertChilds(int position, int count, int columns);

      QVariant data(int column) const;

      void set(int column, const QVariant &value);

      int row() const;

      int repeated() const;

      int addRepeated(int value);

      ProtocolFrame *parent() const;

      void setParent(ProtocolFrame *parent);

      /*
       *
       */
      bool isSenseFrame() const;

      bool isSelectionFrame() const;

      bool isInformationFrame() const;

      bool isAuthFrame() const;

      /*
       *
       */
      bool isRequestFrame() const;

      bool isResponseFrame() const;

      bool isFrameField() const;

      bool isFieldInfo() const;

      bool hasParityError() const;

      bool hasCrcError() const;

   private:

      QSharedPointer<Impl> impl;

};

#endif /* PROTOCOLFRAME_H_ */
