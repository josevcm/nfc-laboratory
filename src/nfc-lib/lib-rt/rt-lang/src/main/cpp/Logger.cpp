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

#include <sys/time.h>
#include <pthread.h>

#include <map>
#include <string>
#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/FileSystem.h>
#include <rt/BlockingQueue.h>

namespace rt {

const char *tags[] = {
      "", // 0
      "ERROR", // 1
      "WARN", // 2
      "INFO", // 3
      "DEBUG", // 4
      "TRACE", // 5
      "", // 6
      "" // 7
};

// log event for store debugging information
struct LogEvent
{
   std::string level;
   std::string logger;
   std::string format;
   std::vector<Variant> params;

   std::thread::id thread;
   std::chrono::time_point<std::chrono::system_clock> time;

   LogEvent(std::string level, std::string logger, std::string format, std::vector<Variant> params) :
         level(std::move(level)),
         logger(std::move(logger)),
         format(std::move(format)),
         params(std::move(params)),
         thread(std::this_thread::get_id()),
         time(std::chrono::system_clock::now())
   {
   }
};

// threaded logger to console stdout, runs on low priority thread
struct Logger::Writer
{
   // events queue
   BlockingQueue<LogEvent *> queue;

   // output file
   std::ostream &stream;

   // shutdown flag
   std::atomic<bool> shutdown;

   // shutdown flag
   std::atomic<bool> buffered;

   // writer thread
   std::thread thread;

   // global writer level (disabled by default)
   int level = -1;

   Writer(std::ostream &stream, bool buffered) : stream(stream), shutdown(false), buffered(buffered), thread([this] { this->exec(); })
   {
      sched_param param {0};

      if (pthread_setschedparam(thread.native_handle(), SCHED_OTHER, &param))
      {
         printf("error setting logger thread priority: %s\n", std::strerror(errno));
      }
   }

   ~Writer()
   {
      // signal shutdown
      shutdown = true;

      // wait for thread to finish
      thread.join();
   }

   void push(LogEvent *event)
   {
      if (buffered)
      {
         queue.add(event);
      }
      else
      {
         write(event);
      }
   }

   void exec()
   {
      while (!shutdown)
      {
         while (auto event = queue.get(100))
         {
            if (stream.good())
            {
               write(event.value());
            }
         }
      }
   }

   void write(LogEvent *event)
   {
      char date[32], buffer[65535];
      struct tm timeinfo {};

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

#ifdef _WIN32
      localtime_s(&timeinfo, &seconds);
#else
      localtime_r(&seconds, &timeinfo);
#endif

      std::ostringstream oss;
      oss << event->thread;

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      int size = snprintf(buffer, sizeof(buffer), "%s.%03d %s (thread-%s) [%s] %s\n", date, (int) millis, event->level.c_str(), oss.str().c_str(), event->logger.c_str(), Format::format(event->format, event->params).c_str());

      stream.write(buffer, size);

      delete event;
   }

};

struct Logger::Impl
{
   int level;
   std::string name;

   Impl(std::string name, int level) : name(std::move(name)), level(level)
   {
   }
};

std::shared_ptr<Logger::Writer> Logger::writer;

std::shared_ptr<Logger::Impl> putLogger(const std::string &name, int level)
{
   static std::map<std::string, std::shared_ptr<Logger::Impl>> loggers;

   if (loggers.find(name) == loggers.end())
      loggers[name] = std::make_shared<Logger::Impl>(name, level);

   return loggers[name];
}

Logger::Logger(const std::string &name, int level)
{
   impl = putLogger(name, level);
}

void Logger::trace(const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= TRACE_LEVEL || writer->level >= TRACE_LEVEL))
      writer->push(new LogEvent(tags[TRACE_LEVEL], impl->name, format, std::move(params)));
}

void Logger::debug(const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= DEBUG_LEVEL || writer->level >= DEBUG_LEVEL))
      writer->push(new LogEvent(tags[DEBUG_LEVEL], impl->name, format, std::move(params)));
}

void Logger::info(const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= INFO_LEVEL || writer->level >= INFO_LEVEL))
      writer->push(new LogEvent(tags[INFO_LEVEL], impl->name, format, std::move(params)));
}

void Logger::warn(const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= WARN_LEVEL || writer->level >= WARN_LEVEL))
      writer->push(new LogEvent(tags[WARN_LEVEL], impl->name, format, std::move(params)));
}

void Logger::error(const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= ERROR_LEVEL || writer->level >= ERROR_LEVEL))
      writer->push(new LogEvent(tags[ERROR_LEVEL], impl->name, format, std::move(params)));
}

void Logger::print(int level, const std::string &format, std::vector<Variant> params) const
{
   if (writer && (impl->level >= level || writer->level >= level))
      writer->push(new LogEvent(tags[level & 0x7], impl->name, format, std::move(params)));
}

void Logger::setLevel(int level)
{
   impl->level = level;
}

void Logger::setWriterLevel(int level)
{
   writer->level = level;
}

void Logger::init(std::ostream &stream, bool buffered)
{
   writer.reset(new Writer(stream, buffered));
}

void Logger::flush()
{
   writer->stream.flush();
}

}