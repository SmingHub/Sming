#pragma once

#include <Delegate.h>

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "nimconfig.h"
#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#include "SwitchJoyconConnection.h"
#include <NimBLEHIDDevice.h>
#include <NimBLECharacteristic.h>

class SwitchJoycon
{
public:
	enum class Type {
		Left = 0,
		Right,
		ProController = 3,
	};

	enum class Button {
		ButtonA = 0,
		ButtonX,
		ButtonB,
		ButtonY,
		ButtonSl,
		ButtonSr,

		ButtonMunus = 8,
		ButtonPlus,

		ButtonHome = 12,
		ButtonCapture,
		ButtonStickrl,
		ButtonZrl,
	};

	enum class JoystickPosition {
		Right = 0,
		DownRight,
		Down,
		DownLeft,
		Left,
		UpLeft,
		Up,
		UpRight,
		Center,
	};

	struct Gamepad // {00 00} 08 {00 80} {00 80} {00 80} {00 80}
	{
		uint8_t buttons[2];
		uint8_t hat;
		uint8_t leftX[2];
		uint8_t leftY[2];
		uint8_t rightX[2];
		uint8_t rightY[2];
	};

	SwitchJoycon(Type type, uint8_t batteryLevel = 100, SwitchJoyconConnection::Callback onConnected = nullptr,
				 SwitchJoyconConnection::Callback onDisconnected = nullptr)
		: controllerType(type), batteryLevel(batteryLevel)
	{
		connectionStatus = new SwitchJoyconConnection(onConnected, onDisconnected);
	}

	virtual ~SwitchJoycon()
	{
		end();
	}

	bool begin();

	void end();

	void setType(Type type)
	{
		controllerType = type;
	}

	void setBatteryLevel(uint8_t level);

	// Buttons

	void press(Button button)
	{
		press(static_cast<uint8_t>(button));
	}

	void press(uint8_t button);

	void release(Button button)
	{
		release(static_cast<uint8_t>(button));
	}

	void release(uint8_t button);

	// Set Axis Values

	void setXAxis(int16_t value);
	void setYAxis(int16_t value);
	void setZAxis(int16_t value);
	void setZAxisRotation(int16_t value);

	// Hat

	void setHat(JoystickPosition position);

	void setAutoReport(bool autoReport)
	{
		this->autoReport = autoReport;
	}

	void sendReport();
	bool isPressed(uint8_t button);
	bool isConnected();
	void resetButtons();

protected:
	virtual void onStarted(NimBLEServer* pServer){};

private:
	bool started{false};
	TaskHandle_t taskHandle{nullptr}; // owned

	// Joystick Type
	Type controllerType;

	// Gamepad State
	Gamepad state;
	uint8_t batteryLevel{0};
	uint8_t buttonCount{0};
	uint8_t hatSwitchCount{0};

	bool autoReport{true};
	size_t reportSize{0};

	// HID Settings
	NimBLEHIDDevice* hid{nullptr}; // owned
	uint8_t hidReportId{0};

	// Connection status and gamepad
	SwitchJoyconConnection* connectionStatus{nullptr};
	NimBLECharacteristic* inputGamepad{nullptr}; // owned

	static void startServer(void* pvParameter);
};

#endif // CONFIG_BT_NIMBLE_ROLE_PERIPHERAL
#endif // CONFIG_BT_ENABLED
