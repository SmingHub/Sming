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
//	_SPISettings = SPISettings(4000000, MSBFIRST, SPI_MODE1);
	_SPISettings = this->SPIDefaultSettings;
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

	prepare(this->SPIDefaultSettings);
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
#ifdef SPI_DEBUG
	debugf("SPIhw::prepare(SPISettings mySettings)");
	_SPISettings.print("_SPISettings");
	mySettings.print("mySettings");
#endif

	// check if we need to change settings
	if (_init & _SPISettings == mySettings)
		return;

	//  setup clock
	setFrequency(mySettings._speed);

	//  Setup Byte Order  tx / rx
	spi_byte_order(SPI_BYTE_ORDER_HIGH_TO_LOW, SPI_BYTE_ORDER_HIGH_TO_LOW);

	// setup mode
	//				spi_cpha - (0) Data is valid on clock leading edge
	//				           (1) Data is valid on clock trailing edge
	//				spi_cpol - (0) Clock is low when inactive
	//				           (1) Clock is high when inactive

//	spi_mode(1,0);

#ifdef SPI_DEBUG
	debugf("SPIhw::prepare(SPISettings mySettings) -> updated settings");
#endif
	_SPISettings = mySettings;
	_init = true;
};

// send out a single byte of data
// cs = 0 is using hardware SS any other pin handles Software controlled client select
unsigned char SPIClass::transfer(unsigned char data){


//#ifdef SPI_DEBUG
//	debugf("SPIhw::transfer(unsigned long %X)", data);
//#endif
	uint32 d = (uint32) data;

	// wait for SPI bus to be ready
	while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);

	// clear registers
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

	// enable MOSI function in SPI module
	SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MOSI);

	// Setup Bitlengths
//	WRITE_PERI_REG(SPI_USER1(SPI_NO), ((8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S );

	// copy data to W0
	if(READ_PERI_REG(SPI_USER(SPI_NO))&SPI_WR_BYTE_ORDER) {
		WRITE_PERI_REG(SPI_W0(SPI_NO), d<<(32-8));
	} else {
			WRITE_PERI_REG(SPI_W0(SPI_NO), d);
	}


////	// enable MISO function in SPI module
	SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_USR_MISO);
