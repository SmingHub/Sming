/*
 * application.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: harry
 */

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/SPI.h>

//#include <Libraries/ArduCAM/ArduCAM.h>
//#include <Libraries/ArduCAM/ov2640_regs.h>

#include "HexDump.h"

#define SPI_SCLK 	14
#define SPI_MOSI 	13
#define SPI_MISO 	12

#define CAM_CS		16

#define CAM_SCL		5
#define CAM_SDA		4

#define TFT_SCLK 	14
#define TFT_MOSI 	13
#define TFT_RST  	16
#define	TFT_DC   	0
#define TFT_CS   	2

uint32_t startTime;

/*
ArduCAM myCAM(OV2640, CAM_CS);

void start_capture(){
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}
*/


/*
void dumpImage(int len) {

#define BUFSIZE		1024

	HexDump hdump;

	char data[1024];

	debugf("readFifoBirst\n");

	char dummy;

	HwSPI.startTX();
	myCAM.set_fifo_burst();

	// clear dummy byte -> send out LO and ignore MISO
	HwSPI.send((uint8)0x0);

	// first loop split up in Buffers
	int blocks = ((len -1)/BUFSIZE) + 1;
	debugf("Reading total [%d] blocks of total [%d] bytes\n", blocks, len);

//	for (int i=0; i< blocks; i++) {
	for (int i=0; i< 3; i++) {
		HwSPI.recv((uint8_t *)data, BUFSIZE);
		hdump.print((uint8_t *)data, BUFSIZE);
		debugf("Block [%d] read operation: %d ms", i, millis() - startTime);
	}

	HwSPI.endTX();


}

void capture() {

	  start_capture();
	  Serial.println("CAM Capturing");

	  int total_time = 0;

	  total_time = millis();
	  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
	  total_time = millis() - total_time;
	  Serial.print("capture total_time used (in miliseconds):");
	  Serial.println(total_time, DEC);

	  size_t len = myCAM.read_fifo_length();
	  if (len >= 393216){
	    Serial.println("Over size.");
	    return;
	  } else if (len == 0 ){
	    Serial.println("Size is 0.");
	    return;
	  }
	  Serial.printf("Image size %d bytes\n", len);

	  dumpImage(len);
}

void setup() {

	uint8_t vid, pid;
	uint8_t temp;

	Serial.begin(115200);
	Serial.println("ArduCAM Start!");

	// init i2c
	// initialize I2C
	Wire.pins(CAM_SCL, CAM_SDA);
	Wire.begin();

	HwSPI.begin(HSPI);
	HwSPI.spi_clock(HSPI,5,2);
	HwSPI.setCS(CAM_CS);

	//Check if the ArduCAM SPI bus is OK
	myCAM.write_reg(ARDUCHIP_TEST1, 0x55);

	temp = myCAM.read_reg(ARDUCHIP_TEST1);
	if (temp != 0x55) {
		Serial.println("SPI1 interface Error!");
		while (1);
	} else {
		Serial.println("SPI1 interface OK!");
	}

	//Check if the camera module type is OV2640
	myCAM.wrSensorReg8_8(0xff, 0x01);
	myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
	myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	if((vid != 0x26) || (pid != 0x42)) {
		Serial.println("Can't find OV2640 module!");
		Serial.printf("vid = [%X]  pid = [%X]\n", vid, pid);
	} else {
		Serial.println("OV2640 detected.");
	}

	//Change to JPEG capture mode and initialize the OV2640 module
	myCAM.set_format(JPEG);
	myCAM.InitCAM();
	myCAM.OV2640_set_JPEG_size(OV2640_640x480);
	myCAM.clear_fifo_flag();
	myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);


}
*/

void setupSPI() {

	SPI.begin(TFT_CS);

	SPISettings setA(400000, MSBFIRST, SPI_MODE1);
	SPISettings setB(200000, MSBFIRST, SPI_MODE1);

//	setA.print(String("setA"));
//	setB.print(String("setB"));

	SPI.beginTransaction(setA);
	SPI.transfer(0x44);
	SPI.transfer(0x45);
	SPI.endTransaction();


	SPI.beginTransaction(setB);
	debugf("Result transfer 1 %X", SPI.transfer(0x46));
	debugf("Result transfer 2 %X", SPI.transfer(0x47));
	SPI.endTransaction();

	SPI.beginTransaction(setB);
	SPI.transfer16(0x1234);
	SPI.transfer16(0x5678);
	SPI.endTransaction();


}

void init() {

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	startTime = millis();
	setupSPI();
	debugf("Setup done: %d ms", millis() - startTime);

//	startTime = millis();
//	capture();
//	debugf("Capture done: %d ms", millis() - startTime);



}

