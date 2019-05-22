#include <SmingCore.h>
#include <Libraries/MCP23017/MCP23017.h>

MCP23017 mcp;
volatile bool awakenByInterrupt = false;
byte mcpPinA = 0;
byte interruptPin = 15;

void interruptCallback()
{
	awakenByInterrupt = true;
	Serial.println("Interrupt Called");
	while(!(mcp.digitalRead(mcpPinA)))
		;
}

void init()
{
	Serial.begin(COM_SPEED_SERIAL);

	// You can select ESP I2C pins here:
	//Wire.pins(5, 4); // SDA, SCL

	mcp.begin(0); // 0 - for default mcp address, possible values: 0..7

	mcp.pinMode(8, OUTPUT);
	mcp.pinMode(9, OUTPUT);
	mcp.pullUp(8, HIGH);
	mcp.pullUp(9, HIGH);
	mcp.digitalWrite(8, LOW);
	mcp.digitalWrite(9, HIGH);

	pinMode(interruptPin, INPUT_PULLUP);

	mcp.setupInterrupts(true, false, LOW);
	mcp.pinMode(mcpPinA, INPUT);
	mcp.pullUp(mcpPinA, HIGH);
	mcp.setupInterruptPin(mcpPinA, FALLING);
	attachInterrupt(interruptPin, InterruptDelegate(interruptCallback), FALLING);
}
