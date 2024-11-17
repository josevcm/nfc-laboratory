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

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

#include <rt/FileSystem.h>

namespace rt {

bool FileSystem::isDirectory(const std::string &path)
{
   struct stat sb {};

   if (stat(path.c_str(), &sb) == 0)
   {
      return S_ISDIR(sb.st_mode);
   }

   return false;
}

bool FileSystem::isRegularFile(const std::string &path)
{
   struct stat sb {};

   if (stat(path.c_str(), &sb) == 0)
   {
      return S_ISREG(sb.st_mode);
   }

   return false;
}

bool FileSystem::exists(const std::string &path)
{
   struct stat sb {};

   return stat(path.c_str(), &sb) == 0;
}

bool FileSystem::createPath(const std::string &path)
{
   if (isDirectory(path))
      return true;

   // append / at the end if not present
   std::string create = path.back() == '/' ? path : path + "/";

   size_t pos = 0;
   std::string stage;

   // create full path
   while ((pos = create.find_first_of('/', pos)) != std::string::npos)
   {
      stage = create.substr(0, pos + 1);

      if (!exists(stage))
      {
#ifdef __WIN32
         if (mkdir(stage.c_str()) < 0)
            return false;
#else
         if (mkdir(stage.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0)
         return false;
#endif
      }

      pos++;
   }

   return true;
}

bool FileSystem::truncateFile(const std::string &path)
{
   if (isDirectory(path))
      return false;

   int lastSlash = path.find_last_of('/');

   if (lastSlash != std::string::npos)
      createPath(path.substr(0, lastSlash));

   std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);

   return file.is_open();
}

std::list<FileSystem::DirectoryEntry> FileSystem::directoryList(const std::string &path)
{
   std::list<DirectoryEntry> result;

   if (isDirectory(path))
   {
      struct dirent *de;

      DIR *dp = opendir(path.c_str());

      while ((de = readdir(dp)) != nullptr)
      {
         result.push_back({path + "/" + de->d_name});
      }

      closedir(dp);
   }

   return result;
}

}