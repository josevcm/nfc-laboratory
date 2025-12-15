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
   Logger *log = Logger::getLogger("rt.Executor");

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

   Impl(const int poolSize, const int coreSize) : poolSize(poolSize), shutdown(false)
   {
      log->info("executor service starting with {} threads", {coreSize});

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

      log->debug("worker thread {} started", {id});

      // main thread loop
      while (!shutdown)
      {
         if (auto next = waitingTasks.get())
         {
            const auto &task = next.value();

            runningTasks.add(task);

            try
            {
               log->info("task {} started in thread {}", {task->name(), id});

               task->run();
            }
            catch (std::exception &e)
            {
               log->error("##################################################");
               log->error("exception in {}: {}", {task->name(), std::string(e.what())});
               log->error("##################################################");
            }
            catch (...)
            {
               log->error("##################################################");
               log->error("unhandled exception in {}", {task->name()});
               log->error("##################################################");
            }

            log->info("task {} finished in thread {}", {task->name(), id});

            // on shutdown process do not remove from list to avoid concurrent modification
            if (!shutdown)
               runningTasks.remove(task);
         }
         else if (!shutdown)
         {
            // lock mutex before wait in condition variable
            std::unique_lock lock(syncMutex);

            // stop thread until is notified
            threadSync.wait(lock);
         }
      }

      log->info("executor thread {} terminated", {id});
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
      else
      {
         log->warn("submit task rejected, shutdown in progress...");
      }
   }

   void terminate(int timeout)
   {
      log->info("stopping threads of the executor service, timeout {}", {timeout});

      // signal executor shutdown
      shutdown = true;

      // terminate running tasks
      while (auto task = runningTasks.get())
      {
         log->debug("send terminate request for task {}", {task.value()->name()});

         task.value()->terminate();
      }

      // notify waiting threads
      threadSync.notify_all();

      log->info("now waiting for completion of all executor threads");

      // joint all threads
      for (auto &thread: threadList)
      {
         if (thread.joinable())
         {
            log->debug("joint on thread {}", {thread.get_id()});

            thread.join();
         }
      }

      // finally remove waiting tasks
      waitingTasks.clear();

      log->info("all threads terminated, executor service shutdown completed!");
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
