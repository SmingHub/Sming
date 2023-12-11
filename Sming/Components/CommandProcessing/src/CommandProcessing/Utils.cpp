#include "Utils.h"

namespace CommandProcessing
{
void enable(Handler& commandHandler, HardwareSerial& serial)
{
	commandHandler.setOutputStream(&serial, false);
	Serial.onDataReceived([&commandHandler](Stream& source, char arrivedChar, uint16_t availableCharsCount) {
		while(availableCharsCount--) {
			commandHandler.process(source.read());
		}
	});
}

} // namespace CommandProcessing
