/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef APP_PROTOCOLFRAME_H
#define APP_PROTOCOLFRAME_H

#include <QVector>
#include <QVariant>
#include <QSharedPointer>

#include <lab/data/RawFrame.h>

class ProtocolFrame : public QObject
{
   struct Impl;

   public:

      enum Flags
      {
         // field bits
         FrameField = 0x0001,
         FieldInfo = 0x0002,

         // type bits
         RequestFrame = 0x0100,
         ResponseFrame = 0x0200,
         SenseFrame = 0x0400,
         SelectionFrame = 0x0800,
         ApplicationFrame = 0x1000,
         AuthFrame = 0x2000,
         StartupFrame = 0x8000,

         // errors bits
         ParityError = 0x010000,
         CrcError = 0x020000,
         SyncError = 0x020000
      };

      enum Fields
      {
         Name = 0,
         Flags = 1,
         Data = 2
      };

   public:

      ProtocolFrame(const QVector<QVariant> &data, int flags, const lab::RawFrame &frame);

      ProtocolFrame(const QVector<QVariant> &data, int flags, ProtocolFrame *parent, int start = -1, int end = -1);

      ~ProtocolFrame() override;

      void clearChilds();

      ProtocolFrame *child(int row);

      int childDeep() const;

      int childCount() const;

      int columnCount() const;

      ProtocolFrame *appendChild(ProtocolFrame *child);

      ProtocolFrame *prependChild(ProtocolFrame *child);

      bool insertChild(int position, int count, int columns);

      lab::RawFrame &frame() const;

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
       * frame type
       */

      /*
       * information type
       */
      bool isStartupFrame() const;

      bool isRequestFrame() const;

      bool isResponseFrame() const;

      bool isExchangeFrame() const;

      bool isSenseFrame() const;

      bool isSelectionFrame() const;

      bool isInformationFrame() const;

      bool isAuthFrame() const;

      /*
       * field type
       */
      bool isFrameField() const;

      bool isFieldInfo() const;

      /*
       * frame errors
       */
      bool hasParityError() const;

      bool hasCrcError() const;

      bool hasSyncError() const;

   private:

      QSharedPointer<Impl> impl;

};

#endif /* PROTOCOLFRAME_H_ */
