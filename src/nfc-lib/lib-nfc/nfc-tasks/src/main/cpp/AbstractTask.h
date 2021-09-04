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

#ifndef NFC_ABSTRACTTASK_H
#define NFC_ABSTRACTTASK_H

#include <rt/Event.h>
#include <rt/Logger.h>
#include <rt/Map.h>
#include <rt/BlockingQueue.h>
#include <rt/Subject.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace nfc {

struct AbstractTask
{
   rt::Logger log;

   // task status stream subject
   rt::Subject<rt::Event> *statusSubject = nullptr;

   // task control stream subject
   rt::Subject<rt::Event> *commandSubject = nullptr;

   // control stream observer
   rt::Subject<rt::Event>::Subscription commandSubscription;

   // command stream queue buffer
   rt::BlockingQueue<rt::Event> commandQueue;

   AbstractTask(const std::string &name, const std::string &subject) : log(name)
   {
      // create decoder status subject
      statusSubject = rt::Subject<rt::Event>::name(subject + ".status");

      // create decoder control subject
      commandSubject = rt::Subject<rt::Event>::name(subject + ".command");

      // subscribe to control events
      commandSubscription = commandSubject->subscribe([this](const rt::Event &command) { commandQueue.add(command); });
   }

   void updateStatus(int code, const json &data) const
   {
      log.trace("status update [{}]: {}", {code, data.dump()});

      statusSubject->next({code, {{"data", data.dump()}}}, true);
   }
};

}
#endif //NFC_LAB_ABSTRACTTASK_H
