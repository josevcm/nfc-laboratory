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
#include <memory>
#include <queue>
#include <list>
#include <thread>
#include <condition_variable>

#include <rt/Logger.h>
#include <rt/BlockingQueue.h>
#include <rt/Executor.h>

namespace rt {

struct Executor::Impl
{
   rt::Logger log {"Executor"};

   // max number of tasks in pool (waiting + running)
   int poolSize;

   // running threads
   std::list<std::thread> threadList;

   // waiting group
   std::condition_variable threadSync;

   // waiting tasks pool
   BlockingQueue<std::shared_ptr<Task>> waitingTasks;

   // current running tasks
   BlockingQueue<std::shared_ptr<Task>> runningTasks;

   // shutdown flag
   std::atomic<bool> shutdown;

   // sync mutex
   std::mutex syncMutex;

   Impl(int poolSize, int coreSize) : poolSize(poolSize), shutdown(false)
   {
      log.info("executor service starting width {} threads", {coreSize});

      // create new thread group
      for (int i = 0; i < coreSize; i++)
      {
         threadList.emplace_back([this] { this->exec(); });
      }
   }

   void exec()
   {
      // get current thread id
      std::thread::id id = std::this_thread::get_id();

      log.debug("worker thread {} started", {id});

      // main thread loop
      while (!shutdown)
      {
         if (auto next = waitingTasks.get())
         {
            auto task = next.value();

            runningTasks.add(task);

            try
            {
               log.debug("task {} started in thread {}", {task->name(), id});

               next.value()->run(); // call next handler

               log.debug("task {} finished in thread {}", {task->name(), id});
            }
            catch (...)
            {
               log.error("unhandled task {} exception in thread {}", {task->name(), id});
            }

            // on shutdown process do not remove from list to avoid concurrent modification
            if (!shutdown)
            {
               runningTasks.remove(task);
            }
         }
         else if (!shutdown)
         {
            // lock mutex before wait in condition variable
            std::unique_lock<std::mutex> lock(syncMutex);

            // stop thread until is notified
            threadSync.wait(lock);
         }
      }

      log.debug("executor thread {} terminated", {id});
   }

   void submit(Task *task)
   {
      if (!shutdown)
      {
         // add task to wait pool
         waitingTasks.add(task);

         // notify waiting threads
         threadSync.notify_all();
      }
   }

   void terminate(int timeout)
   {
      log.info("stopping threads of the executor service");

      // signal executor shutdown
      shutdown = true;

      // terminate running tasks
      while (auto task = runningTasks.get())
      {
         log.debug("send terminate request for task {}", {task.value()->name()});

         task.value()->terminate();
      }

      // notify waiting threads
      threadSync.notify_all();

      log.info("waiting for completion of all threads");

      // joint all threads
      for (auto &thread : threadList)
      {
         if (thread.joinable())
         {
            log.debug("joint on thread {}", {thread.get_id()});

            thread.join();
         }
      }

      // finally remove waiting tasks
      waitingTasks.clear();

      log.info("all threads terminated, executor service shutdown completed!");
   }
};

Executor::Executor(int poolSize, int coreSize) : impl(std::make_shared<Impl>(poolSize, coreSize))
{
}

Executor::~Executor()
{
   impl->terminate(0);
}

void Executor::submit(Task *task)
{
   impl->submit(task);
}

void Executor::shutdown()
{
   impl->terminate(0);
}

}