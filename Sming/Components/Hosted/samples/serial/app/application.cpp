/*
 * This sample application demostrates communication via the serial interface
 */

#include <SmingCore.h>
#include <Hosted/Transport/SerialStream.h>
#include <simpleRPC.h>

using namespace Hosted::Transport;

SerialStream* serialStream = nullptr;

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

	serialStream = new SerialStream(Serial);
	serialStream->onData([](Stream& stream) {
		interface(stream, pinMode, "pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type.",
				  digitalRead, "digitalRead: Read digital pin. @pin: Pin number. @return: Pin value.", digitalWrite,
				  "digitalWrite: Write to a digital pin. @pin: Pin number. @value: Pin value.");

		return true;
	});
}
