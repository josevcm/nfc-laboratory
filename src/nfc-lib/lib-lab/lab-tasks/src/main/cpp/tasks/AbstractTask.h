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

#ifndef TASKS_ABSTRACTTASK_H
#define TASKS_ABSTRACTTASK_H

#include <rt/Event.h>
#include <rt/Logger.h>
#include <rt/Map.h>
#include <rt/BlockingQueue.h>
#include <rt/Subject.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace lab {

struct AbstractTask
{
   rt::Logger *log;

   // task status stream subject
   rt::Subject<rt::Event> *statusSubject = nullptr;

   // task control stream subject
   rt::Subject<rt::Event> *commandSubject = nullptr;

   // control stream observer
   rt::Subject<rt::Event>::Subscription commandSubscription;

   // command stream queue buffer
   rt::BlockingQueue<rt::Event> commandQueue;

   // last status send
   json lastStatus;

   AbstractTask(const std::string &name, const std::string &subject) : log(rt::Logger::getLogger(name))
   {
      // create decoder status subject
      statusSubject = rt::Subject<rt::Event>::name(subject + ".status");

      // create decoder control subject
      commandSubject = rt::Subject<rt::Event>::name(subject + ".command");

      // subscribe to control events
      commandSubscription = commandSubject->subscribe([this](const rt::Event &command) { commandQueue.add(command); });
   }

   void updateStatus(int code, const json &data)
   {
      if (data != lastStatus)
      {
         lastStatus = data;

         log->info("status update: {}", {data.dump()});
      }

      statusSubject->next({code, {{"data", data.dump()}}}, true);
   }
};

}
#endif
