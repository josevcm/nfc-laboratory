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

bool FileSystem::createDir(const std::string &path)
{
   if (isDirectory(path))
      return true;

#ifdef __WIN32
   if (mkdir(path.c_str()) < 0)
      return false;
#else
   if (mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0)
      return false;
#endif

   return true;
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