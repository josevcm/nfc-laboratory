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

#ifndef NFC_LAB_MAP_H
#define NFC_LAB_MAP_H

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

      inline Value &operator[](const Key &key)
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

      template<typename T>
      inline std::optional<T> get(const Key &key) const
      {
         auto it = map.find(key);

         if (it == map.end())
            return {};

         return *std::get_if<T>(&it->second);
      }

      inline void put(const Key &key, const Value &value)
      {
         map[key] = value;
      }

      operator std::string() const;

   private:

      std::map<Key, Value> map;
};

}

#endif
