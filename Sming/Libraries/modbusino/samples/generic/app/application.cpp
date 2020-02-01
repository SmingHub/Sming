#include <SmingCore.h>
#include <Debug.h>
#include <Modbusino.h>

Timer mbSlaveLoopTimer;
Timer mbPrintTimer;
ModbusinoSlave MbSlave(1);
HardwareSerial Serial1(UART1);

uint16_t tab_reg[3] = {0,0,0};

void mbSlaveLoop() {
	uint8_t res = MbSlave.loop(tab_reg,3);
	if( res < 0 ) debugf( "res: %d\r\n");
}

void mbPrint() {
	debugf("Register values, #1: %d, #2: %d, #3: %d", tab_reg[0], tab_reg[1], tab_reg[2]);
}

void init()
{
	MbSlave.setup(115200);
	mbSlaveLoopTimer.initializeMs(10,mbSlaveLoop).start();
	mbPrintTimer.initializeMs(1000,mbPrint).start();
}
