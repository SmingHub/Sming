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

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Serial.println("Initializing lcd via I2C (IIC/TWI) interface");

	lcd.begin(16, 2); // initialize the lcd for 16 chars 2 lines, turn on backlight

	// ------- Quick 3 blinks of backlight  -------------
	for(int i = 0; i < 3; i++) {
		lcd.backlight();
		delay(150);
		lcd.noBacklight();
		delay(250);
	}
	lcd.backlight(); // finish with backlight on

	//-------- Write characters on the display ------------------
	// NOTE: Cursor Position: (CHAR, LINE) start at 0
	lcd.setCursor(0, 0);
	lcd.print("SMING: Let's do");
	lcd.setCursor(0, 1);
	lcd.print("smart things!");
}
