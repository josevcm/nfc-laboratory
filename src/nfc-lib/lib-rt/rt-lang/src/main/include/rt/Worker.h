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

#ifndef RT_WORKER_H
#define RT_WORKER_H

#include <string>
#include <memory>

#include <rt/Task.h>

namespace rt {

class Worker : public Task
{
   struct Impl;

   public:

      explicit Worker(const std::string &name, int interval = -1);

      bool alive() const;

      void wait(int milliseconds = 0) const;

      void notify();

      std::string name() override;

      void terminate() override;

      void run() override;

   protected:

      virtual void start();

      virtual void stop();

      virtual bool loop();

      std::shared_ptr<Impl> impl;
};

}

#endif