//
////	// Setup Bitlengths MISO
//	WRITE_PERI_REG(SPI_USER1(SPI_NO), ((8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S |
//									  ((8-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S );  //Number of bits to receive
	WRITE_PERI_REG(SPI_USER1(SPI_NO),
				( (8-1 & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S ) |
				( (8-1 & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S ) );

	// Begin SPI Transaction
	SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);


	while(READ_PERI_REG(SPI_CMD(SPI_NO))&SPI_USR);
	// 2Mhz (10/4) 0.6 us overlap
	//delayMicroseconds(1);


	// wait for SPI bus to be ready

	data = READ_PERI_REG(SPI_W0(SPI_NO)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!

//  wait for data transmitted before disabling Client Selector
//  Delay = 4 MHz * num bit / convert to micro - HW-Processing Time(factor of num bit)
//	delayMicroseconds(4);

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
//		WRITE_PERI_REG(SPI_USER1(SPI_NO),((numBits-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S |   //Number of bits to Send
//										 ((numBits-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S );  //Number of bits to receive
		WRITE_PERI_REG(SPI_USER1(SPI_NO),
					( (numBits-1 & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S ) |
					( (numBits-1 & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S ) );

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

#ifdef SPI_DEBUG
	debugf("SPIClass::setClock(prediv %d, cntdiv %d)", prediv, cntdiv);
#endif

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

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: spi_mode
//   Description: Configures SPI mode parameters for clock edge and clock polarity.
//    Parameters: spi_no - SPI (0) or HSPI (1)
//				  spi_cpha - (0) Data is valid on clock leading edge
//				             (1) Data is valid on clock trailing edge
//				  spi_cpol - (0) Clock is low when inactive
//				             (1) Clock is high when inactive
//
////////////////////////////////////////////////////////////////////////////////

void SPIClass::spi_mode(uint8 spi_cpha,uint8 spi_cpol){

#ifdef SPI_DEBUG
	debugf("SPIClass::spi_mode(spi_cpha %d,spi_cpol %d)", spi_cpha, spi_cpol);
#endif


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
}

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: spi_tx_byte_order
//   Description: Setup the byte order for shifting data out of buffer
//    Parameters: byte_order - SPI_BYTE_ORDER_HIGH_TO_LOW (1)
//							   Data is sent out starting with Bit31 and down to Bit0
//
//							   SPI_BYTE_ORDER_LOW_TO_HIGH (0)
//							   Data is sent out starting with the lowest BYTE, from
//							   MSB to LSB, followed by the second lowest BYTE, from
//							   MSB to LSB, followed by the second highest BYTE, from
//							   MSB to LSB, followed by the highest BYTE, from MSB to LSB
//							   0xABCDEFGH would be sent as 0xGHEFCDAB
//
//
////////////////////////////////////////////////////////////////////////////////

void SPIClass::spi_byte_order(uint8 tx_byte_order, uint8 rx_byte_order) {

#ifdef SPI_DEBUG
	debugf("SPIClass::spi_byte_order(tx_byte_order %d,rx_byte_order %d)", tx_byte_order, rx_byte_order);
#endif

	if(tx_byte_order){
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
	} else {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
	}
	if(rx_byte_order){
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
	} else {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
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

#ifdef SPI_DEBUG
	uint8 pre = 10; uint8 cdiv = 4;
	debugf("SPIClass::setFrequency(uint32_t freq) testing override pre %d cdiv  %d", pre, cdiv);
	setClock(pre, cdiv); return;
#endif



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

uint32 SPIClass::spi_transaction(uint8 cmd_bits, uint16 cmd_data, uint32 addr_bits, uint32 addr_data, uint32 dout_bits, uint32 dout_data,
				uint32 din_bits, uint32 dummy_bits){


#ifdef SPI_DEBUG
//	debugf("SPIClass::spi_transaction(dout_data %X)",  dout_data);
#endif

	uint8 spi_no = 1;

	//code for custom Chip Select as GPIO PIN here

	while(spi_busy(spi_no)); //wait for SPI to be ready

//########## Enable SPI Functions ##########//
	//disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
	CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

	//enable functions based on number of bits. 0 bits = disabled.
	//This is rather inefficient but allows for a very generic function.
	//CMD ADDR and MOSI are set below to save on an extra if statement.
//	if(cmd_bits) {SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_COMMAND);}
//	if(addr_bits) {SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_ADDR);}
	if(din_bits) {SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO);}
	if(dummy_bits) {SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_DUMMY);}
//########## END SECTION ##########//

//########## Setup Bitlengths ##########//
	WRITE_PERI_REG(SPI_USER1(spi_no), ((addr_bits-1)&SPI_USR_ADDR_BITLEN)<<SPI_USR_ADDR_BITLEN_S | //Number of bits in Address
									  ((dout_bits-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S | //Number of bits to Send
									  ((din_bits-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S |  //Number of bits to receive
									  ((dummy_bits-1)&SPI_USR_DUMMY_CYCLELEN)<<SPI_USR_DUMMY_CYCLELEN_S); //Number of Dummy bits to insert
//########## END SECTION ##########//

//########## Setup Command Data ##########//
	if(cmd_bits) {
		SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_COMMAND); //enable COMMAND function in SPI module
		uint16 command = cmd_data << (16-cmd_bits); //align command data to high bits
		command = ((command>>8)&0xff) | ((command<<8)&0xff00); //swap byte order
		WRITE_PERI_REG(SPI_USER2(spi_no), ((((cmd_bits-1)&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S) | command&SPI_USR_COMMAND_VALUE));
	}
//########## END SECTION ##########//

//########## Setup Address Data ##########//
	if(addr_bits){
		SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_ADDR); //enable ADDRess function in SPI module
		WRITE_PERI_REG(SPI_ADDR(spi_no), addr_data<<(32-addr_bits)); //align address data to high bits
	}


//########## END SECTION ##########//

//########## Setup DOUT data ##########//
	if(dout_bits) {
		SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI); //enable MOSI function in SPI module
	//copy data to W0
	if(READ_PERI_REG(SPI_USER(spi_no))&SPI_WR_BYTE_ORDER) {
		WRITE_PERI_REG(SPI_W0(spi_no), dout_data<<(32-dout_bits));
	} else {

		uint8 dout_extra_bits = dout_bits%8;

		if(dout_extra_bits){
			//if your data isn't a byte multiple (8/16/24/32 bits)and you don't have SPI_WR_BYTE_ORDER set, you need this to move the non-8bit remainder to the MSBs
			//not sure if there's even a use case for this, but it's here if you need it...
			//for example, 0xDA4 12 bits without SPI_WR_BYTE_ORDER would usually be output as if it were 0x0DA4,
			//of which 0xA4, and then 0x0 would be shifted out (first 8 bits of low byte, then 4 MSB bits of high byte - ie reverse byte order).
			//The code below shifts it out as 0xA4 followed by 0xD as you might require.
			WRITE_PERI_REG(SPI_W0(spi_no), ((0xFFFFFFFF<<(dout_bits - dout_extra_bits)&dout_data)<<(8-dout_extra_bits) | (0xFFFFFFFF>>(32-(dout_bits - dout_extra_bits)))&dout_data));
		} else {
			WRITE_PERI_REG(SPI_W0(spi_no), dout_data);
		}
	}
	}
//########## END SECTION ##########//

//########## Begin SPI Transaction ##########//
	SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);
//########## END SECTION ##########//

	// 2Mhz (10/4) 0.6 us overlap
	delayMicroseconds(4);


//########## Return DIN data ##########//
	if(din_bits) {
		while(spi_busy(spi_no));	//wait for SPI transaction to complete

		if(READ_PERI_REG(SPI_USER(spi_no))&SPI_RD_BYTE_ORDER) {
			return READ_PERI_REG(SPI_W0(spi_no)) >> (32-din_bits); //Assuming data in is written to MSB. TBC
		} else {
			return READ_PERI_REG(SPI_W0(spi_no)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!
		}

		return 0; //something went wrong
	}
//########## END SECTION ##########//

	//Transaction completed
	return 1; //success
}





