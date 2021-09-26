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

#ifndef NFC_LAB_NFC_H
#define NFC_LAB_NFC_H

namespace nfc {

enum TechType
{
   None = 0,
   NfcA = 1,
   NfcB = 2,
   NfcF = 3
};

enum RateType
{
   r106k = 0,
   r212k = 1,
   r424k = 2,
   r848k = 3
};

enum FrameType
{
   NoCarrier = 0,
   EmptyFrame = 1,
   PollFrame = 2,
   ListenFrame = 3
};

enum FrameFlags
{
   ShortFrame = 0x01,
   Encrypted = 0x08,
   ParityError = 0x20,
   CrcError = 0x40,
   Truncated = 0x80
};

enum FramePhase
{
   CarrierFrame = 0,
   SelectionFrame = 1,
   ApplicationFrame = 2
};

}

#endif //NFC_LAB_NFC_H
