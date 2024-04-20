/*
 * This sample application demonstrates RPC communication via the serial interface
 */
#include <SmingCore.h>
#include <simpleRPC.h>
#include <Hosted/Transport/SerialTransport.h>
#include <Wire.h>

using namespace Hosted::Transport;

SerialTransport transport(Serial);

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

	using namespace simpleRPC;

	transport.onData([](Stream& stream) {
		// clang-format off
		interface(stream,
			/*
			 * List of exported commands. More commands can be added.
			 * For every command one should specify command and text description in the format below.
			 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
			 */
			pinMode, F("pinMode> Sets mode of digital pin. @pin: Pin number, @mode: Mode type."),
			digitalRead, F("digitalRead> Read digital pin. @pin: Pin number. @return: Pin value."),
			digitalWrite, F("digitalWrite> Write to a digital pin. @pin: Pin number. @value: Pin value."),
			pulseIn, F("pulseIn> Measure duration of pulse on pin. @pin: Pin number. @state:  State of pulse to measure. @timeout: Maximum duration of pulse. @return: Pulse duration in microseconds)"),
			// void TwoWire::begin(uint8_t sda, uint8_t scl)
			makeTuple(&Wire, static_cast<void(TwoWire::*)(uint8_t,uint8_t)>(&TwoWire::begin)), F("TwoWire::begin> Starts two-wire communication. @sda: Data pin. @scl: Clock pin."),
			// void TwoWire::begin()
			makeTuple(&Wire, static_cast<void(TwoWire::*)(void)>(&TwoWire::begin)), F("TwoWire::begin> Starts two-wire communication."),
			makeTuple(&Wire, &TwoWire::pins), F("TwoWire::pins> Starts two-wire communication. @sda: Data pin. @scl: Clock pin."),
			makeTuple(&Wire, &TwoWire::status), F("TwoWire::status> Get status."),
			makeTuple(&Wire, &TwoWire::end), F("TwoWire::end> Ends two-wire communication."),
			makeTuple(&Wire, &TwoWire::setClock), F("TwoWire::setClock> Sets clock frequency. @freq: clock frequency."),
			makeTuple(&Wire, &TwoWire::setClockStretchLimit), F("TwoWire::setClockStretchLimit> Sts clock stretch limit. @limit: stretch limit."),
			makeTuple(&Wire, &TwoWire::requestFrom), F("TwoWire::requestFrom> Request from. @address: Address. @size: Size. @sendStop flag.  @return: uint8_t."),
			makeTuple(&Wire, &TwoWire::beginTransmission), F("TwoWire::beginTransmission> Begin transmission. @address: Address."),
			makeTuple(&Wire, &TwoWire::endTransmission), F("TwoWire::endTransmission> End transmission. @sendStop: flag. @return: error code"),
			// size_t TwoWire::write(uint8_t data)
			makeTuple(&Wire, static_cast<size_t(TwoWire::*)(uint8_t)>(&TwoWire::write)), F("TwoWire::write> Write byte. @data: byte. @return: written bytes"),
			// size_t TwoWire::write(const uint8_t* data, size_t quantity)
			makeTuple(&Wire, static_cast<size_t(TwoWire::*)(const uint8_t*, size_t)>(&TwoWire::write)), F("TwoWire::write> Write bytes. @data: data pointer. @quantity: data size. @return: written bytes"),
			makeTuple(&Wire, &TwoWire::available), F("TwoWire::available> Available bytes. @return: count"),
			makeTuple(&Wire, &TwoWire::read), F("TwoWire::read> Read a byte. @return: byte"),
			makeTuple(&Wire, &TwoWire::peek), F("TwoWire::peek> Peek. @return: byte without advancing the internal pointer."),
			makeTuple(&Wire, &TwoWire::flush), F("TwoWire::flush> Flush.")
		);
		// clang-format on

		return true;
	});
}
