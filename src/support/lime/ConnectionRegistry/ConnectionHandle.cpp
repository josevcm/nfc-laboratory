/**
    @file ConnectionHandle.cpp
    @author Lime Microsystems
    @brief Represent a connection to a device
*/

#include "ConnectionHandle.h"
#include <cctype>
#include <string>
#include <map>
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace lime;

/*******************************************************************
 * String parsing helpers
 ******************************************************************/
static std::string trim(const std::string &s)
{
    std::string out = s;
    while (not out.empty() and std::isspace(out[0])) out = out.substr(1);
    while (not out.empty() and std::isspace(out[out.size()-1])) out = out.substr(0, out.size()-1);
    return out;
}

static std::map<std::string, std::string> argsToMap(const std::string &args)
{
    std::map<std::string, std::string> kwmap;

    bool inKey = true;
    std::string key, val;
    for (size_t i = 0; i < args.size(); i++)
    {
        const char ch = args[i];
        if (inKey)
        {
            if (ch == '=') inKey = false;
            else if (ch == ',') inKey = true;
            else key += ch;
        }
        else
        {
            if (ch == ',') inKey = true;
            else val += ch;
        }
        if ((inKey and not val.empty()) or ((i+1) == args.size()))
        {
            key = trim(key);
            val = trim(val);
            if (not key.empty()) kwmap[key] = val;
            key = "";
            val = "";
        }
    }

    return kwmap;
}

/*******************************************************************
 * Connection handle implementation
 ******************************************************************/
ConnectionHandle::ConnectionHandle(void):
    index(-1)
{
    return;
}

ConnectionHandle::ConnectionHandle(const std::string &args):
    index(-1)
{
    auto kwmap = argsToMap("name="+args); //append name= since it was stripped in serialize
    if (kwmap.count("module") != 0) module = kwmap.at("module");
    if (kwmap.count("media") != 0) media = kwmap.at("media");
    if (kwmap.count("name") != 0) name = kwmap.at("name");
    if (kwmap.count("addr") != 0) addr = kwmap.at("addr");
    if (kwmap.count("serial") != 0) serial = kwmap.at("serial");
    if (kwmap.count("index") != 0) index = std::stoi(kwmap.at("index"));
}

std::string ConnectionHandle::serialize(void) const
{
    std::string out;
    if (not name.empty()) out += name;
    if (not media.empty()) out += ", media="+media;
    if (not module.empty()) out += ", module="+module;
    if (not addr.empty()) out += ", addr="+addr;
    if (not serial.empty()) out += ", serial="+serial;
    if (index != -1) out += ", index="+std::to_string(index);

    return out;
}

std::string ConnectionHandle::ToString(void) const
{
    //name and media format
    std::string out(name);
    if (not media.empty()) out += " [" + media + "]";

    //remove leading zeros for a displayable serial
    std::string trimmedSerial(serial);
    while (not trimmedSerial.empty() and trimmedSerial.at(0) == '0')
    {
        trimmedSerial = trimmedSerial.substr(1);
    }
    if (not trimmedSerial.empty()) out += " " + trimmedSerial;

    //backup condition if we are empty somehow
    if (out.empty()) out = this->serialize();

    return out;
}

bool operator==(const ConnectionHandle &lhs, const ConnectionHandle &rhs)
{
    return lhs.serialize() == rhs.serialize();
}
