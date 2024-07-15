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

const char *ws = " \t\n\r\f\v";

std::string rt::Format::format(const std::string &fmt, const std::vector<Variant> &parameters)
{
   std::regex token(R"(\{([\.0-9]*)?([xX])?\})");

   std::string content = fmt;

   char buffer[16384];

   for (const auto &parameter: parameters)
   {
      buffer[0] = 0;

      std::smatch match;

      if (!std::regex_search(content, match, token))
         break;

      std::string opts = match[1];
      std::string mode = match[2];

      if (auto value = std::get_if<bool>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + "s").c_str(), *value ? "true" : "false");
      }
      else if (auto value = std::get_if<char>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "c" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<short>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "d" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<int>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "d" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "d" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<long long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "ll" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned char>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "u" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned short>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "u" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned int>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "u" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "u" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<unsigned long long>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "ll" : mode)).c_str(), *value);
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
         snprintf(buffer, sizeof(buffer), ("0x%" + opts + "x").c_str(), *value);
      }
      else if (auto value = std::get_if<Buffer<unsigned char>>(&parameter))
      {
         int offset = 0;

         // format line as: 0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
         for (int i = 0; i < value->size(); i += 16)
         {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%04X: ", i);

            for (int j = 0; j < 16; j++)
            {
               if (i + j < value->size())
                  offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", (unsigned int) value->data()[i + j]);
               else
                  offset += snprintf(buffer + offset, sizeof(buffer) - offset, "   ");
            }

            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " ");

            for (int j = 0; j < 16; j++)
            {
               if (i + j < value->size())
               {
                  offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%c", isprint(value->data()[i + j]) ? value->data()[i + j] : '.');
               }
            }

            if (i + 16 < value->size())
            {
               offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

               // exit if print buffer is reached
               if (sizeof(buffer) - offset < 80)
               {
                  snprintf(buffer + offset, sizeof(buffer) - offset, "...");

                  break;
               }
            }
         }
      }
      else if (auto value = std::get_if<std::chrono::duration<long long, std::ratio<1, 1000000000>>>(&parameter))
      {
         // get duration horus
         int hours = (int) std::chrono::duration_cast<std::chrono::hours>(*value).count();

         // get duration minutes
         int minutes = (int) std::chrono::duration_cast<std::chrono::minutes>(*value).count() % 60;

         // get duration seconds
         int seconds = (int) std::chrono::duration_cast<std::chrono::seconds>(*value).count() % 60;

         // get duration milliseconds
         int milliseconds = (int) std::chrono::duration_cast<std::chrono::milliseconds>(*value).count() % 1000;

         // format as HH:MM:SS.mmm
         sprintf(buffer, "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
      }

      content.replace(match.position(), match.length(), buffer);
   }

   return content;
}

std::string rt::Format::trim(const std::string &str)
{
   return ltrim(rtrim(str));
}

std::string rt::Format::ltrim(const std::string &str)
{
   std::string s = str;
   s.erase(0, s.find_first_not_of(ws));
   return s;
}

std::string rt::Format::rtrim(const std::string &str)
{
   std::string s = str;
   s.erase(s.find_last_not_of(ws) + 1);
   return s;
}