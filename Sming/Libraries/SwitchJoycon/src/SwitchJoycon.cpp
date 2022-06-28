#include "SwitchJoycon.h"

#include <WString.h>
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <HIDTypes.h>
#include <debug_progmem.h>

#if DEBUG_VERBOSE_LEVEL == 3
#include <Services/HexDump/HexDump.h>
HexDump dump;
#endif

// TODO: Move report description to progmem
uint8_t tempHidReportDescriptor[150];
int hidReportDescriptorSize = 0;

constexpr uint8_t GAMEPAD_DEFAULT_REPORT_ID = 63;
constexpr uint8_t JOYSTICK_TYPE_GAMEPAD = 0x05;

void SwitchJoycon::resetButtons()
{
	memset(&state.buttons, 0, sizeof(state.buttons));
}

bool SwitchJoycon::begin()
{
	if(started) {
		debug_w("Service already started");
		return false;
	}

	uint8_t axisCount = 0;
	buttonCount = 0;
	hatSwitchCount = 0;
	switch(controllerType) {
	case Type::Left:
		/* fall through */
	case Type::Right:
		buttonCount = 16; // buttonCount;
		hatSwitchCount = 1;
		axisCount = 4; // x, y, z and z rotation
		break;
	case Type::ProController:
		buttonCount = 16; // buttonCount;
		hatSwitchCount = 1;
		axisCount = 4; // x, y, z and z rotation
		break;
	}

	started = true;
	state.hat = static_cast<uint8_t>(JoystickPosition::Center);
	state.leftX[0] = 0x00;
	state.leftX[1] = 0x80;
	state.leftY[0] = 0x00;
	state.leftY[1] = 0x80;
	state.rightX[0] = 0x00;
	state.rightX[1] = 0x80;
	state.rightY[0] = 0x00;
	state.rightY[1] = 0x80;

	/**
	 * For HID debugging see: https://gitlab.freedesktop.org/libevdev/hid-tools
	 * For HID report descriptors see: https://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/
	 */

	hidReportDescriptorSize = 0;

	// USAGE_PAGE (Generic Desktop)
	tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_PAGE(1);
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

	// USAGE (Joystick - 0x04; Gamepad - 0x05; Multi-axis Controller - 0x08)
	tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
	tempHidReportDescriptor[hidReportDescriptorSize++] = JOYSTICK_TYPE_GAMEPAD;

	// COLLECTION (Application)
	tempHidReportDescriptor[hidReportDescriptorSize++] = COLLECTION(1);
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

	// REPORT_ID (Default: 3)
	tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_ID(1);
	tempHidReportDescriptor[hidReportDescriptorSize++] = GAMEPAD_DEFAULT_REPORT_ID;

	if(buttonCount > 0) {
		// USAGE_PAGE (Button)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_PAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

		// USAGE_MINIMUM (Button 1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_MINIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// USAGE_MAXIMUM (Button 16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_MAXIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = buttonCount;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MINIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MAXIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_SIZE (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_SIZE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_COUNT (# of buttons)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_COUNT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = buttonCount;

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = HIDINPUT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
	} // buttonCount

	if(hatSwitchCount > 0) {
		// USAGE_PAGE (Generic Desktop)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_PAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// USAGE (Hat Switch)
		for(int i = 0; i < hatSwitchCount; i++) {
			tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;
		}

		// Logical Min (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MINIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// Logical Max (7)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MAXIMUM(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;

		// Report Size (4)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_SIZE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

		// Report Count (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_COUNT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = hatSwitchCount;

		// Input (Data, Variable, Absolute)
		tempHidReportDescriptor[hidReportDescriptorSize++] = HIDINPUT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x42;

		// -- Padding for the 4 unused bits in the hat switch byte --
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_PAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

		// Report Size (4)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_SIZE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

		// Report Count (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_COUNT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// Input (Cnst,Arr,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = HIDINPUT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
	}

	if(axisCount > 0) {
		// USAGE_PAGE (Generic Desktop)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE_PAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// USAGE (X)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x30;

		// USAGE (Y)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x31;

		// USAGE (Rx)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x33;

		// USAGE (Ry)
		tempHidReportDescriptor[hidReportDescriptorSize++] = USAGE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x34;

		// LOGICAL_MINIMUM(255)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MINIMUM(2);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (255)
		tempHidReportDescriptor[hidReportDescriptorSize++] = LOGICAL_MAXIMUM(3);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// REPORT_SIZE (16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_SIZE(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;

		// REPORT_COUNT (axisCount)
		tempHidReportDescriptor[hidReportDescriptorSize++] = REPORT_COUNT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = axisCount;

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = HIDINPUT(1);
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
	}

	// END_COLLECTION
	tempHidReportDescriptor[hidReportDescriptorSize++] = END_COLLECTION(0);

	xTaskCreate(startServer, "server", 20000, (void*)this, 5, &taskHandle);

	return true;
}

