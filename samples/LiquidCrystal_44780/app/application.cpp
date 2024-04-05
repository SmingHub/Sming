// LiquidCrystal_I2C example
// pinout:
// * GPIO0 SCL
// * GPIO2 SDA
// Can be changed by call Wire.pins(...)

#include <SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal_I2C.h>

// For more information visit useful wiki page: http://arduino-info.wikispaces.com/LCD-Blue-I2C
#define I2C_LCD_ADDR 0x27

LiquidCrystal_I2C lcd(I2C_LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

SimpleTimer flashTimer;

//-------- Write characters on the display ------------------
void writeDisplay()
{
	debug_d("%s", __FUNCTION__);

	// NOTE: Cursor Position: (CHAR, LINE) start at 0
	lcd.setCursor(0, 0);
	lcd.print(_F("SMING: Let's do"));
	lcd.setCursor(0, 1);
	lcd.print(_F("smart things!"));
}

// ------- Quick 3 blinks of backlight  -------------
void flashBacklight()
{
	static unsigned state;

	if(state == 6) {
		debug_d("%s DONE", __FUNCTION__);
		lcd.backlight(); // finish with backlight on
		writeDisplay();
		return;
	}

	if(state == 0) {
		debug_d("%s INIT", __FUNCTION__);
		flashTimer.setCallback(flashBacklight);
	}

	if(state & 0x01) {
		debug_d("%s OFF", __FUNCTION__);
		lcd.noBacklight();
		flashTimer.setIntervalMs<250>();
	} else {
		debug_d("%s ON", __FUNCTION__);
		lcd.backlight();
		flashTimer.setIntervalMs<150>();
	}

	++state;
	flashTimer.startOnce();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Serial.println(_F("Initializing lcd via I2C (IIC/TWI) interface"));

	lcd.begin(16, 2); // initialize the lcd for 16 chars 2 lines, turn on backlight

	flashBacklight();
}
