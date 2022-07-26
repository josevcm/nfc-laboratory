#ifndef LMS7002M_REGISTERS_MAP_H
#define LMS7002M_REGISTERS_MAP_H

#include <vector>
#include <map>
#include <cstdint>
struct LMS7Parameter;
namespace lime{



class LMS7002M_RegistersMap
{
public:
    struct Register
    {
        uint16_t value;
        uint16_t defaultValue;
        uint16_t mask;
    };

    LMS7002M_RegistersMap();
    ~LMS7002M_RegistersMap();

    uint16_t GetValue(uint8_t channel, uint16_t address) const;
    void SetValue(uint8_t channel, const uint16_t address, const uint16_t value);

    void InitializeDefaultValues(const std::vector<const LMS7Parameter*> parameterList);
    uint16_t GetDefaultValue(uint16_t address) const;
    std::vector<uint16_t> GetUsedAddresses(const uint8_t channel) const;

    LMS7002M_RegistersMap &operator=(const LMS7002M_RegistersMap &other)
    {
        mChannelA.insert(other.mChannelA.begin(), other.mChannelA.end());
        mChannelB.insert(other.mChannelB.begin(), other.mChannelB.end());
        return *this;
    }

protected:
    std::map<const uint16_t, Register> mChannelA;
    std::map<const uint16_t, Register> mChannelB;
};

}
#endif
