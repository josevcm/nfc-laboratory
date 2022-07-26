#ifndef SPI_H
#define SPI_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void SPI_write_slow(uint16_t spiAddrReg, uint16_t spiDataReg);
extern uint16_t SPI_read_slow (uint16_t spiAddrReg);

extern void SPI_write(uint16_t spiAddrReg, uint16_t spiDataReg);
extern uint16_t SPI_read (uint16_t spiAddrReg);

extern void SPI_writeSlow(uint16_t spiAddrReg, uint16_t spiDataReg);
extern uint16_t SPI_readSlow(uint16_t spiAddrReg);

extern void Modify_SPI_Reg_bits_WrOnly(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data, const uint16_t initialValue);
extern void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data);
extern uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits);

extern void SPI_read_batch(const uint16_t *addr, uint16_t *values, uint8_t cnt);
extern void SPI_write_batch(const uint16_t * addr, const uint16_t * values, uint8_t cnt);

extern void Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *values, const uint16_t *masks, uint8_t cnt);

extern int BeginBatch(const char* name);
extern void EndBatch();

#ifdef __cplusplus
}
#endif

#endif
