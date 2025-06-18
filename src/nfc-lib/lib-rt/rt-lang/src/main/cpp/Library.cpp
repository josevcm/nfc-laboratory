/*

This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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

#include <string>

#include <rt/Logger.h>
#include <rt/Library.h>

namespace rt {

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// TODO: WORK IN PROGRESS...
struct Library::Impl
{
   Logger *log = Logger::getLogger("rt.Library");

   void *handle = nullptr;

   Impl(const std::string &name)
   {
      std::string path;

      // Check if the name has a file extension
      if (name.find('.') == std::string::npos)
      {
         // If not, append the appropriate extension based on the platform
#if defined(_WIN32)
         path = name + ".dll"; // Windows
#else
         path = name + ".so"; // Linux
#endif
      }

      // and load it
#if defined(_WIN32)
      handle = LoadLibraryA(path.c_str());
#else
      impl->handle = dlopen(path.c_str(), RTLD_LAZY);
#endif

      if (handle)
         log->info("library {} loaded successfully", {name});
      else
         log->warn("failed to load library {}", {name});
   }

   // destructor
   ~Impl()
   {
      if (handle)
      {
#if defined(_WIN32)
         FreeLibrary(static_cast<HMODULE>(handle));
#else
         dlclose(handle);
#endif
      }
   }
};

Library::Library(const std::string &name) : impl(new Impl(name))
{
}

Library::~Library()
{
}

bool Library::isLoaded() const
{
   return false; // Placeholder, implement actual check
}

}
