/*
 * This sample application demostrates RPC communication via the serial interface
 */
#include <SmingCore.h>
#include <simpleRPC.h>
#include <Hosted/Transport/SerialTransport.h>

using namespace Hosted::Transport;

SerialTransport* transport = nullptr;

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

	transport = new SerialTransport(Serial);
	transport->onData([](Stream& stream) {
		// clang-format off
		interface(stream,
			/*
			 * Below we are exporting the following remote commands:
			 * - pinMode
			 * - digitalRead
			 * - digitalWrite
			 * You can add more commands here. For every command you should specify command and text description in the format below.
			 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
			 */
			pinMode, F("pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type."),
			digitalRead, F("digitalRead: Read digital pin. @pin: Pin number. @return: Pin value."),
			digitalWrite, F("digitalWrite: Write to a digital pin. @pin: Pin number. @value: Pin value.")
		);
		// clang-format on

		return true;
	});
}
