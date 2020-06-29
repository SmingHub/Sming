#include <Digital.h>
#include <HostedClient.h>

extern HostedClient hostedClient;

void pinMode(uint16_t pin, uint8_t mode)
{
	INIT_HD_COMMAND(PinMode);
	command->pin = pin;
	command->mode = (PinMode)mode;

	hostedClient.send(message);
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	INIT_HD_COMMAND(DigitalWrite);
	command->pin = pin;
	command->value = val;

	hostedClient.send(message);
}
