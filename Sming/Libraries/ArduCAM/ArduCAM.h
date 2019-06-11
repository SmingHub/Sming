/*
 /*
  ArduCAM.h - Arduino library support for CMOS Image Sensor
  Copyright (C)2011-2015 ArduCAM.com. All right reserved
  
  Basic functionality of this library are based on the demo-code provided by
  ArduCAM.com. You can find the latest version of the library at
  http://www.ArduCAM.com

  Now supported controllers:
		-	OV7670
		-	MT9D111
		-	OV7675
		-	OV2640
		-	OV3640
		-	OV5642
		-	OV7660
		-	OV7725
		- MT9M112
		- MT9V111
		- OV5640
		- MT9M001
		- MT9T112
		- MT9D112

	We will add support for many other sensors in next release.
	
  Supported MCU platform
 		-	Theoretically support all Arduino families
  		-	Arduino UNO R3			(Tested)
  		-	Arduino MEGA2560 R3		(Tested)
  		-	Arduino Leonardo R3		(Tested)
  		-	Arduino Nano			(Tested)
  		-	Arduino DUE				(Tested)
  		- Arduino Yun				(Tested)  		
  		-	Raspberry Pi			(Tested)
  		- ESP8266-12				(Tested)

  If you make any modifications or improvements to the code, I would appreciate
  that you share the code with me so that I might include it in the next release.
  I can be contacted through http://www.ArduCAM.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*------------------------------------
	Revision History:
	2012/09/20 	V1.0.0	by Lee	first release 
	2012/10/23  V1.0.1  by Lee  Resolved some timing issue for the Read/Write Register	
	2012/11/29	V1.1.0	by Lee  Add support for MT9D111 sensor
	2012/12/13	V1.2.0	by Lee	Add support for OV7675 sensor
	2012/12/28  V1.3.0	by Lee	Add support for OV2640,OV3640,OV5642 sensors
	2013/02/26	V2.0.0	by Lee	New Rev.B shield hardware, add support for FIFO control 
															and support Mega1280/2560 boards 
	2013/05/28	V2.1.0	by Lee	Add support all drawing functions derived from UTFT library 			
	2013/08/24	V3.0.0	by Lee	Support ArudCAM shield Rev.C hardware, features SPI interface and low power mode.
								Support almost all series of Arduino boards including DUE.	
	2014/03/09  V3.1.0  by Lee  Add the more impressive example sketches. 
								Optimise the OV5642 settings, improve image quality.
								Add live preview before JPEG capture.
								Add play back photos one by one	after BMP capture.
	2014/05/01  V3.1.1  by Lee  Minor changes to add support Arduino IDE for linux distributions.	
	2014/09/30  V3.2.0  by Lee  Improvement on OV5642 camera dirver.			
	2014/10/06  V3.3.0  by Lee  Add OV7660,OV7725 camera support.			
	2015/02/27  V3.4.0  by Lee  Add the support for Arduino Yun board, update the latest UTFT library for ArduCAM.
	2015/06/09  V3.4.1  by Lee	Minor changes and add some comments
	2015/06/19  V3.4.2  by Lee	Add support for MT9M112 camera.
	2015/06/20  V3.4.3  by Lee	Add support for MT9V111 camera.
	2015/06/22  V3.4.4  by Lee	Add support for OV5640 camera.
	2015/06/22  V3.4.5  by Lee	Add support for MT9M001 camera.
	2015/08/05  V3.4.6  by Lee	Add support for MT9T112 camera.
	2015/08/08  V3.4.7  by Lee	Add support for MT9D112 camera.
	2015/09/20  V3.4.8  by Lee	Add support for ESP8266 processor.
--------------------------------------*/


#ifndef ArduCAM_H
#define ArduCAM_H

#include "Arduino.h"
#include <pins_arduino.h>

#if defined (__AVR__)
#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) pgm_read_byte(&cfont.font[x])  

#define regtype volatile uint8_t
#define regsize uint8_t

#endif
	
#if defined(__arm__)

#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) cfont.font[x]  

