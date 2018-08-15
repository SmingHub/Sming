/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "YeelightBulb.h"
#include "Network/TcpClient.h"
#include "WCharacter.h"
#include "ArduinoJson.h"


YeelightBulb::YeelightBulb(IPAddress addr)
{
	_lamp = addr;
}

YeelightBulb::~YeelightBulb()
{
	delete _connection;
	_connection = nullptr;
}

bool YeelightBulb::connect()
{
	if (_connection) {
		if (_connection->isProcessing())
			return true;

		//connection->close();
		delete _connection;
	}

	_connection = new TcpClient(TcpClientDataDelegate(&YeelightBulb::onResponse, this));

	_connection->setTimeOut(USHRT_MAX); // Stay connected forever
	bool result = _connection->connect(_lamp, _port);
	//if (result) updateState();
	return result;
}


bool isNumeric(String str)
{
	for (unsigned i = 0; i < str.length(); i++)
		if (!isDigit(str[i]))
			return false;

	return true;
}

void YeelightBulb::sendCommand(const String& method, const Vector<String>& params)
{
	connect();

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["id"] = _requestId++;
	root["method"] = method;
	auto &arr = root.createNestedArray("params");
	for (unsigned i = 0; i < params.count(); i++) {
		if (isNumeric(params[i]))
			arr.add(params[i].toInt());
		else
			arr.add(params[i]);
	}
	String request;
	root.printTo(request);
	debugf("LED < %s", request.c_str());
	_connection->writeString(request);
	_connection->writeString("\r\n");
	_connection->flush();
}

void YeelightBulb::on()
{
	Vector<String> params;
	params.add("on");
	sendCommand(F("set_power"), params);
	_state = eYBS_On;
}

void YeelightBulb::off()
{
	Vector<String> params;
	params.add("off");
	sendCommand("set_power", params);
	_state = eYBS_Off;
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
	_propsId = _requestId;
	Vector<String> params;
	params.add("power");
	params.add("bright");
	sendCommand(F("get_prop"), params);
}

void YeelightBulb::setBrightness(int percent)
{
	ensureOn();
	Vector<String> params;
	params.add(String(percent));
	sendCommand(F("set_bright"), params);
}

void YeelightBulb::setRGB(byte r, byte g, byte b)
{
	ensureOn();
	Vector<String> params;
	long val = (long)r*65536 + (long)g*256 + b;
	params.add(String(val));
	sendCommand(F("set_rgb"), params);
}

void YeelightBulb::setHSV(int hue, int sat)
{
	ensureOn();
	Vector<String> params;
	params.add(String(hue));
	params.add(String(sat));
	sendCommand(F("set_hsv"), params);
}

void YeelightBulb::ensureOn()
{
	if (_state <= 0)
		on();
}

void YeelightBulb::parsePower(const String& value)
{
	if (value == "on")
		_state = eYBS_On;
	else if (value == "off")
		_state = eYBS_Off;

	debugf("LED state: %s", value.c_str());
}

bool YeelightBulb::onResponse(TcpClient& client, char* data, int size)
{
	String source(data, size);
	debugf("LED > %s", source.c_str());

	unsigned p = 0;
	while (p < source.length()) {
		int p2 = source.indexOf("\r\n", p);
		if (p2 == -1)
			p2 = source.length();
		String buf = source.substring(p, p2);
		p = p2 + 2;
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(buf);
		bool parsed = root.success();
		if (parsed) {
			if (root.containsKey("id") && root.containsKey("result")) {
				long id = root["id"];
				if (id == _propsId) {
					auto &result = root["result"].asArray();
					String resp = result[0].asString();
					parsePower(resp);
				}
			}

			if (root.containsKey("method") && root.containsKey("params")) {
				String method = root["method"].asString();
				debugf("LED method %s received", method.c_str());
				if (method == "props") {
					auto &result = root["params"].asObject();
					for (JsonObject::iterator it=result.begin(); it!=result.end(); ++it)
						if (strcmp(it->key, "power") == 0)
							parsePower(it->value);
				}
			}
		}
	}
	return true;
}
