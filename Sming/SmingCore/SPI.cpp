/*
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 *
 *  Some code is derived from:
 *  	David Ogilvy (MetalPhreak)
 *
 */

#include "SPI.h"

#include <SmingCore.h>
#include <stdlib.h>
#include "eagle_soc.h"
#include "espinc/spi_register.h"
#include "espinc/c_types_compatible.h"

// define the static singleton
SPIClass SPI;

SPIClass::SPIClass() {
	_SPISettings = SPISettings(4000000, MSBFIRST, SPI_MODE1);;
}

SPIClass::~SPIClass() {
	// TODO Auto-generated destructor stub
}

/*
 * 	Initialize HW SPI with HW SS (PIN 15)
 */
void SPIClass::begin() {

#ifdef SPI_DEBUG
	debugf("ESP clock frequency %i", ets_get_cpu_frequency());
	debugf("SPIhw::begin()");
#endif

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); //GPIO12 is HSPI MISO pin (Master Data In)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //GPIO13 is HSPI MOSI pin (Master Data Out)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //GPIO14 is HSPI CLK pin (Clock)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); //GPIO15 is HSPI CS pin (Chip Select / Slave Select)

	prepare(_SPISettings);
}




/*
 * end(): Disables the SPI bus (leaving pin modes unchanged).
 */
void SPIClass::end() {

//#ifdef SPI_DEBUG
//	debugf("SPIhw::end()");
//#endif
};


/*
 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
 */
void SPIClass::beginTransaction(SPISettings mySettings) {

//#ifdef SPI_DEBUG
//	debugf("SPIhw::beginTransaction(SPISettings mySettings)");
//#endif

	// prepare SPI settings
	prepare(mySettings);
}

/*
 * endTransaction(): Stop using the SPI bus. Normally this is called after
 * de-asserting the chip select, to allow other libraries to use the SPI bus.
 */
void SPIClass::endTransaction() {

//#ifdef SPI_DEBUG
//	debugf("SPIhw::endTransaction()");
//#endif

};


// Receive numberBytes byte of data
// use SetCS and enable() for SW controlled CS GPIO Ports
void SPIClass::transfer(uint8 * buffer, size_t numberBytes) {

#define BLOCKSIZE 64		// the max length of the ESP SPI_W0 registers

	uint16 bufIndx = 0;
	uint8 bufLenght = 0;
	uint8 din_bits = 0;

	int blocks = ((numberBytes -1)/BLOCKSIZE)+1;
	int total = blocks;

	// loop number of blocks
	while  (blocks--) {

		// get full BLOCKSIZE or number of remaining bytes
		bufLenght = min(numberBytes-bufIndx, BLOCKSIZE);

#ifdef SPI_DEBUG
		debugf("Read Block %d total %d bytes", total-blocks, bufLenght);
#endif

		// compute the number of bits to clock
		din_bits = bufLenght * 8;

		// wait for SPI bus to be ready
		while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

		// clear registers
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);
		// enable MISO
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MISO);
		// setup bit lenght
		WRITE_PERI_REG(SPI_USER1(SPI_NO),
				((din_bits-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S );

		// Begin SPI Transaction
		SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);

		// wait for SPI bus to be ready
		while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

		// wait a while before reading the register into the buffer
		delayMicroseconds(4*din_bits/10 - din_bits/5);

		// copy the registers starting from last index position
		memcpy (&buffer[bufIndx], (void *)SPI_W0(SPI_NO),  bufLenght);

		// increment bufIndex
		bufIndx += bufLenght;
	}
};



// Private method implementation
void SPIClass::prepare(SPISettings mySettings) {

	// TODO: use settings instead of defaults
//#ifdef SPI_DEBUG
//	debugf("SPIhw::prepare(SPISettings mySettings)");
//	_SPISettings.print("_SPISettings");
//	mySettings.print("mySettings");
//#endif

	// check if we need to change settings
	if (_SPISettings == mySettings)
		return;

	//  setup clock
	setFrequency(mySettings._speed);

	//  Setup Byte Order
	uint8 byte_order = SPI_BYTE_ORDER_LOW_TO_HIGH;
	if(byte_order){
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
	} else {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
	}

	// setup mode
	//				spi_cpha - (0) Data is valid on clock leading edge
	//				           (1) Data is valid on clock trailing edge
	//				spi_cpol - (0) Clock is low when inactive
	//				           (1) Clock is high when inactive
	uint8 spi_cpha = 0;
	uint8 spi_cpol = 0;

	if(spi_cpha) {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_CK_OUT_EDGE);
	} else {
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_CK_OUT_EDGE);
	}

	if (spi_cpol) {
		SET_PERI_REG_MASK(SPI_PIN(SPI_NO), SPI_IDLE_EDGE);
	} else {
		CLEAR_PERI_REG_MASK(SPI_PIN(SPI_NO), SPI_IDLE_EDGE);
	}

	SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_CS_SETUP|SPI_CS_HOLD);
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_FLASH_MODE);

#ifdef SPI_DEBUG
	debugf("SPIhw::prepare(SPISettings mySettings) -> updated settings");
#endif
	_SPISettings = mySettings;
};

// send out a single byte of data
// cs = 0 is using hardware SS any other pin handles Software controlled client select
unsigned char SPIClass::transfer(unsigned char data){


//#ifdef SPI_DEBUG
//	debugf("SPIhw::transfer(unsigned long %X)", data);
//#endif

	// wait for SPI bus to be ready
	while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

	// clear registers
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

	// enable MOSI function in SPI module
	SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI);
	// Setup Bitlengths
