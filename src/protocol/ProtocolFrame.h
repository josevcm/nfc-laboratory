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

#ifndef PROTOCOLFRAME_H_
#define PROTOCOLFRAME_H_

#include <QObject>
#include <QList>
#include <QVector>
#include <QVariant>

class ProtocolFrame: public QObject
{
   public:

      enum Type
      {
         SenseFrame = 1, SelectionFrame = 2, InformationFrame = 3, AuthFrame = 4
      };

      enum Flags
      {
         RequestFrame = 1, ResponseFrame = 2, FrameField = 4, FieldInfo = 8, ParityError = 128, CrcError = 256
      };

      enum Fields
      {
         Id = 0, Time = 1, Elapsed = 2, Rate = 3, Type = 4, Data = 5, End = 6
      };

   private:

      // parent frame node
      ProtocolFrame *mParent;

      // frame data contents
      QVector<QVariant> mData;

      // frame childs
      QList<ProtocolFrame*> mChilds;

      // frame type
      int mType;

      // frame flags
      int mFlags;

      // frame repeats
      int mRepeated;

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
};

#endif /* PROTOCOLFRAME_H_ */