void SwitchJoycon::end()
{
	if(!started) {
		return;
	}

	if(taskHandle != nullptr) {
		vTaskDelete(taskHandle);
	}

	NimBLEDevice::deinit(true);
	delete hid;
	hid = nullptr;
	delete inputGamepad;
	inputGamepad = nullptr;
	connectionStatus = nullptr;
	memset(&state, 0, sizeof(state));

	started = false;
}

void SwitchJoycon::sendReport(void)
{
	if(!isConnected()) {
		return;
	}

	debug_d("Sending report ....");

#if DEBUG_VERBOSE_LEVEL == 3
	dump.resetAddr();
	dump.print(reinterpret_cast<uint8_t*>(&state), sizeof(state));
#endif

	debug_d("=================");

	this->inputGamepad->setValue(state);
	this->inputGamepad->notify();
}

bool SwitchJoycon::isPressed(uint8_t button)
{
	uint8_t index = button / 8;
	uint8_t bit = button % 8;

	return bitRead(state.buttons[index], bit);
}
void SwitchJoycon::press(uint8_t button)
{
	if(isPressed(button)) {
		return;
	}

	uint8_t index = button / 8;
	uint8_t bit = button % 8;

	bitSet(state.buttons[index], bit);

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::release(uint8_t button)
{
	if(!isPressed(button)) {
		return;
	}

	uint8_t index = button / 8;
	uint8_t bit = button % 8;

	bitClear(state.buttons[index], bit);

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::setHat(JoystickPosition position)
{
	if(state.hat == static_cast<uint8_t>(position)) {
		return;
	}

	state.hat = static_cast<uint8_t>(position);

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::setXAxis(int16_t value)
{
	// TODO: add value checks

	state.leftX[0] = value;
	state.leftX[1] = value >> 8;

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::setYAxis(int16_t value)
{
	// TODO: add value checks

	state.leftY[0] = value;
	state.leftY[1] = value >> 8;

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::setZAxis(int16_t value)
{
	// TODO: add value checks
	//	if(value == -32768) {
	//		value = -32767;
	//	}

	state.rightX[0] = value;
	state.rightX[1] = value >> 8;

	if(autoReport) {
		sendReport();
	}
}

void SwitchJoycon::setZAxisRotation(int16_t value)
{
	// TODO: add value checks

	state.rightY[0] = value;
	state.rightY[1] = value >> 8;

	if(autoReport) {
		sendReport();
	}
}

bool SwitchJoycon::isConnected(void)
{
	if(connectionStatus == nullptr) {
		return false;
	}

	return connectionStatus->connected;
}

void SwitchJoycon::setBatteryLevel(uint8_t level)
{
	batteryLevel = level;
	if(hid != nullptr) {
		hid->setBatteryLevel(batteryLevel);
	}
}

void SwitchJoycon::startServer(void* arg)
{
	SwitchJoycon* joycon = static_cast<SwitchJoycon*>(arg);

	String deviceName;
	uint16_t productId = 0;
	// See: http://gtoal.com/vectrex/vecx-colour/SDL/src/joystick/controller_type.h
	switch(joycon->controllerType) {
	case Type::Left:
		deviceName = F("Joy-Con (L)");
		productId = 0x2006;
		break;
	case Type::Right:
		deviceName = F("Joy-Con (R)");
		productId = 0x2007;
		break;
	case Type::ProController:
		deviceName = F("Pro Controller");
		productId = 0x2009;
		break;
	}

	NimBLEDevice::init(deviceName.c_str());
	NimBLEServer* server = NimBLEDevice::createServer();
	server->setCallbacks(joycon->connectionStatus);

	delete joycon->hid; // TODO: ?!
	joycon->hid = new NimBLEHIDDevice(server);
	joycon->inputGamepad = joycon->hid->inputReport(GAMEPAD_DEFAULT_REPORT_ID);
	joycon->connectionStatus->inputGamepad = joycon->inputGamepad;

	joycon->hid->manufacturer()->setValue("Nintendo");
	joycon->hid->pnp(0x01, __builtin_bswap16(0x057e), __builtin_bswap16(productId), 0x0110);
	joycon->hid->hidInfo(0x00, 0x01);

	NimBLEDevice::setSecurityAuth(true, true, true);

#if DEBUG_VERBOSE_LEVEL == 3
	dump.resetAddr();
	dump.print(tempHidReportDescriptor, hidReportDescriptorSize);
#endif

	joycon->hid->reportMap(tempHidReportDescriptor, hidReportDescriptorSize);
	joycon->hid->startServices();

	joycon->onStarted(server);

	NimBLEAdvertising* pAdvertising = server->getAdvertising();
	pAdvertising->setAppearance(HID_GAMEPAD);
	pAdvertising->addServiceUUID(joycon->hid->hidService()->getUUID());
	pAdvertising->start();
	joycon->hid->setBatteryLevel(joycon->batteryLevel);

	debug_d("Advertising started!");

	vTaskDelay(portMAX_DELAY);
}
