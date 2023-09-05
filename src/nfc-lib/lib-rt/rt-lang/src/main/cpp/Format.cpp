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

#include <regex>
#include <iostream>

#include <rt/Format.h>

std::string rt::Format::format(const std::string &fmt, const std::vector<Variant> &parameters)
{
   std::regex token(R"(\{([\.0-9]*)?\})");

   std::string content = fmt;

   char buffer[4096];

   for (const auto &parameter : parameters)
   {
      buffer[0] = 0;

      std::smatch match;

      if (!std::regex_search(content, match, token))
         break;

      std::string opts = match[1];

      if (auto value = std::get_if<bool>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "s").c_str(), *value ? "true" : "false");
      }
      else if (auto value = std::get_if<char>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "c").c_str(), *value);
      }
      else if (auto value = std::get_if<short>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "d").c_str(), *value);
      }
      else if (auto value = std::get_if<int>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "d").c_str(), *value);
      }
      else if (auto value = std::get_if<long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "d").c_str(), *value);
      }
      else if (auto value = std::get_if<long long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "ll").c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned char>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "u").c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned short>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "u").c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned int>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "u").c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "u").c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned long long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "ll").c_str(), *value);
      }
      else if (auto value = std::get_if<float>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "f").c_str(), *value);
      }
      else if (auto value = std::get_if<double>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "f").c_str(), *value);
      }
      else if (auto value = std::get_if<char *>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "s").c_str(), *value);
      }
      else if (auto value = std::get_if<void *>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), "0x%p", *value);
      }
      else if (auto value = std::get_if<std::string>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "s").c_str(), value->c_str());
      }
      else if (auto value = std::get_if<std::thread::id>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "d").c_str(), *value);
      }
      else if (auto value = std::get_if<ByteBuffer>(&parameter))
      {
         value->reduce<int>(0, [&buffer](int offset, unsigned char value) {
            return offset + snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", value);
         });
      }

      content.replace(match.position(), match.length(), buffer);
   }

   return content;
}
