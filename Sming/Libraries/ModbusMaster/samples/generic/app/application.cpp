#include <SmingCore.h>
#include <ModbusMaster.h>

#define MODBUS_COM_SPEED 115200
#define RS485_RE_PIN 15
#define MB_SLAVE_ADDR 1
#define SLAVE_REG_ADDR 1

namespace
{
SimpleTimer mbLoopTimer;
ModbusMaster mbMaster;
HardwareSerial modbusComPort(UART_ID_0);
HardwareSerial debugComPort(UART_ID_1);

uint16_t globalSeconds;

void mbLoop()
{
	globalSeconds++;

	const uint8_t numberOfRegistersToWrite = 1;
	const uint8_t bufferPosition = 0;
	mbMaster.begin(MB_SLAVE_ADDR, modbusComPort);
	mbMaster.setTransmitBuffer(bufferPosition, globalSeconds);
	mbMaster.writeMultipleRegisters(SLAVE_REG_ADDR, numberOfRegistersToWrite);

	// see also readInputRegisters
	const uint8_t nrOfRegistersToRead = 1;
	uint8_t mbResult = mbMaster.readHoldingRegisters(SLAVE_REG_ADDR, nrOfRegistersToRead);

	if(mbResult == mbMaster.ku8MBSuccess) {
		/*
		for(i = 0; i < nrOfRegisters; i++) {
			buffer[i] = mbMaster.getResponseBuffer(i);
			debugComPort.printf("Reg %d: %d\r\n", i, buffer[i]);
		}
		*/
		debug_i("Data from slave: %d", mbMaster.getResponseBuffer(0));
	} else {
		debug_i("Res err: %d", mbResult);
	}

	mbMaster.clearResponseBuffer();
}

void preTransmission()
{
	digitalWrite(RS485_RE_PIN, HIGH);
}

void postTransmission()
{
	digitalWrite(RS485_RE_PIN, LOW);
}

void mbLogReceive(const uint8_t* adu, size_t aduSize, uint8_t status)
{
	if(status != mbMaster.ku8MBSuccess) {
		switch(status) {
		case mbMaster.ku8MBIllegalFunction:
			debug_i("MB Illegal Function");
			break;
		case mbMaster.ku8MBIllegalDataAddress:
			debug_i("MB Illegal Address");
			break;
		case mbMaster.ku8MBIllegalDataValue:
			debug_i("MB Illegal Data Value");
			break;
		case mbMaster.ku8MBSlaveDeviceFailure:
			debug_i("MB Slave Device Failure");
			break;
		case mbMaster.ku8MBInvalidSlaveID:
			debug_i("MB Invalid Slave ID");
			break;
		case mbMaster.ku8MBInvalidFunction:
			debug_i("MB Invalid function");
			break;
		case mbMaster.ku8MBResponseTimedOut:
			debug_i("MB Response Timeout");
			break;
		case mbMaster.ku8MBInvalidCRC:
			debug_i("MB Invalid CRC");
			break;
		case mbMaster.ku8MBResponseTooLarge:
			debug_i("MB Response too large");
			break;
		}
		debug_i("ADU Size: %d, status: %d ", aduSize, status);
		debug_hex(INFO, "RX ADU", adu, aduSize);
	}
	debug_i("\r\n");
}

void mbLogTransmit(const uint8_t* adu, size_t aduSize)
{
	debug_hex(INFO, "TX ADU", adu, aduSize);
}

} // namespace

void init()
{
	pinMode(RS485_RE_PIN, OUTPUT);
	digitalWrite(RS485_RE_PIN, LOW);
	modbusComPort.begin(MODBUS_COM_SPEED, SERIAL_8N1, SERIAL_FULL);
	debugComPort.begin(SERIAL_BAUD_RATE, SERIAL_8N1, SERIAL_TX_ONLY);
	debugComPort.systemDebugOutput(true);

	mbMaster.preTransmission(preTransmission);
	mbMaster.postTransmission(postTransmission);
	mbMaster.logReceive(mbLogReceive);
	mbMaster.logTransmit(mbLogTransmit);

	mbLoopTimer.initializeMs<1000>(mbLoop).start();
}