#define regtype volatile uint32_t
#define regsize uint32_t
#define PROGMEM


    #define pgm_read_byte(x)        (*((char *)x))
//  #define pgm_read_word(x)        (*((short *)(x & 0xfffffffe)))
    #define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_byte_near(x)   (*((char *)x))
    #define pgm_read_byte_far(x)    (*((char *)x))
//  #define pgm_read_word_near(x)   (*((short *)(x & 0xfffffffe))
//  #define pgm_read_word_far(x)    (*((short *)(x & 0xfffffffe)))
    #define pgm_read_word_near(x)   ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_word_far(x)    ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x))))
    #define PSTR(x)  x
  #if defined F
    #undef F
  #endif
  #define F(X) (X)

	
#endif	

#if defined(ESP8266) || defined(__ESP8266_EX__)

#define cbi(reg, bitmask) digitalWrite(bitmask, LOW)
#define sbi(reg, bitmask) digitalWrite(bitmask, HIGH)
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) cfont.font[x]

#define regtype volatile uint32_t
#define regsize uint32_t

#endif

/****************************************************/
/* Sensor related definition 												*/
/****************************************************/
#define BMP 	0
#define JPEG	1

#define OV7670		0
#define MT9D111_A	1
#define OV7675		2
#define OV5642		3
#define OV3640  	4
#define OV2640  	5
#define OV9655		6
#define MT9M112		7
#define OV7725		8
#define OV7660		9
#define MT9M001 	10
#define OV5640 		11
#define MT9D111_B	12
#define OV9650		13
#define MT9V111		14
#define MT9T112		15
#define MT9D112		16

#define OV2640_160x120 		0	//160x120
#define OV2640_176x144 		1	//176x144
#define OV2640_320x240 		2	//320x240
#define OV2640_352x288 		3	//352x288
#define OV2640_640x480		4	//640x480
#define OV2640_800x600 		5	//800x600
#define OV2640_1024x768		6	//1024x768
#define OV2640_1280x1024	7	//1280x1024
#define OV2640_1600x1200	8	//1600x1200

#define OV5642_320x240 		1	//320x240
#define OV5642_640x480		2	//640x480
#define OV5642_1280x720 	3	//1280x720
#define OV5642_1920x1080	4	//1920x1080
#define OV5642_2048x1563	5	//2048x1563
#define OV5642_2592x1944	6	//2592x1944

/****************************************************/
/* I2C Control Definition 													*/
/****************************************************/
#define I2C_ADDR_8BIT 0
#define I2C_ADDR_16BIT 1
#define I2C_REG_8BIT 0
#define I2C_REG_16BIT 1
#define I2C_DAT_8BIT 0
#define I2C_DAT_16BIT 1

/* Register initialization tables for SENSORs */
/* Terminating list entry for reg */
#define SENSOR_REG_TERM_8BIT                0xFF
#define SENSOR_REG_TERM_16BIT               0xFFFF
/* Terminating list entry for val */
#define SENSOR_VAL_TERM_8BIT                0xFF
#define SENSOR_VAL_TERM_16BIT               0xFFFF

/****************************************************/
/* ArduChip related definition 											*/
/****************************************************/
#define RWBIT					0x80  //READ AND WRITE BIT IS BIT[7]

#define ARDUCHIP_TEST1       	0x00  //TEST register
#define ARDUCHIP_TEST2      	0x01  //TEST register

#define ARDUCHIP_FRAMES			0x01  //Bit[2:0]Number of frames to be captured

#define ARDUCHIP_MODE      		0x02  //Mode register
#define MCU2LCD_MODE       		0x00
#define CAM2LCD_MODE       		0x01
#define LCD2MCU_MODE       		0x02

#define ARDUCHIP_TIM       		0x03  //Timming control
#define HREF_LEVEL_MASK    		0x01  //0 = High active , 		1 = Low active
#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
#define LCD_BKEN_MASK      		0x04  //0 = Enable, 			1 = Disable
#define DELAY_MASK         		0x08  //0 = no delay, 			1 = delay one clock
#define MODE_MASK          		0x10  //0 = LCD mode, 			1 = FIFO mode
#define FIFO_PWRDN_MASK	   		0x20  //0 = Normal operation, 	1 = FIFO power down
#define LOW_POWER_MODE			0x40  //0 = Normal mode, 		1 = Low power mode

