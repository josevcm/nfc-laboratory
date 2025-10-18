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

#include <regex>
#include <iostream>

#include <rt/Format.h>

const char *ws = " \t\n\r\f\v";

std::string rt::Format::format(const std::string &fmt, const std::vector<Variant> &parameters)
{
   std::regex token(R"(\{(['-+]?\.?[0-9]*)?([xXt])?\})");

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
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "llu" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<float>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%'" + opts + "f").c_str(), *value);
      }
      else if (auto value = std::get_if<double>(&parameter))
      {
         snprintf(buffer, sizeof(buffer), ("%'" + opts + "f").c_str(), *value);
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
         snprintf(buffer, sizeof(buffer), ("%" + opts + (mode.empty() ? "d" : mode)).c_str(), *value);
      }
      else if (auto value = std::get_if<std::vector<int>>(&parameter))
      {
         // format as: {n, n, .... n}
         snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "{");

         for (int i = 0; i < value->size(); i++)
         {
            if (i > 0)
               snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ", ");

            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%d", value->at(i));
         }

         snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "}");
      }
      else if (auto value = std::get_if<Buffer<unsigned char>>(&parameter))
      {
         int offset = 0;

         if (mode.empty())
         {
            // format line as: 0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
            for (int i = 0; i < value->size(); i += 16)
            {
               offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%04X: ", i);

               for (int j = 0; j < 16; j++)
               {
                  if (i + j < value->size())
                     offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", static_cast<unsigned int>(value->data()[i + j]));
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
         else if (mode == "x" || mode == "X")
         {
            // format as hex: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00...
            for (int i = 0; i < value->size(); ++i)
            {
               offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", static_cast<unsigned int>(value->data()[i]));
            }
         }
      }
      else if (auto value = std::get_if<std::chrono::duration<long long, std::ratio<1, 1000000000>>>(&parameter))
      {
         // get duration horus
         int hours = (int)std::chrono::duration_cast<std::chrono::hours>(*value).count();

         // get duration minutes
         int minutes = (int)std::chrono::duration_cast<std::chrono::minutes>(*value).count() % 60;

         // get duration seconds
         int seconds = (int)std::chrono::duration_cast<std::chrono::seconds>(*value).count() % 60;

         // get duration milliseconds
         int milliseconds = (int)std::chrono::duration_cast<std::chrono::milliseconds>(*value).count() % 1000;

         // get duration microseconds
         int microseconds = (int)std::chrono::duration_cast<std::chrono::microseconds>(*value).count() % 1000000;

         if (mode.empty())
         {
            sprintf(buffer, "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
         }
         else if (mode == "t")
         {
            if (hours > 0)
               sprintf(buffer, "%02d:", hours);

            if (hours > 0 || minutes > 0)
               sprintf(buffer, "%02d:", minutes);

            if (hours > 0 || minutes > 0 || seconds > 0)
               sprintf(buffer, "%02d.", seconds);

            if (milliseconds > 0)
               sprintf(buffer, "%03d", milliseconds);
            else
               sprintf(buffer, "%06d", microseconds);
         }
         else
         {
            sprintf(buffer, "%lld ns", value->count());
         }

         // format as HH:MM:SS.mmm
         if (hours > 0)
            sprintf(buffer, "%02d:", hours);

         if (minutes > 0)
            sprintf(buffer, "%02d:", minutes);

         if (seconds > 0)
            sprintf(buffer, "%02d.", seconds);

         if (milliseconds > 0)
            sprintf(buffer, "%03d", milliseconds);
         else
            sprintf(buffer, "%06d", microseconds);
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
