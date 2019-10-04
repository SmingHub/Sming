/**
 * Basic Servo test application
 *
*/

#include <SmingCore.h>
#include <Libraries/Servo/ServoChannel.h>
#include <Libraries/Servo/Servo.h>

// Un-comment to use raw time values instead of degrees for servo positioning
//#define UPDATE_RAW

#define UPDATE_INTERVAL 500

// Enable to show current frame timing
//#define SHOW_FRAME_TIMING

class MyServoChannel : public ServoChannel
{
public:
	bool begin(uint8_t pin, unsigned intervalMs)
	{
		if(!attach(pin)) {
			return false; // Out of channels
		}
		return timer.initializeMs(intervalMs, TimerDelegate(&MyServoChannel::calcValue, this)).start();
	}

protected:
	void calcValue();

private:
	Timer timer;
	uint16 centerdelay = 0;
	uint32 value = 0;
	int degree = 0;
};

static MyServoChannel channels[4];

void MyServoChannel::calcValue()
{
	const char indent[] = "                    ";

	auto pin = getPin();
	Serial.write(indent, pin);
	Serial.print("GPIO");
	Serial.print(pin);

#ifdef UPDATE_RAW

	// Wait some time at middle of the servo range
	if(value == 1000) {
		centerdelay--;
		if(centerdelay == 0) {
			value += 10; // restart after waiting
		}
	} else {			  // linear ramp by increasing the value in steps
		centerdelay = 50; // delay 50 times 200ms = 10s
		value += 10;
	}

	if(value >= 2000) {
		value = 0; // overflow and restart linear ramp
	}

	Serial.print(" value = ");
	Serial.print(value);

	if(!setValue(value)) {
		Serial.print(": setValue(");
		Serial.print(value);
		Serial.print(") failed!");
	}

#else

	if(degree == 90) {
		degree = -90;
	} else {
		++degree;
	}

	Serial.print(" degree = ");
	Serial.print(degree);

	if(!setDegree(degree)) {
		Serial.print(": setDegree(");
		Serial.print(degree);
		Serial.print(") failed!");
	}

#endif

#ifdef SHOW_FRAME_TIMING
	// List active frame details
	auto& frame = servo.getActiveFrame();
	String buf = ": ";
	buf.reserve(256);
	for(unsigned i = 0; i < frame.slotCount; ++i) {
		if(i > 0) {
			buf += ", ";
		}
		buf += frame.slots[i];
	}
	Serial.print(buf);
#endif

	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	Serial.setTxWait(false);
	Serial.setTxBufferSize(1024);

	Serial.println("Init Basic Servo Sample");
	System.setCpuFrequency(eCF_80MHz);

#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	// Set up two channels, one updating slowly the other more quickly
	channels[0].begin(2, UPDATE_INTERVAL * 1.3);
	channels[1].begin(4, UPDATE_INTERVAL);
	channels[2].begin(5, UPDATE_INTERVAL * 0.5);
	channels[3].begin(0, UPDATE_INTERVAL * 0.25);
}
