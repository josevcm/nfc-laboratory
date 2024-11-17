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

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <utility>

#include <rt/Logger.h>
#include <rt/Worker.h>

namespace rt {

struct Worker::Impl
{
   Logger *log = Logger::getLogger("rt.Worker");

   // worker name
   std::string name;

   // loop intreval
   int interval;

   // alive synchronization condition
   std::mutex aliveMutex;

   // wait synchronization condition
   std::mutex sleepMutex;

   // synchronization condition
   std::condition_variable sync;

   // terminate flag
   std::atomic<int> terminated {0};

   explicit Impl(std::string name, int interval) : name(std::move(name)), interval(interval)
   {
   }

   ~Impl()
   {
      terminate();
   }

   // wait and wait for notification
   void wait(int milliseconds)
   {
      std::unique_lock lock(sleepMutex);

      if (milliseconds > 0)
         sync.wait_for(lock, std::chrono::milliseconds(milliseconds));
      else
         sync.wait(lock);
   }

   void notify()
   {
      sync.notify_one();
   }

   // signal termination
   void terminate()
   {
      // set terminate flag
      if (!terminated.fetch_add(1))
      {
         // notify
         sync.notify_one();

         // wait until worker finish
         std::lock_guard lock(aliveMutex);
      }
   }
};

Worker::Worker(const std::string &name, int interval) : impl(std::make_shared<Impl>(name, interval))
{
}

std::string Worker::name()
{
   return impl->name;
}

bool Worker::alive() const
{
   return !impl->terminated;
}

void Worker::wait(int milliseconds) const
{
   impl->wait(milliseconds);
}

void Worker::notify()
{
   impl->notify();
}

void Worker::terminate()
{
   impl->terminate();
}

void Worker::run()
{
   std::lock_guard lock(impl->aliveMutex);

   const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

   impl->log->info("started worker {}", {impl->name});

   // call workert start
   this->start();

   // run until worker terminated
   while (!impl->terminated)
   {
      if (!this->loop())
      {
         break;
      }
   }

   // call worker stop
   this->stop();

   auto duration = std::chrono::steady_clock::now() - start;

   impl->terminated = 1;

   impl->log->info("finished worker {}, running time {}", {impl->name, duration});
}

void Worker::start()
{
}

void Worker::stop()
{
}

bool Worker::loop()
{
   return false;
}

}