#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/MCP23017/MCP23017.h>

#define LED_PIN 2 // GPIO2

MCP23017 mcp;
volatile boolean awakenByInterrupt = false;
byte mcpPinA = 0;
byte interruptPin = 5;

void interruptCallback() {
	awakenByInterrupt = true;
	Serial.println("Interrupt Called");
	while (!(mcp.digitalRead(mcpPinA)));
}

void init() {

	Serial.begin(115200);

	mcp.begin(0);
	mcp.pinMode(8, OUTPUT);
	mcp.pinMode(9, OUTPUT);
	mcp.pullUp(8, HIGH);
	mcp.pullUp(9, HIGH);
	mcp.digitalWrite(8, LOW);
	mcp.digitalWrite(9, HIGH);

	pinMode(interruptPin, INPUT);
	pullup(interruptPin);

	mcp.setupInterrupts(true, false, LOW);
	mcp.pinMode(mcpPinA, INPUT);
	mcp.pullUp(mcpPinA, HIGH);
	mcp.setupInterruptPin(mcpPinA, FALLING);
	attachInterrupt(interruptPin, interruptCallback, FALLING);
}
