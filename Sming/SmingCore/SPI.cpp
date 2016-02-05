/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// SPIClass code based on https://github.com/Perfer/esp8266_ili9341

#include "../SmingCore/SPI.h"
#include "../SmingCore/Digital.h"

SPIClass SPI(SPI_ID_HSPI);

SPIClass::SPIClass(uint8_t spiID) : id(spiID)
{
	// Only SPI_HSPI tested on hardware for now!
}

void SPIClass::begin() {
	SPIClass::begin(2, 4);
}

void SPIClass::begin(uint16_t predivider, uint8_t divider)
{
	//bit9 of PERIPHS_IO_MUX should be cleared when HSPI clock doesn't equal CPU clock
	//bit8 of PERIPHS_IO_MUX should be cleared when SPI clock doesn't equal CPU clock
	if (id == SPI_ID_MAIN)
	{
		WRITE_PERI_REG(PERIPHS_IO_MUX, 0x005); //clear bit9,and bit8

		PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, 1); //configure io to spi mode
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, 1); //configure io to spi mode
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, 1); //configure io to spi mode
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, 1); //configure io to spi mode
	}
	else if (id == SPI_ID_HSPI)
	{
		WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9

		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); // HSPIQ MISO == GPIO12
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // HSPID MOSI == GPIO13
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // CLK		 == GPIO14
	}
	else
		SYSTEM_ERROR("UNSUPPORTED SPI id: %d", id);


	// SPI clock = CPU clock / 10 / 4
	// time length HIGHT level = (CPU clock / 10 / 2) ^ -1,
	// time length LOW level = (CPU clock / 10 / 2) ^ -1
	// Frequency calculation: 80Mhz / predivider / divider
	if (predivider < 1 || predivider > 8192) {
		SYSTEM_ERROR("SPI PRE-DIVIDER [%d] OUT OF BOUNDS [1-8192]", predivider);
	}

	if (divider < 1 || divider > 64) {
		SYSTEM_ERROR("SPI DIVIDER [%d] OUT OF BOUNDS [1-64]", predivider);
	}

	WRITE_PERI_REG(SPI_FLASH_CLOCK(id),
		(((predivider-1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
		(((divider-1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
		(((divider / 2 - 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
		(((divider-1) & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
}

void SPIClass::writeData(uint8_t * data, uint8_t numberByte)
{
	uint8_t i = 0;
	uint8_t shift = 0;
	uint32_t *buffer = (uint32_t *)SPI_FLASH_C0(id);
	for (i = 0; i < numberByte; ++i)
	{
		if (shift >= 32)
		{
			shift = 0;
			buffer += 1;
		}
		*buffer &= ~( 0xFFUL << shift );
		*buffer |= ((uint32_t)data[i]) << shift;
		shift += 8;
	}
}

void SPIClass::readData(uint8_t * data, uint8_t numberByte)
{
	uint8_t i = 0;
	uint8_t shift = 0;
	uint32_t buffer = 0;

	for (i = 0; i < numberByte; ++i)
	{
		if ( (i % 4 == 0) )
		{
			buffer = ( *( (uint32_t *)SPI_FLASH_C0(id) + i/4 ) );
			shift = 0;
		}
		data[i] = (buffer >> shift) & 0xFF;
		shift += 8;
	}
}

void SPIClass::end()
{
}

void SPIClass::transfer(uint8_t * data, uint8_t count)
{
	uint32_t regvalue = 0;
	uint16_t numberBit = 0;

	while (READ_PERI_REG(SPI_FLASH_CMD(id))&SPI_FLASH_USR);

	regvalue |=  SPI_FLASH_DOUT | SPI_DOUTDIN | SPI_CK_I_EDGE;
	regvalue &= ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | SPI_FLASH_USR_DIN | SPI_USR_COMMAND); //clear bit 2 see example IoT_Demo
	WRITE_PERI_REG(SPI_FLASH_USER(id), regvalue);

	numberBit = count * 8 - 1;

	WRITE_PERI_REG(SPI_FLASH_USER1(id),
			( (numberBit & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S ) |
			( (numberBit & SPI_USR_DIN_BITLEN) << SPI_USR_DIN_BITLEN_S ) );

	writeData(data, count);

	SET_PERI_REG_MASK(SPI_FLASH_CMD(id), SPI_FLASH_USR);   // send

	while (READ_PERI_REG(SPI_FLASH_CMD(id)) & SPI_FLASH_USR);

	readData(data, count);
}

byte SPIClass::transfer(uint8_t data)
{
	transfer(&data, 1);
	return data;
}

/*void SPIClass::setBitOrder(uint8_t bitOrder)
{
	order = bitOrder;
}*/
