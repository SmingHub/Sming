#include <Digital.h>
#include <Hosted/Client.h>

extern Hosted::Client* hostedClient;

void pinMode(uint16_t pin, uint8_t mode)
{
	hostedClient->send(__func__, pin, mode);
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	hostedClient->send(__func__, pin, val);
}

uint8_t digitalRead(uint16_t pin)
{
	hostedClient->send(__func__, pin);
	uint8_t response = hostedClient->wait<uint8_t>();

	return response;
}
