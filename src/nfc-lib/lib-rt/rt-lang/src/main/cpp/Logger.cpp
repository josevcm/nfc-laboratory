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
#include <utility>
#include <chrono>
#include <cstring>
#include <iostream>
// this include do not work in mingw64!
//#include <filesystem>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>

//#define NULL_LOG
#define STDERR_LOG
//#define STDOUT_LOG
//#define FSTREAM_LOG

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

// direct to stderr logger, warning, performance impact! use only for strange debugging when others logger do not work!
#ifdef STDERR_LOG
struct LogWriter
{
   void push(LogEvent *event)
   {
      char date[32];
      struct tm timeinfo {};

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

      localtime_s(&timeinfo, &seconds);

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      fprintf(stderr, "%s.%03d %s (thread-%d) [%s] %s\n", date, millis, event->level.c_str(), event->thread, event->logger.c_str(), Format::format(event->format, event->params).c_str());

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

      fprintf(stdout, "%s.%03d %s (thread-%d) [%s] %s\n", date, millis, event->level.c_str(), event->thread, event->logger.c_str(), Format::format(event->format, event->params).c_str());

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
//      std::filesystem::create_directories("log");

      // open log file
      stream.open("log/nfc-lab.log", std::ios::out | std::ios::app);

      while (!shutdown)
      {
         while (auto event = queue.get(100))
         {
            if (stream)
            {
               write(event.value());
            }
         }
      }

      // close file
      stream.close();
   }

   void write(LogEvent *event)
   {
      struct tm timeinfo {};
      char date[32], buffer[4096];

      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

      localtime_s(&timeinfo, &seconds);

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      snprintf(buffer, sizeof(buffer), "%s.%03d %s (thread-%d) [%s] %s\n", date, millis, event->level.c_str(), event->thread, event->logger.c_str(), Format::format(event->format, event->params).c_str());

      stream << buffer;

      delete event;
   }

} writer;
#endif

struct Logger::Impl
{
   int level;
   std::string name;

   Impl(std::string name, int level) : name(std::move(name)), level(level)
   {
   }
};

static std::map<std::string, std::shared_ptr<Logger::Impl>> loggers;

Logger::Logger(const std::string &name, int level)
{
   if (loggers.find(name) == loggers.end())
      loggers[name] = std::make_shared<Logger::Impl>(name, level);

   impl = loggers[name];
}

void Logger::trace(const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= TRACE)
      writer.push(new LogEvent(tags[TRACE], impl->name, format, std::move(params)));
}

void Logger::debug(const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= DEBUG)
      writer.push(new LogEvent(tags[DEBUG], impl->name, format, std::move(params)));
}

void Logger::info(const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= INFO)
      writer.push(new LogEvent(tags[INFO], impl->name, format, std::move(params)));
}

void Logger::warn(const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= WARN)
      writer.push(new LogEvent(tags[WARN], impl->name, format, std::move(params)));
}

void Logger::error(const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= ERROR)
      writer.push(new LogEvent(tags[ERROR], impl->name, format, std::move(params)));
}

void Logger::print(int level, const std::string &format, std::vector<Variant> params) const
{
   if (impl->level >= level)
      writer.push(new LogEvent(tags[level & 0x7], impl->name, format, std::move(params)));
}

void Logger::setLevel(int value)
{
   impl->level = value;
}

}