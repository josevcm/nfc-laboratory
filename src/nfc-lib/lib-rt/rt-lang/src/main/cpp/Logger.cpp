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

#include <sys/time.h>
#include <pthread.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>
#include <rt/Tokenizer.h>

namespace rt {

const char *tags[] = {
   "NONE", // 0
   "ERROR", // 1
   "WARN", // 2
   "INFO", // 3
   "DEBUG", // 4
   "TRACE", // 5
   "", // 6
   "" // 7
};

int getLevelIndex(std::string name)
{
   // convert tu uppercase (as TAGs are uppercase)
   std::transform(name.begin(), name.end(), name.begin(), toupper);

   // search for level in tags
   for (int i = 0; i < std::size(tags); i++)
   {
      if (name == tags[i])
      {
         return i;
      }
   }

   return -1;
}

// log event for store debugging information
struct Log
{
   int level;
   std::string tag;
   std::string logger;
   std::string format;
   std::vector<Variant> params;

   std::thread::id thread;
   std::chrono::time_point<std::chrono::system_clock> time;

   Log(const int level, std::string logger, std::string format, std::vector<Variant> params) :
      level(level),
      tag(tags[level]),
      logger(std::move(logger)),
      format(std::move(format)),
      params(std::move(params)),
      thread(std::this_thread::get_id()),
      time(std::chrono::system_clock::now())
   {
   }
};

// threaded appender
struct Appender
{
   // global writer level (disabled by default)
   int level = -1;

   // events queue
   BlockingQueue<Log *> queue;

   // output file
   std::ostream &stream;

   // shutdown flag
   std::atomic<bool> shutdown;

   // shutdown flag
   std::atomic<bool> buffered;

   // flush flag
   std::atomic<bool> flush;

   // writer thread
   std::thread thread;

   Appender(std::ostream &stream, int level, bool buffered) : level(level), stream(stream), shutdown(false), buffered(buffered), flush(false), thread([this] { this->exec(); })
   {
      constexpr sched_param param {};

      if (pthread_setschedparam(thread.native_handle(), SCHED_OTHER, &param))
      {
         printf("error setting logger thread priority: %s\n", std::strerror(errno));
      }
   }

   ~Appender()
   {
      stop();
   }

   void push(Log *event)
   {
      if (buffered)
         queue.add(event);
      else
         write(event);
   }

   void exec()
   {
      do
      {
         while (auto event = queue.get(100))
         {
            if (stream.good())
            {
               write(event.value());
            }
         }

         if (flush || shutdown)
         {
            stream.flush();
            flush = false;
         }
      }
      while (!shutdown);
   }

   void write(const Log *event) const
   {
      tm timeinfo {};
      char date[32], buffer[65535];
      std::stringstream ss;

      const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(event->time.time_since_epoch()).count();
      const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(event->time.time_since_epoch()).count() % 1000;

#ifdef _WIN32
      localtime_s(&timeinfo, &seconds);
#else
      localtime_r(&seconds, &timeinfo);
#endif

      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timeinfo);

      ss << std::setw(2) << std::setfill('0') << event->thread;

      const int size = snprintf(buffer, sizeof(buffer), "%s.%03d %s [%s] (%s) %s\n", date, static_cast<int>(millis), event->tag.c_str(), ss.str().c_str(), event->logger.c_str(), Format::format(event->format, event->params).c_str());

      stream.write(buffer, size);

      delete event;
   }

   void stop()
   {
      // if thread is active
      if (thread.joinable())
      {
         // signal shutdown
         shutdown = true;

         // wait for thread to finish
         thread.join();
      }
   }
};

// global appender instance
std::unique_ptr<Appender> appender;

// global mutex for logger instances (using construct-on-first-use to avoid static initialization order fiasco)
std::mutex &Logger::getMutex()
{
   static std::mutex instance;
   return instance;
}

// global levels map (using construct-on-first-use to avoid static initialization order fiasco)
std::map<std::string, int> &Logger::getLevels()
{
   static std::map<std::string, int> instance;
   return instance;
}

// logger implementation
Logger::Logger(std::string name, const int level) : level(level), name(std::move(name))
{
}