//	WRITE_PERI_REG(SPI_USER1(SPI_NO), ((8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S );

	// copy data to W0
	WRITE_PERI_REG(SPI_W0(SPI_NO), data);

//	// enable MISO function in SPI module
	SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MISO);

//	// Setup Bitlengths MISO
	WRITE_PERI_REG(SPI_USER1(SPI_NO), ((8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S ); //|
//									  ((8-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S );  //Number of bits to receive

	// Begin SPI Transaction
	SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);

	// wait for SPI bus to be ready
	while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

	data = READ_PERI_REG(SPI_W0(SPI_NO)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!

//  wait for data transmitted before disabling Client Selector
//  Delay = 4 MHz * num bit / convert to micro - HW-Processing Time(factor of num bit)
//	delayMicroseconds(2);

	return data;
};



/*
 * transfer32()
 *
 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16). In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
 *
 * 		receivedVal = SPI.transfer(long val, uint8 numerBytes)
 */
uint32 SPIClass::transfer32(uint32 val, uint8 bytes) {

#ifdef SPI_DEBUG
	debugf("SPIhw::transfer32(unsigned long %X,%d bytes)", val, bytes);
#endif

		uint8 numBits = bytes * 4;

		// wait for SPI bus to be ready
		while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

		// disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

		// Write val

		// enable MOSI function in SPI module
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI);

		//copy data to W0
		if(READ_PERI_REG(SPI_USER(SPI_NO))&SPI_WR_BYTE_ORDER) {
			WRITE_PERI_REG(SPI_W0(SPI_NO), val<<(32-numBits));
		} else {
			WRITE_PERI_REG(SPI_W0(SPI_NO), val);
		}
		// copy data to W0
//		WRITE_PERI_REG(SPI_W0(SPI_NO), val);


		// Read val

		// enable MISO function in SPI module
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MISO);

		// Setup Bitlengths MOSI, MISO
		WRITE_PERI_REG(SPI_USER1(SPI_NO),((numBits-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S |   //Number of bits to Send
										 ((numBits-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S );  //Number of bits to receive

		// Begin SPI Transaction
		SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);

		// Read Data

		// wait for SPI bus to be ready
		while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

		// Read SPI_WO register
		if(READ_PERI_REG(SPI_USER(SPI_NO))&SPI_RD_BYTE_ORDER) {
			return READ_PERI_REG(SPI_W0(SPI_NO)) >> (32-numBits); //Assuming data in is written to MSB. TBC
		} else {
			return READ_PERI_REG(SPI_W0(SPI_NO)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!
		}

};


void SPIClass::setClock(uint8 prediv, uint8 cntdiv) {

	if((prediv==0)|(cntdiv==0)){
		// go full speed = SYSTEMCLOCK
		WRITE_PERI_REG(SPI_CLOCK(SPI_NO), SPI_CLK_EQU_SYSCLK);
	} else {
		WRITE_PERI_REG(SPI_CLOCK(SPI_NO),
					(((prediv-1)&SPI_CLKDIV_PRE)<<SPI_CLKDIV_PRE_S)|
					(((cntdiv-1)&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
					(((cntdiv>>1)&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
					((0&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));
	}
}

int CPU_MAX = 8000000UL;


// div from stdlib
div_t div (int numer, int denom) {
  div_t result;
  result.quot = numer / denom;
  result.rem = numer % denom;

  if (numer >= 0 && result.rem < 0)
    {
      ++result.quot;
      result.rem -= denom;
    }
  return result;
}

uint32_t SPIClass::getFrequency(int freq, uint8 &pre, uint8 clk) {

	int divider = CPU_MAX / freq;

	div_t divresult = div (divider,clk);
	pre = divresult.quot;

	int f = CPU_MAX/pre/clk;
	while (f > freq) {
		pre++;
		f = CPU_MAX/pre/clk;
	}

//	printf ("-> freq %d result %d devider %d pre %d clk %d\n", freq, f, pre*clk, pre, clk);

	return f;
}

void SPIClass::setFrequency(uint32_t freq) {

	// dont run code if there are no changes
	if (freq == _SPISettings._speed) return;

	_SPISettings._speed = freq;

	uint8 pre2;
	uint32_t f2 = getFrequency(freq, pre2, 2);
	if (f2 == freq) {
		debugf ("-> Hit!! -> target freq %d -> result %d", freq, CPU_MAX/pre2/2);
		setClock(pre2, 2);
		return;
	}

	uint8 pre3;
	uint32_t f3  = getFrequency(freq, pre3, 3);
	uint8 pre5;
	uint32_t f5 = getFrequency(freq, pre5, 5);
	if (f3 <= f2 && f2 >= f5) {
		debugf ("-> Hit!! -> target freq %d -> result %d", freq, CPU_MAX/pre2/2);
		setClock(pre2, 2);
		return;
	} else {
		if (f5 <= f3) {
			debugf ("-> Hit!! -> target freq %d -> result %d", freq, CPU_MAX/pre3/3);
			debugf ("  setClock(%d, %d)\n", pre3, 3);
			setClock(pre3, 3);
			return;
		} else {
			debugf ("-> Hit!! -> target freq %d -> result %d", freq, CPU_MAX/pre5/5);
			debugf ("  setClock(%d, %d)\n", pre5, 5);
			setClock(pre5, 5);
			return;
		}
	}

}




