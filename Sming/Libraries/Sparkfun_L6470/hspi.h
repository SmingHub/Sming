#ifndef INCLUDE_HSPI_H_
#define INCLUDE_HSPI_H_

#include "spi_register.h" // from 0.9.4 IoT_Demo
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

//#define SPI         0
#define HSPI        1
#define SPIFIFOSIZE 16 //16 words length

extern uint32_t *spi_fifo;

extern void hspi_init(void);
extern void hspi_send_data(const uint8_t * data, uint8_t datasize);
extern void hspi_send_uint16_r(const uint16_t data, int32_t repeats);
inline void hspi_wait_ready(void){while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);}

inline void hspi_prepare_tx(uint32_t bytecount)
{
	uint32_t bitcount = bytecount * 8 - 1;

	WRITE_PERI_REG(SPI_FLASH_USER1(HSPI), (bitcount & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S);
}


inline void hspi_start_tx()
{
	SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);   // send
}

inline void hspi_send_uint8(uint8_t data)
{
	hspi_prepare_tx(1);
	*spi_fifo = data;
	hspi_start_tx();
}

inline void hspi_send_uint16(uint16_t data)
{
	hspi_prepare_tx(2);
	*spi_fifo = data;
	hspi_start_tx();
}

inline void hspi_send_uint32(uint32_t data)
{
	hspi_prepare_tx(4);
	*spi_fifo = data;
	hspi_start_tx();
}

#endif /* INCLUDE_HSPI_H_ */
