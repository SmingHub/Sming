/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "YeelightBulb.h"
#include "Network/TcpClient.h"
#include "Digital.h"
#include "Data/ArduinoJson.h"
#include "WCharacter.h"

YeelightBulb::~YeelightBulb()
{
	if (connection != nullptr)
		delete connection;
	connection = nullptr;
}

bool YeelightBulb::connect()
{
	if (connection != nullptr)
	{
		if (connection->isProcessing())
			return true;

		//connection->close();
		delete connection;
	}

	connection = new TcpClient(TcpClientDataDelegate(&YeelightBulb::onResponse, this));

	connection->setTimeOut(USHRT_MAX); // Stay connected forever
	bool result = connection->connect(lamp, port);
	//if (result) updateState();
	return result;
}

bool isNumeric(String str)
{
  for (unsigned i = 0; i < str.length(); i++)
  {
	  if (!isDigit(str[i]))
		  return false;
  }
  return true;
}

void YeelightBulb::sendCommand(const String& method, const Vector<String>& params)
{
	connect();

	DynamicJsonDocument doc(1024);
	doc["id"] = requestId++;
	doc["method"] = method;
	auto arr = doc.createNestedArray("params");
	for (unsigned i = 0; i < params.count(); i++)
	{
		if (isNumeric(params[i]))
			arr.add(params[i].toInt());
		else
			arr.add(params[i]);
	}
	String request = Json::serialize(doc);
	request += "\r\n";
	debugf("LED < %s", request.c_str());
	connection->writeString(request);
	connection->flush();
}

void YeelightBulb::on()
{
	Vector<String> params;
	params.add("on");
	sendCommand("set_power", params);
	state = eYBS_On;
}

void YeelightBulb::off()
{
	Vector<String> params;
	params.add("off");
	sendCommand("set_power", params);
	state = eYBS_Off;
}

void YeelightBulb::setState(bool isOn)
{
	if (isOn)
		on();
	else
		off();
}

void YeelightBulb::updateState()
{
	propsId = requestId;
	Vector<String> params;
	params.add("power");
	params.add("bright");
	sendCommand("get_prop", params);
}

void YeelightBulb::setBrightness(int percent)
{
	ensureOn();
	Vector<String> params;
	params.add(String(percent));
	sendCommand("set_bright", params);
}

void YeelightBulb::setRGB(byte r, byte g, byte b)
{
	ensureOn();
	Vector<String> params;
	long val = (long)r*65536 + (long)g*256 + b;
	params.add(String(val));
	sendCommand("set_rgb", params);
}

void YeelightBulb::setHSV(int hue, int sat)
{
	ensureOn();
	Vector<String> params;
	params.add(String(hue));
	params.add(String(sat));
	sendCommand("set_hsv", params);
}

void YeelightBulb::ensureOn()
{
	if (state <= 0)
		on();
}

void YeelightBulb::parsePower(const String& value)
{
	if (value == "on")
		state = eYBS_On;
	else if (value == "off")
		state = eYBS_Off;

	debugf("LED state: %s", value.c_str());
}

bool YeelightBulb::onResponse(TcpClient& client, char* data, int size)
{
	String source(data, size);
	debugf("LED > %s", source.c_str());

	unsigned p = 0;
	while (p < source.length())
	{
		int p2 = source.indexOf("\r\n", p);
		if (p2 < 0)
			p2 = source.length();
		String buf = source.substring(p, p2);
		p = unsigned(p2) + 2;
		DynamicJsonDocument doc(1024);
		if(Json::deserialize(doc, buf))
		{
			long id = doc["id"] | -1;
			if (id == propsId)
			{
				JsonArray result = doc["result"];
				const char* value = result[0];
				if(value != nullptr) {
					parsePower(value);
				}
			}

			const char* method = doc["method"];
			debugf("LED method %s received", method);
			if (strcmp(method,"props") == 0)
			{
				JsonObject result = doc["params"];
				const char* value = result["power"];
				if(value != nullptr) {
					parsePower(value);
				}
			}
		}
	}
	return true;
}
