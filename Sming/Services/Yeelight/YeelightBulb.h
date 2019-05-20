/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/
#pragma once

#include "WVector.h"
#include "WString.h"
#include "IPAddress.h"
class TcpClient;

enum YeelightBulbState
{
	eYBS_Unknown = -1,
	eYBS_Off = 0,
	eYBS_On = 1
};

/** @brief Yeelight wifi bulb controller class
 */
class YeelightBulb
{
public:
	YeelightBulb(IPAddress addr) : lamp(addr)
	{
	}

	~YeelightBulb();

	/** @brief Can be skipped. This method will be called automatically from any action method below
	 */
	bool connect();

	/** @brief Send any command to the lamp
	 */
	void sendCommand(const String& method, const Vector<String>& params);

	void on();
	void off();
	void setState(bool isOn);

	/** @brief Start async reading of current lamp state
	 */
	void updateState();
	/** @brief Get current lamp state (should be called only after updateState)
	 */
	YeelightBulbState currentState() { return state; }

	void setBrightness(int percent);
	void setRGB(byte r, byte g, byte b);
	void setHSV(int hue, int sat);

protected:
	void ensureOn();
	bool onResponse(TcpClient& client, char *data, int size);
	void parsePower(const String& resp);

private:
	IPAddress lamp;
	uint16_t port = 55443;

	TcpClient* connection = nullptr;
	long requestId = 0;
	long propsId = 0;
	YeelightBulbState state = eYBS_Unknown;
};
