#include "mcu.h"
#include "spi.h"

#ifdef __cplusplus
    #include <chrono>
    #include <stdio.h>
    #include <thread>
    #include <chrono>
using namespace std;
#endif

void MCU_RunProcedure(uint8_t id)
{
    const uint16_t x0002reg = SPI_read(0x0002) & 0xFF;
    const uint16_t interupt6 = 0x0008;
    const uint16_t addrs[5] = {0x0006, 0x0, 0x0002, 0x0002, 0x0002};
    const uint16_t values[5] = {
        (uint16_t)(1),
        (uint16_t)(id),
        (uint16_t)(x0002reg & ~interupt6),
        (uint16_t)(x0002reg | interupt6),
        (uint16_t)(x0002reg & ~interupt6)};
    SPI_write_batch(addrs, values, 5);
    SPI_read(0x0002);
}

uint8_t MCU_WaitForStatus(uint16_t timeout_ms)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    unsigned short value = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    do {
        value = SPI_read(0x0001) & 0xFF;
        if (value != 0xFF) //working
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        t2 = std::chrono::high_resolution_clock::now();
    }while (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms);
    SPI_write(0x0006, 0); //return SPI control to PC
    return value & 0x7F;
}

uint8_t MCU_SetParameter(MCU_Parameter param, float value)
{
    uint8_t x0002reg = SPI_read(0x0002);
    const uint8_t interupt7 = 0x04;
    if(param==MCU_REF_CLK || param == MCU_BW)
    {
        uint8_t inputRegs[3];
        value /= 1e6;
        inputRegs[0] = (uint8_t)value; //frequency integer part
        uint16_t fracPart = value * 1000.0 - inputRegs[0]*1000.0;
        inputRegs[1] = (fracPart >> 8) & 0xFF;
        inputRegs[2] = fracPart & 0xFF;
        for(uint8_t i = 0; i < 3; ++i)
        {
            SPI_write(0, inputRegs[2-i]);
            SPI_write(0x0002, x0002reg | interupt7);
            SPI_write(0x0002, x0002reg & ~interupt7);
            int status = MCU_WaitForStatus(10);
            if(status != 0)
                printf("MCU error status 0x%02X\n", status);
        }
    }
    if(param==MCU_REF_CLK)
        MCU_RunProcedure(4);
    if(param == MCU_BW)
        MCU_RunProcedure(3);
    if(param == MCU_EXT_LOOPBACK_PAIR)
    {
        uint8_t intVal = (int)value;
        SPI_write(0, intVal);
        SPI_write(0x0002, x0002reg | interupt7);
        SPI_write(0x0002, x0002reg & ~interupt7);
        int status = MCU_WaitForStatus(10);
        if(status != 0)
            printf("MCU error status 0x%02X\n", status);
        MCU_RunProcedure(9);
    }
    int status = MCU_WaitForStatus(100);
    if(status != 0)
            printf("MCU error status 0x%02X\n", status);
    return status;
}

uint8_t MCU_UploadProgram(const uint8_t* binImage, const uint16_t len)
{
#ifndef NDEBUG
    auto timeStart = std::chrono::high_resolution_clock::now();
#endif // NDEBUG
    const auto timeout = std::chrono::milliseconds(100);
    uint16_t i;
    const uint32_t controlAddr = 0x0002;
    const uint32_t statusReg = 0x0003;
    const uint32_t addrDTM = 0x0004; //data to MCU
    const uint16_t EMTPY_WRITE_BUFF = 1 << 0;
    const uint16_t PROGRAMMED = 1 << 6;
    const uint8_t fifoLen = 32;

    //reset MCU, set mode
    SPI_write(controlAddr, 0);
    SPI_write(controlAddr, 2 & 0x3); //SRAM

    for(i=0; i<len; i+=fifoLen)
    {
        //wait till EMPTY_WRITE_BUFF = 1
        bool fifoEmpty = false;
        auto t1 = std::chrono::high_resolution_clock::now();
        auto t2 = t1;
        do{
            fifoEmpty = SPI_read(statusReg) & EMTPY_WRITE_BUFF;
            t2 = std::chrono::high_resolution_clock::now();
        }while( !fifoEmpty && (t2-t1)<timeout);

        if(!fifoEmpty)
            return -1;//ReportError(ETIMEDOUT, "MCU FIFO full");

        //write 32 bytes into FIFO
        {
            uint8_t j;
            uint16_t addr[fifoLen];
            uint16_t data[fifoLen];
            for(j=0; j<fifoLen; ++j)
            {
                addr[j] = addrDTM;
                data[j] = binImage[i+j];
            }
            SPI_write_batch(addr, data, fifoLen);
        }
#ifndef NDEBUG
        printf("MCU programming : %4i/%4i\r", i+fifoLen, len);
#endif
    }

    //wait until programmed flag
    {
        bool programmed = false;
        auto t1 = std::chrono::high_resolution_clock::now();
        auto t2 = t1;
        do{
            programmed = SPI_read(statusReg) & PROGRAMMED;
            t2 = std::chrono::high_resolution_clock::now();
        }while( !programmed && (t2-t1)<timeout);

        if(!programmed)
            return -1;//ReportError(ETIMEDOUT, "MCU not programmed");
    }
#ifndef NDEBUG
    auto timeEnd = std::chrono::high_resolution_clock::now();
    printf("\nMCU Programming finished, %li ms\n",
            std::chrono::duration_cast<std::chrono::milliseconds>
            (timeEnd-timeStart).count());
#endif //NDEBUG
    return 0;
}
