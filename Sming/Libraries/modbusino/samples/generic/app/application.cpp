#include <SmingCore.h>
#include <Debug.h>
#include <Modbusino.h>

Timer mbSlaveLoopTimer;
Timer mbPrintTimer;
ModbusinoSlave MbSlave(MB_SLAVE_ADDR);
HardwareSerial debugComPort(UART1);

uint16_t tab_reg[3] = {0,0,0};

void mbSlaveLoop() {
	uint8_t res = MbSlave.loop(tab_reg,3);
	if( res < 0 ) debugf("error: %d\r\n");
}

void mbPrint() {
	debugf("Register values, #0: %d, #1: %d, #2: %d", tab_reg[0], tab_reg[1], tab_reg[2]);
}

void init()
{
	pinMode(15, OUTPUT);
	debugComPort.begin(SERIAL_BAUD_RATE, SERIAL_8N1,
					  SERIAL_TX_ONLY);
	debugComPort.systemDebugOutput(true);
	Debug.setDebug(debugComPort);
	Debug.start();
	MbSlave.setup();

	mbSlaveLoopTimer.initializeMs(10,mbSlaveLoop).start();
	mbPrintTimer.initializeMs(1000,mbPrint).start();
}
