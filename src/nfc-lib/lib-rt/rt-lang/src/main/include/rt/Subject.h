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

#ifndef LANG_SUBJECT_H
#define LANG_SUBJECT_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <utility>

#include <rt/Logger.h>
#include <rt/Finally.h>
#include <rt/Observer.h>

namespace rt {

template<typename T>
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

         bool operator==(const struct Observer &other) const
         {
            return this == &other;
         }
      };

      ~Subject() = default;

      inline void next(const T &value, bool retain = false)
      {
         for (auto observer = observers.begin(); observer != observers.end(); observer++)
         {
            if (observer->next)
            {
               observer->next(value);
            }
         }

         if (retain)
         {
            retained = std::make_shared<T>(value);
         }
      }

      inline void error(int error, const std::string &message)
      {
         for (auto observer = observers.begin(); observer != observers.end(); observer++)
         {
            if (observer->error)
            {
               observer->error(error, message);
            }
         }
      }

      inline void close()
      {
         for (auto observer = observers.begin(); observer != observers.end(); observer++)
         {
            if (observer->close)
            {
               observer->close();
            }
         }
      }

      inline Subscription subscribe(NextHandler next, ErrorHandler error = nullptr, CloseHandler close = nullptr)
      {
         // append observer to list
         auto &observer = observers.emplace_back(observers.size() + 1, next, error, close);
         log.debug("created subscription {} ({}) on subject {}", {observer.index, (void *) &observer, id});

         // emit retained values
         if (retained)
         {
            if (observer.next)
            {
               observer.next(*retained);
            }
         }

         // returns finisher to remove observer when destroyed
         return Finally {[this, &observer]() {
            log.debug("removed subscription {} ({}) from subject {}", {observer.index, (void *) &observer, id});
            observers.remove(observer);
         }};
      }

   public:

      static Subject<T> *name(const std::string &name)
      {
         std::lock_guard<std::mutex> lock(mutex);

         if (subjects.find(name) == subjects.end())
         {
            log.debug("create new subject for name {}", {name});
            subjects.emplace(name, name);
         }

         return &subjects[name];
      }

   public:

      explicit Subject(std::string id = std::string()) : id(std::move(id))
      {
      }

   private:

      // subject logger
      static Logger log;

      // attach / detach mutex
      static std::mutex mutex;

      // named subjects
      static std::map<std::string, Subject<T>> subjects;

      // subject name id
      std::string id;

      // subject observers subscriptions
      std::list<Observer> observers;

      // last value
      std::shared_ptr<T> retained;
};

template<typename T>
Logger Subject<T>::log {"Subject"};

template<typename T>
std::mutex Subject<T>::mutex;

template<typename T>
std::map<std::string, Subject<T>> Subject<T>::subjects;

}

#endif //NFCLAB_SUBJECT_H
