/*
 * This sample application demonstrates RPC communication via the serial interface
 */
#include <SmingCore.h>
#include <simpleRPC.h>
#include <Hosted/Transport/SerialTransport.h>
#include <Wire.h>

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
			 * List of exported commands. More commands can be added.
			 * For every command one should specify command and text description in the format below.
			 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
			 */
			pinMode, F("pinMode> Sets mode of digital pin. @pin: Pin number, @mode: Mode type."),
			digitalRead, F("digitalRead> Read digital pin. @pin: Pin number. @return: Pin value."),
			digitalWrite, F("digitalWrite> Write to a digital pin. @pin: Pin number. @value: Pin value."),
			pulseIn, F("pulseIn> Measure duration of pulse on pin. @pin: Pin number. @state:  State of pulse to measure. @timeout: Maximum duration of pulse. @return: Pulse duration in microseconds)"),
			// void TwoWire::begin(uint8_t sda, uint8_t scl)
			pack(&Wire, (void(TwoWire::*)(uint8_t,uint8_t))&TwoWire::begin), F("TwoWire::begin> Starts two-wire communication. @sda: Data pin. @scl: Clock pin."),
			// void TwoWire::begin()
			pack(&Wire, (void(TwoWire::*)(void))&TwoWire::begin), F("TwoWire::begin> Starts two-wire communication."),
			pack(&Wire, &TwoWire::pins), F("TwoWire::pins> Starts two-wire communication. @sda: Data pin. @scl: Clock pin."),
			pack(&Wire, &TwoWire::status), F("TwoWire::status> Get status."),
			pack(&Wire, &TwoWire::end), F("TwoWire::end> Ends two-wire communication."),
			pack(&Wire, &TwoWire::setClock), F("TwoWire::setClock> Sets clock frequency. @freq: clock frequency."),
			pack(&Wire, &TwoWire::setClockStretchLimit), F("TwoWire::setClockStretchLimit> Sts clock strech limit. @limit: stretch limit."),
			pack(&Wire, &TwoWire::requestFrom), F("TwoWire::requestFrom> Request from. @address: Address. @size: Size. @sendStop flag.  @retrun: uint8_t."),
			pack(&Wire, &TwoWire::beginTransmission), F("TwoWire::beginTransmission> Begin transmission. @address: Address."),
			pack(&Wire, &TwoWire::endTransmission), F("TwoWire::endTransmission> End transmission. @sendStop: flag. @return: error code"),
			// size_t TwoWire::write(uint8_t data)
			pack(&Wire, (size_t(TwoWire::*)(uint8_t))&TwoWire::write), F("TwoWire::write> Write byte. @data: byte. @return: written bytes"),
			// size_t TwoWire::write(const uint8_t* data, size_t quantity)
			pack(&Wire, (size_t(TwoWire::*)(const uint8_t*, size_t))&TwoWire::write), F("TwoWire::write> Write bytes. @data: data pointer. @quantity: data size. @return: written bytes"),
			pack(&Wire, &TwoWire::available), F("TwoWire::available> Available bytes. @return: count"),
			pack(&Wire, &TwoWire::read), F("TwoWire::read> Read a byte. @return: byte"),
			pack(&Wire, &TwoWire::peek), F("TwoWire::peek> Peek. @return: byte without advancing the internal pointer."),
			pack(&Wire, &TwoWire::flush), F("TwoWire::flush> Flush.")
		);
		// clang-format on

		return true;
	});
}
