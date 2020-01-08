#include "LMS7002M_RegistersMap.h"
#include "LMS7002M_parameters.h"
using namespace lime;

LMS7002M_RegistersMap::LMS7002M_RegistersMap()
{

}

LMS7002M_RegistersMap::~LMS7002M_RegistersMap()
{

}

uint16_t LMS7002M_RegistersMap::GetDefaultValue(uint16_t address) const
{
    auto iter = mChannelA.find(address);
    if( iter != mChannelA.end())
        return iter->second.defaultValue;
    else
        return 0;
}

void LMS7002M_RegistersMap::InitializeDefaultValues(const std::vector<const LMS7Parameter*> parameterList)
{
    for(auto parameter : parameterList)
    {
        uint16_t regValue = mChannelA[parameter->address].defaultValue;
        mChannelA[parameter->address].defaultValue = regValue | (parameter->defaultValue << parameter->lsb);
        mChannelA[parameter->address].value = mChannelA[parameter->address].defaultValue;
        if(parameter->address >= 0x0100)
            mChannelB[parameter->address].value = mChannelA[parameter->address].value;
    }
    //add NCO/PHO registers
    const uint16_t addr = 0x0242;
    for (int i = 0; i < 32; ++i)
    {
        mChannelA[addr + i].defaultValue = 0;
        mChannelA[addr + i].value = 0;
        mChannelB[addr + i].defaultValue = 0;
        mChannelB[addr + i].value = 0;
        mChannelA[addr + i + 0x0200].defaultValue = 0;
        mChannelA[addr + i + 0x0200].value = 0;
        mChannelB[addr + i + 0x0200].defaultValue = 0;
        mChannelB[addr + i + 0x0200].value = 0;
    }

    //add GFIRS
    std::vector<std::pair<uint16_t, uint16_t> > intervals = {
        {0x0280, 0x02A7},
        {0x02C0, 0x02E7},
        {0x0300, 0x0327},
        {0x0340, 0x0367},
        {0x0380, 0x03A7},
    };
    for (const auto &range : intervals)
    {
        for(int i=range.first; i<=range.second; ++i)
        {
            mChannelA[i].defaultValue = 0;
            mChannelA[i].value = 0;
            mChannelB[i].defaultValue = 0;
            mChannelB[i].value = 0;

            mChannelA[i+0x0200].defaultValue = 0;
            mChannelA[i+0x0200].value = 0;
            mChannelB[i+0x0200].defaultValue = 0;
            mChannelB[i+0x0200].value = 0;
        }
    }
}

void LMS7002M_RegistersMap::SetValue(uint8_t channel, const uint16_t address, const uint16_t value)
{
    if(channel == 0)
        mChannelA[address].value = value;
    else if(channel == 1)
        mChannelB[address].value = value;
}

uint16_t LMS7002M_RegistersMap::GetValue(uint8_t channel, uint16_t address) const
{
    const std::map<const uint16_t, Register> *regMap(nullptr);
    if(channel == 0)
        regMap = &mChannelA;
    else if(channel == 1)
        regMap = &mChannelB;
    else
        return 0;
    std::map<const uint16_t, Register>::const_iterator iter;
    iter = regMap->find(address);
    if (iter != regMap->end())
        return iter->second.value;
    else
        return 0;
}

std::vector<uint16_t> LMS7002M_RegistersMap::GetUsedAddresses(const uint8_t channel) const
{
    std::vector<uint16_t> addresses;
    if(channel == 0)
        for(auto iter : mChannelA)
            addresses.push_back(iter.first);
    else if(channel == 1)
        for(auto iter : mChannelB)
            addresses.push_back(iter.first);
    return addresses;
}
