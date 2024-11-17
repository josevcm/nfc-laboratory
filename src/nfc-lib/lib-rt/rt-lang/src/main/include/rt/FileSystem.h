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

#ifndef RT_FILESYSTEM_H
#define RT_FILESYSTEM_H

#include <string>
#include <list>

namespace rt {

class FileSystem
{
   public:

      struct DirectoryEntry
      {
         std::string name;
      };

   public:

      static bool isDirectory(const std::string &path);

      static bool isRegularFile(const std::string &path);

      static bool exists(const std::string &path);

      static bool createPath(const std::string &path);

      static bool truncateFile(const std::string &path);

      static std::list<DirectoryEntry> directoryList(const std::string &path);
};

}

#endif