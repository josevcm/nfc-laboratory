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