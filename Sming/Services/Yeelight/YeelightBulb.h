/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WString.h"
#include "WVector.h"
#include "IPAddress.h"

class TcpClient;

#ifndef SERVICES_YEELIGHTBULB_H_
#define SERVICES_YEELIGHTBULB_H_

enum YeelightBulbState {
	eYBS_Unknown = -1,
	eYBS_Off = 0,
	eYBS_On = 1
};

/** @brief Yeelight wifi bulb controller class
 */
class YeelightBulb {
public:
	YeelightBulb(IPAddress addr);
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
	YeelightBulbState currentState()
	{
		return _state;
	}

	void setBrightness(int percent);
	void setRGB(byte r, byte g, byte b);
	void setHSV(int hue, int sat);

protected:
	void ensureOn();
	bool onResponse(TcpClient& client, char *data, int size);
	void parsePower(const String& resp);

private:
	IPAddress _lamp;
	uint16_t _port = 55443;

	TcpClient* _connection = nullptr;
	long _requestId = 0;
	long _propsId = 0;
	YeelightBulbState _state = eYBS_Unknown;
};

#endif /* SERVICES_YEELIGHTBULB_H_ */
