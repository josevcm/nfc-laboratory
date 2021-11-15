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

#ifndef LANG_LOGGER_H
#define LANG_LOGGER_H

#include <cstdio>
#include <string>
#include <vector>
#include <initializer_list>

#include <rt/Variant.h>

namespace rt {

class Logger
{
   public:

      struct Impl;

      enum Level
      {
         NONE = 0,
         ERROR = 1,
         WARN = 2,
         INFO = 3,
         DEBUG = 4,
         TRACE = 5
      };

      explicit Logger(const std::string &name, int level = INFO);

      void trace(const std::string &format, std::vector<Variant> params = {}) const;

      void debug(const std::string &format, std::vector<Variant> params = {}) const;

      void info(const std::string &format, std::vector<Variant> params = {}) const;

      void warn(const std::string &format, std::vector<Variant> params = {}) const;

      void error(const std::string &format, std::vector<Variant> params = {}) const;

      void print(int level, const std::string &format, std::vector<Variant> params = {}) const;

      void setLevel(int value);

   private:

      std::shared_ptr<Impl> impl;
};

}
#endif //LAB_LOGGING_LOGGER_H