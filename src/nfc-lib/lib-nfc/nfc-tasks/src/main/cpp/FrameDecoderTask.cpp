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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <rt/Logger.h>
#include <rt/BlockingQueue.h>

#include <nfc/NfcDecoder.h>
#include <nfc/FrameDecoderTask.h>

#include "AbstractTask.h"

namespace nfc {

struct FrameDecoderTask::Impl : FrameDecoderTask, AbstractTask
{
   // decoder status
   int status;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalSubject = nullptr;

   // frame stream subject
   rt::Subject<nfc::NfcFrame> *frameSubject = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // decoder
   std::shared_ptr<nfc::NfcDecoder> decoder;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   Impl() : AbstractTask("FrameDecoderTask", "decoder"), status(FrameDecoderTask::Halt), decoder(new nfc::NfcDecoder())
   {
      // access to signal subject stream
      signalSubject = rt::Subject<sdr::SignalBuffer>::name("signal.iq");

      // create frame stream subject
      frameSubject = rt::Subject<nfc::NfcFrame>::name("decoder.frame");

      // subscribe to signal events
      signalSubscription = signalSubject->subscribe([this](const sdr::SignalBuffer &buffer) {
         if (status == FrameDecoderTask::Listen)
            signalQueue.add(buffer);
      });
   }

   void start() override
   {
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.info("decoder command [{}]", {command->code});

         if (command->code == FrameDecoderTask::Start)
         {
            startDecoder(command.value());
         }
         else if (command->code == FrameDecoderTask::Stop)
         {
            stopDecoder(command.value());
         }
         else if (command->code == FrameDecoderTask::Configure)
         {
            configDecoder(command.value());
         }
      }

      /*
       * process signal decode
       */
      if (status == FrameDecoderTask::Listen)
      {
         signalDecode();
      }
      else
      {
         wait(50);
      }

      return true;
   }

   void startDecoder(rt::Event &command)
   {
      log.info("start frame decoding, pending frames {}", {signalQueue.size()});

      decoder->setSampleRate(0);

      signalQueue.clear();

      command.resolve();

      updateDecoderStatus(FrameDecoderTask::Listen);
   }

   void stopDecoder(rt::Event &command)
   {
      log.info("stop frame decoding, pending frames {}", {signalQueue.size()});

      decoder->setSampleRate(0);

      signalQueue.clear();

      command.resolve();

      updateDecoderStatus(FrameDecoderTask::Halt);
   }

   void configDecoder(rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log.info("change decoder config: {}", {config.dump()});

         if (config.contains("powerLevelThreshold"))
            decoder->setPowerLevelThreshold(config["powerLevelThreshold"]);

         if (config.contains("modulationThreshold"))
            decoder->setModulationThreshold(config["modulationThreshold"]);

         command.resolve();
      }
      else
      {
         command.reject();
      }
   }

   void signalDecode()
   {
      if (auto buffer = signalQueue.get())
      {
         for (const auto &frame : decoder->nextFrames(buffer.value()))
         {
            frameSubject->next(frame);
         }

         if (!buffer->isValid())
         {
            log.info("decoder EOF buffer received, finish!");

            updateDecoderStatus(FrameDecoderTask::Halt);
         }
      }
   }

   void updateDecoderStatus(int value)
   {
      status = value;

      json data({
                      {"status",    status == Listen ? "decoding" : "idle"},
                      {"queueSize", signalQueue.size()}
                });

      updateStatus(status, data);

      lastStatus = std::chrono::steady_clock::now();
   }
};

FrameDecoderTask::FrameDecoderTask() : rt::Worker("FrameDecoderTask")
{
}

rt::Worker *FrameDecoderTask::construct()
{
   return new FrameDecoderTask::Impl;
}

}
