#include <SmingCore.h>

#include <Libraries/MCP_CAN_lib/mcp_can.h>
#include <SPI.h>

unsigned long rxId;
byte len;
byte rxBuf[8];

byte txBuf0[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
byte txBuf1[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

MCP_CAN CAN0(10); // CAN0 interface using CS on digital pin 10
MCP_CAN CAN1(9);  // CAN1 interface using CS on digital pin 9

void init()
{
	Serial.begin(115200);

	// init CAN0 bus, baudrate: 250k@16MHz
	if(CAN0.begin(MCP_EXT, CAN_250KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.print("CAN0: Init OK!\r\n");
		CAN0.setMode(MCP_NORMAL);
	} else {
		Serial.print("CAN0: Init Fail!!!\r\n");
	}

	// init CAN1 bus, baudrate: 250k@16MHz
	if(CAN1.begin(MCP_EXT, CAN_250KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.print("CAN1: Init OK!\r\n");
		CAN1.setMode(MCP_NORMAL);
	} else {
		Serial.print("CAN1: Init Fail!!!\r\n");
	}

	// Set SPI to run at 8MHz (16MHz / 2 = 8 MHz)
	SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));

	CAN0.sendMsgBuf(0x1000000, 1, 8, txBuf0);
	CAN1.sendMsgBuf(0x1000001, 1, 8, txBuf1);
}
