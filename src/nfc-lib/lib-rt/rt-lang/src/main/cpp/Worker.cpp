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
   Logger log;

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

   explicit Impl(const std::string &name, int interval) : log(name), name(name), interval(interval)
   {
   }

   ~Impl()
   {
      terminate();
   }

   // wait and wait for notification
   inline void wait(int milliseconds)
   {
      std::unique_lock<std::mutex> lock(sleepMutex);

      if (milliseconds > 0)
         sync.wait_for(lock, std::chrono::milliseconds(milliseconds));
      else
         sync.wait(lock);
   }

   inline void notify()
   {
      sync.notify_one();
   }

   // signal termination
   inline void terminate()
   {
      // set terminate flag
      if (!terminated.fetch_add(1))
      {
         // notify
         sync.notify_one();

         // wait until worker finish
         std::lock_guard<std::mutex> lock(aliveMutex);
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

bool Worker::alive()
{
   return !impl->terminated;
}

void Worker::wait(int milliseconds)
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
   std::lock_guard<std::mutex> lock(impl->aliveMutex);

   std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

   impl->log.info("started worker for task {}", {impl->name});

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

   impl->log.info("finished worker for task {}, running time {}", {impl->name, duration});
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