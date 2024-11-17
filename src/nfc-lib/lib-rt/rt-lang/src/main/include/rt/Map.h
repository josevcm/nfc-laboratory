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

#ifndef RT_MAP_H
#define RT_MAP_H

#include <map>
#include <optional>

#include <rt/Variant.h>

namespace rt {

class Map
{
   public:

      typedef std::string Key;
      typedef Variant Value;

      typedef struct
      {
         Key key;
         Value value;
      } Entry;

   public:

      Map() = default;

      Map(std::initializer_list<Entry> values);

      Value &operator[](const Key &key)
      {
         return map[key];
      }

      //      inline std::optional<Value> get(const Key &key) const
      //      {
      //         auto it = map.find(key);
      //
      //         if (it == map.end())
      //            return {};
      //
      //         return it->second;
      //      }

      template <typename T>
      std::optional<T> get(const Key &key) const
      {
         auto it = map.find(key);

         if (it == map.end())
            return {};

         return *std::get_if<T>(&it->second);
      }

      void put(const Key &key, const Value &value)
      {
         map[key] = value;
      }

      operator std::string() const;

   private:

      std::map<Key, Value> map;
};

}

#endif
