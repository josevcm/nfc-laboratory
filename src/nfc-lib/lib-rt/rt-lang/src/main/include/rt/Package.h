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

#ifndef RT_PACKAGE_H
#define RT_PACKAGE_H

#include <string>
#include <memory>

namespace rt {

class Package
{
   struct Impl;

   public:

      enum Mode
      {
         Read, Write
      };

      explicit Package(const std::string &filename);

      int open(Mode mode);

      void close();

      bool isOpen() const;

      int addEntry(const std::string &name, unsigned int length);

      int getEntry(std::string &name, unsigned int &length);

      int findEntry(const std::string &name, unsigned int &length);

      int nextEntry();

      int readData(void *data, unsigned size);

      int writeData(const void *data, unsigned size);

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif
