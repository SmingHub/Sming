/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/WHashMap.h"
#include "../Wiring/WVector.h"
#include "../Wiring/WString.h"
#include "IPAddress.h"
class TcpClient;

#ifndef SERVICES_YEELIGHTBULB_H_
#define SERVICES_YEELIGHTBULB_H_

class YeelightBulb
{
public:
	YeelightBulb(IPAddress addr);
	bool connect();

	void sendCommand(String method, Vector<String> params);
	void on();
	void off();
	void updateState();
	bool currentState() { return state > 0; }

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
	long propsId;
	int state = -1;
};

#endif /* SERVICES_YEELIGHTBULB_H_ */
