#include <SmingCore.h>

#include <Libraries/MCP_CAN_lib/mcp_can.h>
#include <SPI.h>

unsigned long rxId = 0;
uint8_t len = 0;
uint8_t rxBuf[8] = {0};

uint8_t txBuf0[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
uint8_t txBuf1[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

MCP_CAN canBus0(10); // CAN0 interface using CS on digital pin 10
MCP_CAN canBus1(9);  // CAN1 interface using CS on digital pin 9

Timer loopTimer;

void loop()
{
	if(!digitalRead(2)) {						 // If pin 2 is low, read CAN0 receive buffer
		canBus0.readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)
		canBus1.sendMsgBuf(rxId, 1, len, rxBuf); // Immediately send message out CAN1 interface
	}
	if(!digitalRead(3)) {						 // If pin 3 is low, read CAN1 receive buffer
		canBus1.readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)
		canBus0.sendMsgBuf(rxId, 1, len, rxBuf); // Immediately send message out CAN0 interface
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

	// init CAN0 bus, baudrate: 250k@16MHz
	if(canBus0.begin(MCP_EXT, CAN_250KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.print("CAN0: Init OK!\r\n");
		canBus0.setMode(MCP_NORMAL);
	} else {
		Serial.print("CAN0: Init Fail!!!\r\n");
	}

	// init CAN1 bus, baudrate: 250k@16MHz
	if(canBus1.begin(MCP_EXT, CAN_250KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.print("CAN1: Init OK!\r\n");
		canBus1.setMode(MCP_NORMAL);
	} else {
		Serial.print("CAN1: Init Fail!!!\r\n");
	}

	// Set SPI to run at 8MHz (16MHz / 2 = 8 MHz)
	SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));

	canBus0.sendMsgBuf(0x1000000, 1, 8, txBuf0);
	canBus1.sendMsgBuf(0x1000001, 1, 8, txBuf1);

	loopTimer.initializeMs(1000, loop).start();
}
