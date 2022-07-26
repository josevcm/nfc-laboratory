#include <iostream>
#include <cstring>
#include <stdint.h>
#include "../INI.h"

using namespace std;

void centerString(string str); //Printing to console
std::string getStringFromFile(const std::string& path); //Source for data loading from memory.

int main()
{
   ///Declare
   typedef INI<> ini_t;  //Makes things shorter/easier to write <Section, Key, Value>
   //or
   //typedef INI<string, string, string> ini_t;  //Equivelant to previous line when wide characters are disabled
   ini_t ini("file.ini", true);  //File to open/default save filename. The constuctor is set to parse by default, unless specified as false

   ///Manipulate and access contents
   centerString("########## Access & Manipulate Contents ##########");

   //Common usage
   ini.create("Section 1");
   ini.create("Section 2");
   ini.get("Key1", "DefaultValue");
   ini.select("Section 1");
   ini.set("Key2", "Value");
   ini.save(); //Save contents to file, optional filename parameter available
   ini.clear(); //Clear INI contents from memory

   //Extended usage
   ini["Section Name"]["Key"] = "Value";  //You are not required to create a section first

   ini.create("Section1");  //Also selects as current section
   ini.create("Section2");  //Current

   ini.set("Key1", "Value1");  //Added pair under section "Section2"

   ini.select("Section1");  //Current
   cout << ini.get("Key1", "-1") << endl;  //Returns "-1" as no key exists, no default will return NULL for data type, eg int() is 0

   ini.select("Section2");
   ini.set("Key1", "1.123");
   cout << ini.get("Key1", -1.0) << endl;  //Return value as double
   ini.set(123, 123);  //Will convert to provided INI data type for key/value, in this case string for both

   ini.save();
   ini.clear();
   ini.parse();  //Parses file into objects in memory

   cout << ini["Section2"]["Key1"] << endl;  //Returns "Value1", slightly more overhead involved seeking section, avoid using excessively

   ///Iterate through sections and keys for both C++11 and C++98
   centerString("########## Iterate Contents ##########");

#ifdef FINI_CPP11
   for(auto i: ini.sections)
   {
      cout << "[" << i.first << "]" << endl;

      //for(auto j = i.second->begin(); j != i.second->end(); j++)
      for(auto j: *i.second)
      {
         cout << " " << j.first << "=" << j.second << endl;
      }
   }
#else
   for(ini_t::sectionsit_t i = ini.sections.begin(); i != ini.sections.end(); i++)
   {
      //Section name as ini_t::section_t
      cout << i->first << endl;

      if (i->second->size() == 0)  //No keys/values in section, skip to next
         continue;

      for(ini_t::keysit_t j = i->second->begin(); j != i->second->end(); j++)
      {
         //Name as ini_t::key_t & Value as ini_t::key_t
         cout << " " << j->first << "=" << j->second << endl;
      }
   }
#endif

///Example with different data types
   typedef INI <unsigned char, string, float> ini_int_t;  //Makes things shorter/easier to write <Section, Key, Value>
   ini_int_t ini_int("file_ints.ini", false);  //File to open/default save filename. The constuctor is set to parse by default, unless specified as false
   for(int i = 1; i <= 200; i++)
   {
      ini_int.create(i); //Section
      ini_int.set("Key", i / 2.f);
   }

   ini_int.save();

///Wide char support example (please define FINI_WIDE_SUPPORT in project)
   /*
   ini_t ini_w("file.ini", true);
   wcout << ini_w[L"Section2"][L"Key1"] << endl;
   */

///Load from memory
   std::string str = getStringFromFile("config/test.ini"); //Allows us to tap into a source for the purpose of this example

   ini_t ini_mem((void*)str.c_str(), str.size(), true); //This is the line which parses data from memory

///Merge contents and keep values
   ini_t inid("file.ini", true);
   ini_t inis("merge.ini", true);
   inid.merge(inis, true);
   inid.save("merged.ini");

   return EXIT_SUCCESS;
}

void centerString(string str)
{
   const char* s = str.c_str();
   int l = strlen(s);
   int pos = (int)((80 - l) / 2);
   for(int i = 0; i < pos; i++)
   cout << " ";
   cout << s << endl;
}

std::string getStringFromFile(const std::string& path) {
  std::ostringstream buf;
  std::ifstream input (path.c_str());
  buf << input.rdbuf();
  return buf.str();
}
