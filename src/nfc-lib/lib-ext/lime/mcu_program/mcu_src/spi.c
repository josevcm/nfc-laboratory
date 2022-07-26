#include "spi.h"
#include "LMS7002_REGx51.h"

void Delay()
{
    TR0 = 0; //stop timer 0
    TH0 = 0xFF;
    TL0 = 0xF0;
    TF0 = 0; //clear overflow
    TR0 = 1; //start timer
    while( !TF0 ); //wait for timer overflow
}

void SPI_transferVariable(unsigned short value)
{								  
	uint8_t spiIter;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=0;	//set Clock low
		ucSDIN = value & 0x8000; //if current bit is 1 set Output High
		value <<= 1; //shift mask to right
		ucSCLK=1; 	//set Clock high
	}
	ucSCLK=0;	//set Clock low
}

void SPI_transferVariable_slow(unsigned short value)
{
	uint8_t spiIter;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=0;	//set Clock low
        Delay();
		ucSDIN = value & 0x8000; //if current bit is 1 set Output High
        Delay();
		value <<= 1; //shift mask to right
		ucSCLK=1; 	//set Clock high
        Delay();
	}
	ucSCLK=0;	//set Clock low
}

void SPI_write(const unsigned short spiAddrReg, const unsigned short spiDataReg)
{
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable(spiAddrReg | 0x8000); //set write bit
	//write data
	SPI_transferVariable(spiDataReg);
	ucSEN=1;
	ucSDIN=1;
}

void SPI_write_slow(const unsigned short spiAddrReg, const unsigned short spiDataReg)
{
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable_slow(spiAddrReg | 0x8000); //set write bit
	//write data
	SPI_transferVariable_slow(spiDataReg);
	ucSEN=1;
	ucSDIN=1;
}

unsigned short SPI_read_slow (const unsigned short spiAddrReg)
{
	uint8_t spiIter;	
	uint16_t spiDataReg = 0;
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable_slow(spiAddrReg & ~0x8000);	//clear write bit
	ucSDIN=1;
	//read data
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=1; 	//set Clock high
        Delay();
		spiDataReg <<= 1;
		if (ucSDOUT)
			spiDataReg |= 1;
		ucSCLK=0;	//set Clock low
        Delay();
	}
	ucSEN=1;
	ucSDIN=1;
	return spiDataReg;
}

unsigned short SPI_read (const unsigned short spiAddrReg)
{
	uint8_t spiIter;
	uint16_t spiDataReg = 0;
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable(spiAddrReg & ~0x8000);	//clear write bit
	ucSDIN=1;
	//read data
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=1; 	//set Clock high
		spiDataReg <<= 1;
		if (ucSDOUT)
			spiDataReg |= 1;
		ucSCLK=0;	//set Clock low
	}
	ucSEN=1;
	ucSDIN=1;
	return spiDataReg;
}

void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data)
{
	uint16_t spiDataReg = SPI_read(SPI_reg_addr); //read current SPI reg data
	const uint16_t spiMask = (~(~0 << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
	spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits
	SPI_write(SPI_reg_addr, spiDataReg); //write modified data back to SPI reg
}

uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits)
{
	return (SPI_read(SPI_reg_addr) & (~(~0<<((bits>>4)+1)))) >> (bits&0xF); //shift bits to LSB
}
