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

#ifndef RT_SUBJECT_H
#define RT_SUBJECT_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <utility>

#include <rt/Logger.h>
#include <rt/Finally.h>
#include <rt/Observer.h>

namespace rt {

template <typename T>
class Subject
{
   public:

      typedef Finally Subscription;
      typedef std::function<void(T)> NextHandler;
      typedef std::function<void(int, std::string)> ErrorHandler;
      typedef std::function<void()> CloseHandler;

      struct Observer
      {
         int index;
         NextHandler next;
         ErrorHandler error;
         CloseHandler close;

         Observer(int index, NextHandler next, ErrorHandler error, CloseHandler close) : index(index), next(std::move(next)), error(std::move(error)), close(std::move(close))
         {
         }

         bool operator==(const Observer &other) const
         {
            return this == &other;
         }
      };

      ~Subject() = default;

      void next(const T &value, bool retain = false)
      {
         std::vector<Observer> localObservers;

         {
            std::lock_guard lock(mutex);
            localObservers = observers;
         }

         for (auto observer = localObservers.begin(); observer != localObservers.end(); ++observer)
         {
            if (observer->next)
            {
               observer->next(value);
            }
         }

         if (retain)
         {
            std::lock_guard lock(mutex);
            retained = std::make_shared<T>(value);
         }
      }

      void error(int error, const std::string &message)
      {
         std::vector<Observer> localObservers;

         {
            std::lock_guard lock(mutex);
            localObservers = observers; // Copia para evitar iterar bajo lock
         }

         for (auto observer = localObservers.begin(); observer != localObservers.end(); ++observer)
         {
            if (observer->error)
            {
               observer->error(error, message);
            }
         }
      }

      void close()
      {
         std::vector<Observer> localObservers;

         {
            std::lock_guard lock(mutex);
            localObservers = observers;
         }

         for (auto observer = localObservers.begin(); observer != localObservers.end(); ++observer)
         {
            if (observer->close)
            {
               observer->close();
            }
         }
      }

      Subscription subscribe(NextHandler next, ErrorHandler error = nullptr, CloseHandler close = nullptr)
      {
         std::lock_guard lock(mutex);

         // append observer to list
         auto &observer = observers.emplace_back(observers.size() + 1, next, error, close);
         log->debug("created subscription {} ({}) on subject {}", {observer.index, static_cast<void *>(&observer), id});

         // emit retained values
         if (retained)
         {
            if (observer.next)
            {
               observer.next(*retained);
            }
         }

         // returns finisher to remove observer when destroyed
         return {
            [this, &observer] {

               std::lock_guard lock(mutex);

               log->debug("removed subscription {} ({}) from subject {}", {observer.index, static_cast<void *>(&observer), id});

               auto it = std::find(observers.begin(), observers.end(), observer);

               if (it != observers.end())
                  observers.erase(it);
            }
         };
      }

      static Subject *name(const std::string &name)
      {
         std::lock_guard lock(mutex);

         if (subjects.find(name) == subjects.end())
         {
            log->debug("create new subject for name {}", {name});
            subjects.emplace(name, name);
         }

         return &subjects[name];
      }

      explicit Subject(std::string id = std::string()) : id(std::move(id))
      {
      }

   private:

      // subject logger
      static Logger *log;

      // attach / dettach mutex
      static std::mutex mutex;

      // named subjects
      static std::map<std::string, Subject> subjects;

      // subject name id
      std::string id;

      // subject observers subscriptions
      std::vector<Observer> observers;

      // last value
      std::shared_ptr<T> retained;
};

template <typename T>
Logger *Subject<T>::log = Logger::getLogger("rt.Subject");

template <typename T>
std::mutex Subject<T>::mutex;

template <typename T>
std::map<std::string, Subject<T>> Subject<T>::subjects;

}

#endif
