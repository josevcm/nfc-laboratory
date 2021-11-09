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

#include "DecoderStatusEvent.h"

const int DecoderStatusEvent::Type = QEvent::registerEventType();

const QString DecoderStatusEvent::Idle = "idle";
const QString DecoderStatusEvent::Decoding = "decoding";

DecoderStatusEvent::DecoderStatusEvent() : QEvent(QEvent::Type(Type))
{

}

DecoderStatusEvent::DecoderStatusEvent(int status) : QEvent(QEvent::Type(Type))
{
}

DecoderStatusEvent::DecoderStatusEvent(QJsonObject data) : QEvent(QEvent::Type(Type)), data(std::move(data))
{
}

bool DecoderStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString DecoderStatusEvent::status() const
{
   return data["status"].toString();
}

DecoderStatusEvent *DecoderStatusEvent::create()
{
   return new DecoderStatusEvent();
}

DecoderStatusEvent *DecoderStatusEvent::create(const QJsonObject &data)
{
   return new DecoderStatusEvent(data);
}
