#include <SmingCore.h>
#include <Debug.h>
#include <CS5460.h>

CS5460 powerMeter(PIN_NDEFINED, PIN_NDEFINED, PIN_NDEFINED, PIN_NDEFINED);

Timer printVoltageTimer;

void printVoltage()
{
	debugf("Measured RMS voltage is: %f", powerMeter.getRMSVoltage());
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE, SERIAL_8N1,
				 SERIAL_FULL); // 115200 by default, GPIO1,GPIO3, see Serial.swap(), HardwareSerial
	Serial.systemDebugOutput(true);
	Debug.setDebug(Serial);

	powerMeter.init();
	powerMeter.setCurrentGain(190.84); //0.25 / shunt (0.00131)
	powerMeter.setVoltageGain(500);	//0.25V (Veff max) * dividerGain
	powerMeter.writeRegister(CONFIG_REGISTER, ENABLE_CURRENT_HPF);
	powerMeter.writeRegister(CONFIG_REGISTER, ENABLE_VOLTAGE_HPF);
	powerMeter.startMultiConvert();

	printVoltageTimer.initializeMs(1000, printVoltage).start();
}
