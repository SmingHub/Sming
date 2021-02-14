#include <Digital.h>
#include <HostedClient.h>

extern HostedClient* hostedClient;

void pinMode(uint16_t pin, uint8_t mode)
{
	NEW_HD_COMMAND(message, PinMode, {
		command->pin = pin;
		command->mode = (PinMode)mode;
	});

	hostedClient->send(&message);
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	NEW_HD_COMMAND(message, DigitalWrite, {
		command->pin = pin;
		command->value = val;
	});

	hostedClient->send(&message);
}

uint8_t digitalRead(uint16_t pin)
{
	NEW_HD_COMMAND(message, DigitalRead, { command->pin = pin; });

	hostedClient->send(&message);
	HostedCommand response = hostedClient->wait();

	return uint8_t(response.payload.responseDigitalRead.value);
}
