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

#include <parser/ParserNfcA.h>
#include <parser/ParserNfcB.h>
#include <parser/ParserNfcF.h>
#include <parser/ParserNfcV.h>
#include <parser/ParserISO7816.h>

#include "ProtocolParser.h"

struct ProtocolParser::Impl
{
   ParserNfcA nfca;

   ParserNfcB nfcb;

   ParserNfcF nfcf;

   ParserNfcV nfcv;

   ParserISO7816 iso7816;

   unsigned int frameCount;

   lab::RawFrame lastFrame;

   Impl() : frameCount(1)
   {
   }

   void reset()
   {
      frameCount = 1;

      nfca.reset();
      nfcb.reset();
      nfcf.reset();
      nfcv.reset();
      iso7816.reset();
   }

   ProtocolFrame *parse(const lab::RawFrame &frame)
   {
      switch (frame.techType())
      {
         case lab::FrameTech::NfcATech:
            return nfca.parse(frame);

         case lab::FrameTech::NfcBTech:
            return nfcb.parse(frame);

         case lab::FrameTech::NfcFTech:
            return nfcf.parse(frame);

         case lab::FrameTech::NfcVTech:
            return nfcv.parse(frame);

         case lab::FrameTech::Iso7816Tech:
            return iso7816.parse(frame);

         default:
            return nullptr;
      }
   }
};

ProtocolParser::ProtocolParser(QObject *parent) : QObject(parent), impl(new Impl)
{
}

ProtocolParser::~ProtocolParser() = default;

void ProtocolParser::reset()
{
   impl->reset();
}

ProtocolFrame *ProtocolParser::parse(const lab::RawFrame &frame)
{
   return impl->parse(frame);
}
