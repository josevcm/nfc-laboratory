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

#include <fstream>
#include <iomanip>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>

#include <nfc/Nfc.h>
#include <nfc/NfcFrame.h>
#include <nfc/FrameStorageTask.h>

#include "AbstractTask.h"

namespace nfc {

struct FrameStorageTask::Impl : FrameStorageTask, AbstractTask
{
   // frame storage subject
   rt::Subject<nfc::NfcFrame> *storageStream = nullptr;

   // frame decoder subject
   rt::Subject<nfc::NfcFrame> *decoderStream = nullptr;

   // frame stream subscription
   rt::Subject<nfc::NfcFrame>::Subscription decoderSubscription;

   // frame stream queue buffer
   rt::BlockingQueue<nfc::NfcFrame> frameQueue;

   Impl() : AbstractTask("FrameStorageTask", "storage")
   {
      // create storage stream subject
      storageStream = rt::Subject<nfc::NfcFrame>::name("storage.frame");

      // create decoder stream subject
      decoderStream = rt::Subject<nfc::NfcFrame>::name("decoder.frame");

      // subscribe to frame events
      decoderSubscription = decoderStream->subscribe([this](const nfc::NfcFrame &frame) {
         frameQueue.add(frame);
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
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.info("recorder command [{}]", {command->code});

         if (command->code == FrameStorageTask::Read)
         {
            readFile(command.value());
         }
         else if (command->code == FrameStorageTask::Write)
         {
            writeFile(command.value());
         }
         else if (command->code == FrameStorageTask::Clear)
         {
            clearQueue(command.value());
         }
      }

      wait(250);

      return true;
   }

   void readFile(rt::Event &command)
   {
      if (auto file = command.get<std::string>("file"))
      {
         log.info("read frames from file {}", {file.value()});

         json data;

         // create output file
         std::ifstream input(file.value());

         // read json file
         input >> data;

         if (data.contains("frames"))
         {
            // read frames from file
            for (const auto &frame : data["frames"])
            {
               nfc::NfcFrame nfcFrame;

               nfcFrame.setTechType(nfc::TechType::NfcA);
               nfcFrame.setFrameType(frame["frameCmd"]);
               nfcFrame.setFramePhase(frame["framePhase"]);
               nfcFrame.setFrameFlags(frame["frameFlags"]);
               nfcFrame.setFrameRate(frame["frameRate"]);
               nfcFrame.setTimeStart(frame["timeStart"]);
               nfcFrame.setTimeEnd(frame["timeEnd"]);
               nfcFrame.setSampleStart(frame["sampleStart"]);
               nfcFrame.setSampleEnd(frame["sampleEnd"]);

               std::string frameData = frame["frameData"];

               for (size_t index = 0, size = 0; index < frameData.length(); index += size + 1)
               {
                  nfcFrame.put(std::stoi(frameData.c_str() + index, &size, 16));
               }

               nfcFrame.flip();

               storageStream->next(nfcFrame);
            }
         }

         command.resolve();
      }
      else
      {
         command.reject();
      }
   }

   void writeFile(rt::Event &command)
   {
      if (auto file = command.get<std::string>("file"))
      {
         log.info("write frames to file {}", {file.value()});

         json frames = json::array();

         for (const auto &frame : frameQueue)
         {
            if (frame.isPollFrame() || frame.isListenFrame())
            {
               char buffer[4096];

               frame.reduce<int>(0, [&buffer](int offset, unsigned char value) {
                  return offset + snprintf(buffer + offset, sizeof(buffer) - offset, offset > 0 ? ":%02X" : "%02X", value);
               });

               frames.push_back({
                                      {"sampleStart", frame.sampleStart()},
                                      {"sampleEnd",   frame.sampleEnd()},
                                      {"timeStart",   frame.timeStart()},
                                      {"timeEnd",     frame.timeEnd()},
                                      {"frameCmd",   frame.frameType()},
                                      {"frameRate",   frame.frameRate()},
                                      {"frameFlags",  frame.frameFlags()},
                                      {"framePhase",  frame.framePhase()},
                                      {"frameData",   buffer}
                                });
            }
         }

         json data({{"frames", frames}});

         // create output file
         std::ofstream output(file.value());

         // write prettified JSON to another file
         output << std::setw(3) << data << std::endl;

         command.resolve();
      }
      else
      {
         command.reject();
      }
   }

   void clearQueue(rt::Event &event)
   {
      log.info("frame clearQueue");

      frameQueue.clear();

      event.resolve();
   }
};

FrameStorageTask::FrameStorageTask() : rt::Worker("FrameStorageTask")
{
}

rt::Worker *FrameStorageTask::construct()
{
   return new FrameStorageTask::Impl;
}

}