#define ARDUCHIP_FIFO      		0x04  //FIFO and I2C control
#define FIFO_CLEAR_MASK    		0x01
#define FIFO_START_MASK    		0x02
#define FIFO_RDPTR_RST_MASK     0x10
#define FIFO_WRPTR_RST_MASK     0x20

#define ARDUCHIP_GPIO			0x06  //GPIO Write Register
#define GPIO_RESET_MASK			0x01  //0 = default state,		1 =  Sensor reset IO value
#define GPIO_PWDN_MASK			0x02  //0 = Sensor power down IO value, 1 = Sensor power enable IO value

#define BURST_FIFO_READ			0x3C  //Burst FIFO read operation
#define SINGLE_FIFO_READ		0x3D  //Single FIFO read operation

#define ARDUCHIP_REV       		0x40  //ArduCHIP revision
#define VER_LOW_MASK       		0x3F
#define VER_HIGH_MASK      		0xC0

#define ARDUCHIP_TRIG      		0x41  //Trigger source
#define VSYNC_MASK         		0x01
#define SHUTTER_MASK       		0x02
#define CAP_DONE_MASK      		0x08

#define FIFO_SIZE1				0x42  //Camera write FIFO size[7:0] for burst to read
#define FIFO_SIZE2				0x43  //Camera write FIFO size[15:8]
#define FIFO_SIZE3				0x44  //Camera write FIFO size[18:16]


/****************************************************/


/****************************************************************/
/* define a structure for sensor register initialization values */
/****************************************************************/
struct sensor_reg {
	uint16_t reg;
	uint16_t val;
};

class ArduCAM
{
	public:
		ArduCAM();
		ArduCAM(byte model,int CS);
		void InitCAM();
		
		void CS_HIGH(void);
		void CS_LOW(void);

		void flush_fifo(void);
		void start_capture(void);
		void clear_fifo_flag(void);
		uint8_t read_fifo(void);
		
		uint8_t read_reg(uint8_t addr);
		void write_reg(uint8_t addr, uint8_t data);

		uint32_t read_fifo_length(void);
		void set_fifo_burst(void);
		void set_bit(uint8_t addr, uint8_t bit);
		void clear_bit(uint8_t addr, uint8_t bit);
		uint8_t get_bit(uint8_t addr, uint8_t bit);
		void set_mode(uint8_t mode);
		
		int wrSensorRegs(const struct sensor_reg*);
		int wrSensorRegs8_8(const struct sensor_reg*);
		int wrSensorRegs8_16(const struct sensor_reg*);
		int wrSensorRegs16_8(const struct sensor_reg*);
		int wrSensorRegs16_16(const struct sensor_reg*);
		
		byte wrSensorReg(int regID, int regDat);
		byte wrSensorReg8_8(int regID, int regDat);
		byte wrSensorReg8_16(int regID, int regDat);
		byte wrSensorReg16_8(int regID, int regDat);
		byte wrSensorReg16_16(int regID, int regDat);
		
		byte rdSensorReg8_8(uint8_t regID, uint8_t* regDat);
		byte rdSensorReg16_8(uint16_t regID, uint8_t* regDat);
		byte rdSensorReg8_16(uint8_t regID, uint16_t* regDat);
		byte rdSensorReg16_16(uint16_t regID, uint16_t* regDat);
		
		void OV2640_set_JPEG_size(uint8_t size);
		void OV5642_set_JPEG_size(uint8_t size);
		void set_format(byte fmt);
		byte get_format();
		
		void bus_write(int address, int value);
		uint8_t bus_read(int address);
	protected:
		regtype *P_CS;
		regsize B_CS;
  		byte m_fmt;
		byte sensor_model;
		byte sensor_addr;

};

#endif
