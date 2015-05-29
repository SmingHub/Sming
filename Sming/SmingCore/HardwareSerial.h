/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _HARDWARESERIAL_H_
#define _HARDWARESERIAL_H_

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/Stream.h"
#include "../SmingCore/Delegate.h"

#define UART_ID_0   0
#define UART_ID_1   1

DELEGATE_CALLBACK (HardwareSerial,void,unsigned short)

class HardwareSerial : public Stream
{
public:
	HardwareSerial(const int uartPort);

	void begin(const uint32_t baud = 9600);

	int available();
	int read();
	int readMemoryBlock(char* buf, int max_len);
	int peek();
	void flush();
	size_t write(uint8_t oneChar);

	//void printf(const char *fmt, ...);
	void systemDebugOutput(bool enabled);
	void setCallback(HardwareSerialCallback reqCallback);
	void setCallback(HardwareSerialDelegate reqDelegate);
	void resetCallback();

	static void IRAM_ATTR uart0_rx_intr_handler(void *para);

private:
	int uart;
	static HardwareSerialDelegate HWSDelegates[2];

};

extern HardwareSerial Serial;

#endif /* _HARDWARESERIAL_H_ */
