#include <SmingCore.h>
#include <Debug.h>
#include <ModbusMaster.h>

#define MODBUS_COM_SPEED 115200
#define RS485_RE_PIN 15
#define MB_SLAVE_ADDR 1
#define SLAVE_REG_ADDR 1

Timer mbLoopTimer;

ModbusMaster mbMaster;
HardwareSerial modbusComPort(UART0);
HardwareSerial debugComPort(UART1);

uint16_t globalSeconds = 0;

void mbLoop()
{
	globalSeconds++;

	uint8_t numberOfRegistersToWrite = 1;
	uint8_t bufferPosition = 0;
	mbMaster.begin(MB_SLAVE_ADDR, modbusComPort);
	mbMaster.setTransmitBuffer(bufferPosition, globalSeconds);
	mbMaster.writeMultipleRegisters(SLAVE_REG_ADDR, numberOfRegistersToWrite);

	uint8_t nrOfRegistersToRead = 1;
	uint8_t mbResult = mbMaster.readHoldingRegisters(SLAVE_REG_ADDR, nrOfRegistersToRead); //see also readInputRegisters

	if(mbResult == mbMaster.ku8MBSuccess) {
		/*
		for(i = 0; i < nrOfRegisters; i++) {
			buffer[i] = mbMaster.getResponseBuffer(i);
			debugComPort.printf("Reg %d: %d\r\n", i, buffer[i]);
		}
		*/
		debugf("Data from slave: %d", mbMaster.getResponseBuffer(0));
	} else {
		debugf("Res err: %d", mbResult);
	}

	mbMaster.clearResponseBuffer();
}

void preTransmission()
{
	digitalWrite(RS485_RE_PIN, HIGH);
	delayMilliseconds(2);
}

void postTransmission()
{
	delayMicroseconds(500);
	digitalWrite(RS485_RE_PIN, LOW);
}

void mbLogReceive(const uint8_t* adu, size_t aduSize, uint8_t status)
{
	if(status != mbMaster.ku8MBSuccess) {
		switch(status) {
		case mbMaster.ku8MBIllegalFunction:
			debugf("MB Illegal Function");
			break;
		case mbMaster.ku8MBIllegalDataAddress:
			debugf("MB Illegal Address");
			break;
		case mbMaster.ku8MBIllegalDataValue:
			debugf("MB Illegal Data Value");
			break;
		case mbMaster.ku8MBSlaveDeviceFailure:
			debugf("MB Slave Device Failure");
			break;
		case mbMaster.ku8MBInvalidSlaveID:
			debugf("MB Invalid Slave ID");
			break;
		case mbMaster.ku8MBInvalidFunction:
			debugf("MB Invalid function");
			break;
		case mbMaster.ku8MBResponseTimedOut:
			debugf("MB Response Timeout");
			break;
		case mbMaster.ku8MBInvalidCRC:
			debugf("MB Invalid CRC");
			break;
		case mbMaster.ku8MBResponseTooLarge:
			debugf("MB Response too large");
			break;
		}
		debugf("ADU Size: %d, status: %d ", aduSize, status);
	}
	debugf("\r\n");
}

void mbLogTransmit(const uint8_t* adu, size_t aduSize)
{
	debug_hex(INFO, "ADU", adu, aduSize);
}

void init()
{
	pinMode(RS485_RE_PIN, OUTPUT);
	digitalWrite(RS485_RE_PIN, LOW);
	modbusComPort.begin(MODBUS_COM_SPEED, SERIAL_8N1, SERIAL_FULL);
	debugComPort.begin(SERIAL_BAUD_RATE, SERIAL_8N1,
				  SERIAL_TX_ONLY); // 115200 by default, GPIO1,GPIO3, see Serial.swap(), HardwareSerial
	debugComPort.systemDebugOutput(true);

	Debug.setDebug(debugComPort);
	Debug.initCommand();
	Debug.start();
	Debug.printf("This is the debug output\r\n");

	mbMaster.preTransmission(preTransmission);
	mbMaster.postTransmission(postTransmission);
	mbMaster.logReceive(mbLogReceive);
	mbMaster.logTransmit(mbLogTransmit);

	mbLoopTimer.initializeMs(1000, mbLoop).start();
}
