#include "mcuHexBin.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

typedef struct
{
	unsigned long m_startAddress;
	std::vector<unsigned char> m_bytes;
} MemBlock;

int MCU_HEX2BIN(const char* filename, const uint16_t limit, uint8_t *binImage, uint16_t *imgSize)
{
    vector<MemBlock>	m_chunks;
	unsigned long		m_top;

    FILE* m_file = fopen(filename, "r");
    if (m_file == NULL)
    {
        printf("File not found: %s\n", filename);
        return -1;
    }
    memset(binImage, 0, limit);

    const uint16_t cMaxLineLen = 1024;
    char szLine[cMaxLineLen];
    bool endSeen = false;
    bool linear = true;				// Only used for intel hex
    unsigned long addressBase = 0;	// Only used for intel hex
    while(!feof(m_file))
    {
        //memset(szLine, 0, cMaxLineLen);
        if(fgets(szLine, cMaxLineLen, m_file) == 0)
        {
            if (ferror(m_file))
                printf("Error reading input!\n");
            continue;
        }

        //clear \r\n
        for(int i=0; i<2; ++i)
        {
            int lastCharPos = strlen(szLine)-1;
            if (szLine[lastCharPos] == '\n' || szLine[lastCharPos] == '\r')
                szLine[lastCharPos] = 0;
        }

        if (strlen(szLine) == cMaxLineLen-1)
        {
            printf("Hex file lines to long!\n");
            return -2;
        }
        // Ignore blank lines
        if (szLine[0] == '\n' || szLine[0] == '\r')
            continue;

        // Detect format and warn if garbage lines are found
        if (szLine[0] != ':')
        {
            printf("Ignoring garbage line!\n");
            continue;
        }

        if(endSeen)
        {
            printf("Hex line after end of file record!\n");
            return -3;
        }

        unsigned long dataBytes;
        unsigned long startAddress;
        unsigned long type;
        if(sscanf(&szLine[1], "%2lx%4lx%2lx", &dataBytes, &startAddress, &type) != 3)
        {
            printf("Hex line beginning corrupt!\n");
            return -4;
        }
        // Check line length
        if (szLine[11 + dataBytes * 2] != '\n' && szLine[11 + dataBytes * 2] != 0)
        {
            printf("Hex line length incorrect!\n");
            return -6;
        }
        // Check line checksum
        uint8_t checkSum = 0;
        unsigned long tmp;
        for (unsigned int i = 0; i <= dataBytes + 4; ++i)
        {
            if (sscanf(&szLine[1 + i * 2], "%2lx", &tmp) != 1)
            {
                printf("Hex line data corrupt!\n");
                return -7;
            }
            checkSum += tmp;
        }
        if (checkSum != 0)
        {
            printf("Hex line checksum error!\n");
            return -8;
        }

        switch (type)
        {
        case 0: // Data record
            if (!linear)
            {
                // Segmented
                unsigned long test = startAddress;
                test += dataBytes;
                if (test > 0xffff)
                {
                    printf("Can't handle wrapped segments!\n");
                    return -9;
                }
            }
            if (!m_chunks.size() ||
                m_chunks.back().m_startAddress + m_chunks.back().m_bytes.size() !=
                addressBase + startAddress)
            {
                m_chunks.push_back(MemBlock());
                m_chunks.back().m_startAddress = addressBase + startAddress;
            }
            {
                unsigned char i = 0;
                for (i = 0; i < dataBytes; ++i)
                {
                    sscanf(&szLine[9 + i * 2], "%2lx", &tmp);
                    if (addressBase + startAddress + i > limit)
                    {
                        cout << "Ignoring data above address space!\n";
                        cout << "Data address: " << addressBase + startAddress + i;
                        cout << " Limit: " << limit << "\n";
                        if (!m_chunks.back().m_bytes.size())
                        {
                            m_chunks.pop_back();
                        }
                        continue;
                    }
                    m_chunks.back().m_bytes.push_back(tmp);
                }
            }
            break;

        case 1: // End-of-file record
            if (dataBytes != 0)
                printf("Warning: End of file record not zero length!\n");
            if (startAddress != 0)
                printf("Warning: End of file record address not zero!\n");
            endSeen = true;
            break;

        case 2: // Extended segment address record
            if(dataBytes != 2)
            {
                printf("Length field must be 2 in extended segment address record!\n");
                return -10;
            }
            if (startAddress != 0)
            {
                printf("Address field must be zero in extended segment address record!\n");
                return -11;
            }
            sscanf(&szLine[9], "%4lx", &startAddress);
            addressBase = startAddress << 4;
            linear = false;
            break;

        case 3: // Start segment address record
            if (dataBytes != 4)
                printf("Warning: Length field must be 4 in start segment address record!\n");
            if (startAddress != 0)
                printf("Warning: Address field must be zero in start segment address record!\n");
            if (dataBytes == 4)
            {
                unsigned long ssa;
                char	ssaStr[16];
                sscanf(&szLine[9], "%8lx", &ssa);
                sprintf(ssaStr, "%08lX\n", ssa);
                cout << "Segment start address (CS/IP): ";
                cout << ssaStr;
            }
            break;

        case 4: // Extended linear address record
            if (dataBytes != 2)
            {
                printf("Length field must be 2 in extended linear address record!\n");
                return -12;
            }
            if (startAddress != 0)
            {
                printf("Address field must be zero in extended linear address record!\n");
                return -13;
            }
            sscanf(&szLine[9], "%4lx", &startAddress);
            addressBase = ((unsigned long)startAddress) << 16;
            linear = true;
            break;

        case 5: // Start linear address record
            if (dataBytes != 4)
                printf("Warning: Length field must be 4 in start linear address record!\n");
            if (startAddress != 0)
                printf("Warning: Address field must be zero in start linear address record!\n");
            if (dataBytes == 4)
            {
                unsigned long lsa;
                char	lsaStr[16];
                sscanf(&szLine[9], "%8lx", &lsa);
                sprintf(lsaStr, "%08lX\n", lsa);
                cout << "Linear start address: ";
                cout << lsaStr;
            }
            break;

        default:
            printf("Waring: Unknown record found!\n");
        }
    }
    if (!endSeen)
        printf("No end of file record!\n");
    if (!m_chunks.size())
    {
        printf("No data in file!\n");
        return -14;
    }
    vector<MemBlock>::iterator	vi;
    m_top = 0;
    for (vi = m_chunks.begin(); vi < m_chunks.end(); vi++)
    {
        m_top = std::max<size_t>(m_top, vi->m_startAddress + vi->m_bytes.size() - 1);
    }
    if(binImage)
    {
        memset(binImage, 0, limit);
        for(auto i : m_chunks)
        {
            for(size_t j=0; j<i.m_bytes.size(); ++j)
                if(i.m_startAddress+j < limit)
                    binImage[i.m_startAddress+j] = i.m_bytes[j];
        }
    }
    if(imgSize)
        *imgSize = m_top;
    return 0;
}
