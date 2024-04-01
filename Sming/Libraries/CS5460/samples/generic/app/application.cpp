#include <SmingCore.h>
#include <CS5460.h>

namespace
{
CS5460 powerMeter(PIN_NDEFINED, PIN_NDEFINED, PIN_NDEFINED, PIN_NDEFINED);
SimpleTimer printVoltageTimer;

void printVoltage()
{
	Serial << _F("Measured RMS voltage is: ") << powerMeter.getRMSVoltage() << endl;
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	powerMeter.init();
	powerMeter.setCurrentGain(190.84); // 0.25 / shunt (0.00131)
	powerMeter.setVoltageGain(500);	// 0.25V (Veff max) * dividerGain

	uint32_t conf = powerMeter.readRegister(CONFIG_REGISTER);
	conf |= ENABLE_VOLTAGE_HPF | ENABLE_CURRENT_HPF;
	powerMeter.writeRegister(CONFIG_REGISTER, conf);
	powerMeter.startMultiConvert();

	printVoltageTimer.initializeMs<1000>(printVoltage).start();
}