void Logger::trace(const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(TRACE_LEVEL))
      appender->push(new Log(TRACE_LEVEL, name, format, std::move(params)));
}

void Logger::debug(const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(DEBUG_LEVEL))
      appender->push(new Log(DEBUG_LEVEL, name, format, std::move(params)));
}

void Logger::info(const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(INFO_LEVEL))
      appender->push(new Log(INFO_LEVEL, name, format, std::move(params)));
}

void Logger::warn(const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(WARN_LEVEL))
      appender->push(new Log(WARN_LEVEL, name, format, std::move(params)));
}

void Logger::error(const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(ERROR_LEVEL))
      appender->push(new Log(ERROR_LEVEL, name, format, std::move(params)));
}

void Logger::print(int level, const std::string &format, std::vector<Variant> params) const
{
   if (isEnabled(level))
      appender->push(new Log(level & 0x7, name, format, std::move(params)));
}

bool Logger::isEnabled(int value) const
{
   return appender && (level >= value || appender->level >= value);
}

bool Logger::isTraceEnabled() const
{
   return isEnabled(TRACE_LEVEL);
}

bool Logger::isDebugEnabled() const
{
   return isEnabled(DEBUG_LEVEL);
}

bool Logger::isInfoEnabled() const
{
   return isEnabled(INFO_LEVEL);
}

int Logger::getLevel() const
{
   return level;
}

void Logger::setLevel(int level)
{
   this->level = level;
}

void Logger::setLevel(const std::string &level)
{
   this->level = getLevelIndex(level);
}

Logger *Logger::getLogger(const std::string &name, const int level)
{
   std::lock_guard lock(getMutex());

   // insert logger if not found in instances
   loggers().emplace(name, std::shared_ptr<Logger>(new Logger(name, level)));

   // check if logger has a specific level and update
   const std::vector<std::string> tokens = Tokenizer::tokenize(name, '.');

   // check if any of already created logger matches and set its level
   for (const auto &[target, l]: getLevels())
   {
      int i = 0;

      // split logger name by '.'
      const std::vector<std::string> filter = Tokenizer::tokenize(target, '.');

      // check each token
      for (; i < std::min(tokens.size(), filter.size()); i++)
      {
         // use "*" as any match
         if (filter[i] == "*")
            continue;

         if (tokens[i] != filter[i])
            break;
      }

      // update logger level
      if (i == std::min(tokens.size(), filter.size()))
         loggers()[name]->level = l;
   }

   // return logger instance
   return loggers()[name].get();
}

std::map<std::string, std::shared_ptr<Logger>> &Logger::loggers()
{
   static std::map<std::string, std::shared_ptr<Logger>> instances;

   return instances;
}

int Logger::getRootLevel()
{
   return appender->level;
}

void Logger::setRootLevel(int level)
{
   appender->level = level;
}

void Logger::setRootLevel(const std::string &level)
{
   setRootLevel(getLevelIndex(level));
}

void Logger::setLoggerLevel(const std::string &target, const int level)
{
   std::lock_guard lock(getMutex());

   // add or update level for future loggers
   getLevels()[target] = level;

   // split target name by '.'
   const std::vector<std::string> filter = Tokenizer::tokenize(target, '.');

   // check if any of already created logger matches and set its level
   for (const auto &[name, logger]: loggers())
   {
      // split logger name by '.'
      const std::vector<std::string> tokens = Tokenizer::tokenize(name, '.');

      // check each token
      for (int i = 0; i < std::min(tokens.size(), filter.size()); i++)
      {
         // use "*" as any match
         if (filter[i] == "*")
            continue;

         if (tokens[i] != filter[i])
            return;
      }

      logger->level = level;
   }
}

void Logger::setLoggerLevel(const std::string &target, const std::string &level)
{
   setLoggerLevel(target, getLevelIndex(level));
}

void Logger::init(std::ostream &stream, int level, bool buffered)
{
   appender = std::make_unique<Appender>(stream, level, buffered);
}

void Logger::flush()
{
   if (appender)
      appender->flush = true;
}

void Logger::shutdown()
{
   if (appender)
      appender->stop();
}

}
