#include "spi.h"
#include "IConnection.h"
#include <ciso646>

extern lime::IConnection* serPort;

bool batchActive = false;
#include <vector>
#include <string>
#include <fstream>
using namespace std;
std::string bName;
std::vector<uint16_t> bAddr;
std::vector<uint16_t> bData;
std::vector<uint16_t> bMask;

uint32_t readCnt = 0;
uint32_t writeCnt = 0;

//spiAddrReg might not have SPI write bit, add it here if necessary
void SPI_write(unsigned short spiAddrReg, unsigned short spiDataReg)
{
    ++writeCnt;
    uint32_t wrdata = spiAddrReg << 16 | spiDataReg;
    serPort->TransactSPI(0x10, &wrdata, nullptr, 1);
}

unsigned short SPI_read (unsigned short spiAddrReg)
{
    ++readCnt;
    uint32_t wrdata = spiAddrReg << 16;
    uint32_t rddata = 0;
    serPort->TransactSPI(0x10, &wrdata, &rddata, 1);
    return rddata & 0xFFFF;
}

void Modify_SPI_Reg_bits_WrOnly(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data, const uint16_t spiDataReg)
{
    const uint16_t spiMask = (~(~0u << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
    //spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits

    if(batchActive)
    {
    bool found = false;
    for(size_t i=0; i<bAddr.size(); ++i)
    {
        if(bAddr[i] == SPI_reg_addr)
        {
            found = true;
            bData[i] &= ~spiMask;
            bData[i] |= ((new_bits_data << (bits&0xF)) & spiMask);
            bMask[i] |= spiMask;
        }
    }
    if(!found)
    {
        bAddr.push_back(SPI_reg_addr);
        bData.push_back((new_bits_data << (bits&0xF)) & spiMask);
        bMask.push_back(spiMask);
    }
    }
    SPI_write(SPI_reg_addr, (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask)); //write modified data back to SPI reg
}

void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data)
{
    uint16_t spiDataReg = SPI_read(SPI_reg_addr); //read current SPI reg data
    const uint16_t spiMask = (~(~0u << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
    spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits

    if(batchActive)
    {
    bool found = false;
    for(size_t i=0; i<bAddr.size(); ++i)
    {
        if(bAddr[i] == SPI_reg_addr)
        {
            found = true;
            bData[i] &= ~spiMask;
            bData[i] |= ((new_bits_data << (bits&0xF)) & spiMask);
            bMask[i] |= spiMask;
        }
    }
    if(!found)
    {
        bAddr.push_back(SPI_reg_addr);
        bData.push_back((new_bits_data << (bits&0xF)) & spiMask);
        bMask.push_back(spiMask);
    }
    }
    SPI_write(SPI_reg_addr, spiDataReg); //write modified data back to SPI reg
}

uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits)
{
    return (SPI_read(SPI_reg_addr) & (~(~0u<<((bits>>4)+1)))) >> (bits&0xF); //shift bits to LSB
}


void SPI_read_batch(const uint16_t *addr, uint16_t *values, uint8_t cnt)
{
    readCnt += cnt;
    std::vector<uint32_t> wrdata(cnt);
    std::vector<uint32_t> rddata(cnt);
    for(int i=0; i<cnt; ++i)
        wrdata[i] = addr[i] << 16;
    serPort->TransactSPI(0x10, wrdata.data(), rddata.data(), wrdata.size());
    for(int i=0; i<cnt; ++i)
        values[i] = rddata[i];
}
void SPI_write_batch(const uint16_t *addr, const uint16_t *values, uint8_t cnt)
{
    writeCnt += cnt;
    std::vector<uint32_t> data(cnt);
    for(int i=0; i<cnt; ++i)
    {
        data[i] = addr[i] << 16 | values[i];

        if(batchActive)
        {
            bool found = false;
            for(size_t j=0; j<bAddr.size(); ++j)
            {
                if(bAddr[j] == addr[i])
                {
                    found = true;
                    bData[i] = values[i];
                    bMask[i] = 0xFFFF;
                }
            }
            if(!found)
            {
                bAddr.push_back(addr[i]);
                bData.push_back(values[i]);
                bMask.push_back(0xFFFF);
            }
        }
    }
    serPort->TransactSPI(0x10, data.data(), nullptr, data.size());
}

int BeginBatch(const char* name)
{
    if(batchActive)
        return -1;
    batchActive = true;
    bAddr.clear();
    bData.clear();
    bMask.clear();
    bName = name;
    return 0;
}

void EndBatch()
{
    batchActive = false;
    //sort batch
    for(size_t i=0; i<bAddr.size(); ++i)
    {
        for(size_t j=i; j<bAddr.size(); ++j)
        {
            if(bAddr[i] > bAddr[j])
            {
                uint16_t temp;
                temp = bAddr[j];
                bAddr[j] = bAddr[i];
                bAddr[i] = temp;

                temp = bData[j];
                bData[j] = bData[i];
                bData[i] = temp;

                temp = bMask[j];
                bMask[j] = bMask[i];
                bMask[i] = temp;
            }
        }
    }

    vector<uint16_t> zeroValued;
    for(int i=0; i<bAddr.size(); ++i)
    {
        if(bMask[i] == 0xFFFF && bData[i] == 0x0)
            zeroValued.push_back(i);
    }
    for(int i=zeroValued.size()-1; i>=0; --i)
    {
        bAddr.push_back(bAddr[zeroValued[i]]); // move zero valued registers to end
        bAddr.erase(bAddr.begin()+zeroValued[i]);
        bMask.erase(bMask.begin()+zeroValued[i]);
        bData.erase(bData.begin()+zeroValued[i]);
    }

    char temp[64];
    std::fstream fout;
    fout.open("Batches.txt", ios::out | ios::app);
    fout << "\ncode const uint16_t " << bName << "Addr[] = {";
    bool skip = false;
    int i=0;
    for(auto val : bAddr)
    {
        if(bMask[i++] == 0xFFFF && ! skip)
            continue;
        sprintf(temp, "0x%04X,", val);
        fout << temp;
    }
    fout << "};";
    fout << "\ncode const uint16_t " << bName << "Data[] = {";
    i=0;
    for(auto val : bData)
    {
        if(bMask[i++] == 0xFFFF && !skip)
            continue;
        sprintf(temp, "0x%04X,", val);
        fout << temp;
    }
    fout << "};";
    fout << "\ncode const uint16_t " << bName << "Mask[] = {";
    i=0;
    for(auto val : bMask)
    {
        if(bMask[i++] == 0xFFFF && !skip)
            continue;
        sprintf(temp, "0x%04X,", val);
        fout << temp;
    }
    fout << "};";

    fout << "\n\ncode const uint16_t " << bName << "AddrWrOnly[] = {";
    i=0;
    for(auto val : bAddr)
    {
        if(bMask[i++] != 0xFFFF)
            continue;
        sprintf(temp, "0x%04X,", val);
        fout << temp;
    }
    fout << "};";
    fout << "\ncode const uint16_t " << bName << "DataWrOnly[] = {";
    i=0;
    for(auto val : bData)
    {
        if(bMask[i++] != 0xFFFF)
            continue;
        sprintf(temp, "0x%04X,", val);
        fout << temp;
    }
    fout << "};";
    fout.close();
}

/** @brief Modifies given registers with values applied using masks
    @param addr array of register addresses
    @param masks array of applied masks
    @param values array of values to be written
    @param start starting index of given arrays
    @param stop end index of given arrays
*/
void Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *values, const uint16_t *masks, uint8_t cnt)
{
    uint8_t i;
    for(i=0; i<cnt; ++i)
        SPI_write(addr[i], ( SPI_read(addr[i]) & ~masks[i] ) | (values[i] & masks[i]));
}
