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

#include <string>
#include <fstream>
#include <cmath>
#include <thread>
#include <utility>
#include <chrono>
#include <cstring>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>

//#define NULL_LOG
//#define STDERR_LOG
#define STDOUT_LOG
//#define FSTREAM_LOG

namespace rt {

const char *tags[] = {
      "", // 0
      "ERROR", // 1
      "WARN", // 2
      "", // 3
      "INFO", // 4
      "", // 5
      "", // 6
      "", // 7
      "DEBUG", // 8
      "", // 9
      "", // 10
      "", // 11
      "", // 12
      "", // 13
      "", // 14
      "", // 15
      "TRACE" // 16
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

// null logger for discard all messages
#ifdef NULL_LOG
struct LogWriter
{
   void push(LogEvent *event)
   {
      delete event;
   }
} writer;
#endif

// direct to stderr logger, waring: big performance impact! use only for strange debugging when others logger do not work!
#ifdef STDERR_LOG
struct LogWriter
{
   void push(LogEvent *event)
   {
      write(std::shared_ptr<LogEvent>(event));
   }

   void write(const std::shared_ptr<LogEvent> &event)
   {
      char date[32];
      struct tm timeinfo {};

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

      localtime_s(&timeinfo, &seconds);

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      fprintf(stderr,"%s.%03d (thread-%d) %s [%s] %s\n", date, millis, event->thread, event->level.c_str(), event->logger.c_str(), Format::format(event->format, event->params).c_str());

      delete event;
   }

} writer;
#endif

// threaded logger to console stdout, runs on low priority thread
#ifdef STDOUT_LOG
struct LogWriter
{
   // writer thread
   std::thread thread;

   // shutdown flag
   std::atomic<bool> shutdown;

   // events queue
   BlockingQueue<LogEvent *> queue;

   LogWriter() : thread([this] { this->exec(); }), shutdown(false)
   {
      sched_param param {0};

      if (pthread_setschedparam(thread.native_handle(), SCHED_OTHER, &param))
      {
         printf("error setting logger thread priority: %s\n", std::strerror(errno));
      }
   }

   ~LogWriter()
   {
      // signal shutdown
      shutdown = true;

      // wait for thread to finish
      thread.join();
   }

   void push(LogEvent *event)
   {
      queue.add(event);
   }

   void exec()
   {
      while (!shutdown)
      {
         while (auto event = queue.get(50))
         {
            write(event.value());
         }
      }
   }

   void write(LogEvent *event)
   {
      char date[32];
      struct tm timeinfo {};

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

      localtime_s(&timeinfo, &seconds);

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      fprintf(stdout, "%s.%03d (thread-%d) %s [%s] %s\n", date, millis, event->thread, event->level.c_str(), event->logger.c_str(), Format::format(event->format, event->params).c_str());

      delete event;
   }

} writer;
#endif

// threaded logger to file stream, runs on low priority thread
#ifdef FSTREAM_LOG
struct LogWriter
{
   // writer thread
   std::thread thread;

   // shutdown flag
   std::atomic<bool> shutdown;

   // output file
   std::ofstream stream;

   // events queue
   BlockingQueue<std::shared_ptr<LogEvent>> queue;

   LogWriter() : thread([this] { this->exec(); }), shutdown(false), stream("log/nfc-lab.log", std::ios::out | std::ios::app)
   {
      sched_param param {0};

      if (pthread_setschedparam(thread.native_handle(), SCHED_OTHER, &param))
      {
         printf("error setting logger thread priority: %s\n", std::strerror(errno));
      }
   }

   ~LogWriter()
   {
      // close file
      stream.close();

      // signal shutdown
      shutdown = true;

      // wait for thread to finish
      thread.join();
   }

   void push(LogEvent *event)
   {
      queue.add(event);
   }

   void exec()
   {
      while (!shutdown)
      {
         while (auto event = queue.get(50))
         {
            if (stream)
            {
               write(event.value());
            }
         }
      }
   }

   void write(LogEvent *event)
   {
      struct tm timeinfo {};
      char date[32], buffer[4096];

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

      localtime_s(&timeinfo, &seconds);

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      snprintf(buffer, sizeof(buffer), "%s.%03d (thread-%d) %s [%s] %s\n", date, millis, event->thread, event->level.c_str(), event->logger.c_str(), Format::format(event->format, event->params).c_str());

      stream << buffer;

      delete event;
   }

} writer;
#endif

struct Logger::Impl
{
   int levels;
   std::string name;

   Impl(int levels, std::string name) : levels(levels), name(std::move(name))
   {
   }
};

Logger::Logger(const char *name, int level) : self(std::make_shared<Impl>(level, name))
{
}

Logger::Logger(const std::string &name, int level) : self(std::make_shared<Impl>(level, name))
{
}

void Logger::trace(const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & TRACE)
   {
      writer.push(new LogEvent(tags[TRACE], self->name, format, std::move(params)));
   }
}

void Logger::debug(const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & DEBUG)
   {
      writer.push(new LogEvent(tags[DEBUG], self->name, format, std::move(params)));
   }
}

void Logger::info(const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & INFO)
   {
      writer.push(new LogEvent(tags[INFO], self->name, format, std::move(params)));
   }
}

void Logger::warn(const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & WARN)
   {
      writer.push(new LogEvent(tags[WARN], self->name, format, std::move(params)));
   }
}

void Logger::error(const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & ERROR)
   {
      writer.push(new LogEvent(tags[ERROR], self->name, format, std::move(params)));
   }
}

void Logger::print(int level, const std::string &format, std::vector<Variant> params) const
{
   if (self->levels & level)
   {
      writer.push(new LogEvent(tags[level & 0x07], self->name, format, std::move(params)));
   }
}

void Logger::set(int levels, bool enabled)
{
   if (enabled)
      self->levels |= levels;
   else
      self->levels ^= levels;
}

}