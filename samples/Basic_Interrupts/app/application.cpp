#include <SmingCore.h>

// Input pin for demonstrating a call to a low-level interrupt handler callback
#define INT_PIN_A 0 // GPIO0

// Input pin for demonstrating a call to an InterruptDelegate function
#define INT_PIN_B 4  // GPIO4
#define TOGGLE_PIN 5 // GPIO5

#define say(a) (Serial.print(a))
#define newline() (Serial.println())

static unsigned interruptToggleCount;

void showInterruptToggleCount(uint32_t toggleCount)
{
	say("Toggle count hit ");
	say(toggleCount);
	say(", current value is ");
	say(interruptToggleCount);
	say("!");
	newline();
	say("Max tasks queued: ");
	say(System.getMaxTaskCount());
	newline();
}

/** @brief Low-level interrupt handler
 *  @note An interrupt handling callback must have the IRAM_ATTR attribute.
 *  Interrupt processing code should be as short as possible.
 *  You could perhaps set a flag, then check it in your main code (timers, etc) or read actual
 *  pin state and save it to a global variable.
 *  Avoid doing things like calling malloc(), new(), reading Flash memory.
 *  If your application is not timing-critical, then use an InterruptDelegate callback instead.
 */
void IRAM_ATTR interruptHandler()
{
	// For this example, we just toggle the state of an output pin.
	bool state = digitalRead(TOGGLE_PIN);
	digitalWrite(TOGGLE_PIN, !state);

	// Example of how you can queue a callback from inside a regular interrupt handler
	const unsigned MAX_TOGGLE_COUNTS = 10;
	++interruptToggleCount;
	if(interruptToggleCount > MAX_TOGGLE_COUNTS) {
		System.queueCallback(showInterruptToggleCount, interruptToggleCount);
		/*
		 * Note that `queueCallback` also supports std::function arguments, so we can use lambdas,
		 * class methods, etc.
		 *
		 * For example, we could use a lambda to capture to capture the instantaneous value of 'toggleCount':
		 *
		 * 	```
		 * 	System.queueCallback([toggleCount]() {
		 * 		showInterruptToggleCount(toggleCount);
		 * 	};
		 * 	```
		 *
		 * IMPORTANT: the lambda inherits this function's context, so will be stored in IRAM which
		 * is a very limited resource. The lambda is therefore best suited to simple 'glue' code.
		 *
		 * IMPORTANT: Avoid using std::bind from interrupt handlers because it may attempt to allocate
		 * storage on the heap; this will likely crash the system.
		 *
		 */
		interruptToggleCount = 0;
	}
}

/** @brief Example of an InterruptDelegate function
 *  @note Unlike interruptHandler() above, this function is not called directly from an interrupt so there
 *  are no restrictions on what you can do here, and you don't need to use IRAM_ATTR.
 */
void interruptDelegate()
{
	// For this example, we write some stuff out of the serial port.
	say(micros());
	say("   Pin changed, now   ");
	say(digitalRead(INT_PIN_B));
	newline();

	// Interrupt delegates work by queueing your callback routine, so let's just show you how many requests got queued
	say("Max tasks queued: ");
	say(System.getMaxTaskCount());
	newline();

	/* OK, so you probably got a number which hit 255 pretty quickly! It stays there to indicate the task queue
	 * overflowed, which happens because we're getting way more interrupts than we can process in a timely manner, so
	 * lots of them get dropped. This is because of 'contact bounce'.
	 */
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default

	delay(3000);
	say("======= Bring GPIO");
	say(INT_PIN_A);
	say(" low to trigger interrupt(s) =======");
	newline();

	// Note we enable pullup on our test pin so it will stay high when not connected
	attachInterrupt(INT_PIN_A, interruptHandler, CHANGE);
	pinMode(INT_PIN_A, INPUT_PULLUP);
	say("Interrupt A attached");
	newline();

	// For an interrupt delegate callback, we simply cast our function or method using InterruptDelegate()
	pinMode(TOGGLE_PIN, OUTPUT);
	attachInterrupt(INT_PIN_B, InterruptDelegate(interruptDelegate), CHANGE);
	pinMode(INT_PIN_B, INPUT_PULLUP);
	say("Interrupt B attached");
	newline();
}
