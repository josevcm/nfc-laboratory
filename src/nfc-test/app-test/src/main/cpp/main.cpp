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

#include <filesystem>

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/RecordDevice.h>


#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

using namespace rt;

Logger logger {"main"};

int startTest1(int argc, char *argv[])
{
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   nfc::NfcDecoder decoder;

   decoder.setEnableNfcA(true);
   decoder.setEnableNfcB(true);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(true);

   sdr::RecordDevice source(argv[1]);

   if (source.open(sdr::RecordDevice::OpenMode::Read))
   {
      while (!source.isEof())
      {
         sdr::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate(), 0, 0, sdr::SignalType::SAMPLE_REAL);

         if (source.read(samples) > 0)
         {
            std::list<nfc::NfcFrame> frames = decoder.nextFrames(samples);

            for (const nfc::NfcFrame &frame: frames)
            {
               if (frame.isPollFrame())
               {
                  logger.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.isListenFrame())
               {
                  logger.info("frame at {} -> {}: RX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
            }
         }
      }
   }

   return 0;
}

int processPath(const std::filesystem::path &path)
{
   return 0;
}

int processFile(const std::filesystem::path &path)
{
   nfc::NfcDecoder decoder;

   decoder.setEnableNfcA(true);
   decoder.setEnableNfcB(true);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(true);

   sdr::RecordDevice source(path.string());

   if (source.open(sdr::RecordDevice::OpenMode::Read))
   {
      while (!source.isEof())
      {
         sdr::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate(), 0, 0, sdr::SignalType::SAMPLE_REAL);

         if (source.read(samples) > 0)
         {
            std::list<nfc::NfcFrame> frames = decoder.nextFrames(samples);

            for (const nfc::NfcFrame &frame: frames)
            {
               if (frame.isPollFrame())
               {
                  logger.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.isListenFrame())
               {
                  logger.info("frame at {} -> {}: RX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
            }
         }
      }
   }

   return 0;
}

int main(int argc, char *argv[])
{
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   for (int i = 1; i < argc; i++)
   {
      std::filesystem::path path = argv[i];

      if (std::filesystem::is_directory(path))
      {
         logger.info("processing path {}", {path.string()});

         processPath(path);
      }
      else if (std::filesystem::is_regular_file(path))
      {
         logger.info("processing file {}", {path.string()});

         processFile(path);
      }
   }

   return 0;
}


