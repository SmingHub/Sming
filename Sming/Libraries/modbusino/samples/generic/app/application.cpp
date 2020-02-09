#include <SmingCore.h>
#include <Debug.h>
#include <Modbusino.h>

#define ARRLEN 3
uint16_t mbDataArray[ARRLEN] = {0,0,0};
ModbusinoSlave MbSlave(MB_SLAVE_ADDR, mbDataArray, ARRLEN);

HardwareSerial debugComPort(UART1);

void mbPrint() {
	debugf("Register values, #0: %d, #1: %d, #2: %d", mbDataArray[0], mbDataArray[1], mbDataArray[2]);
}

void init()
{
	debugComPort.begin(SERIAL_BAUD_RATE, SERIAL_8N1,
					  SERIAL_TX_ONLY);
	debugComPort.systemDebugOutput(true);
	Debug.setDebug(debugComPort);
	Debug.start();
	MbSlave.setup(SERIAL_BAUD_RATE);
	MbSlave.setRxCallback(mbPrint);
}
