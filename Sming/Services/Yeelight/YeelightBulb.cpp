/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "YeelightBulb.h"
#include "TcpClient.h"
#include <SmingCore.h>

YeelightBulb::YeelightBulb(IPAddress addr)
{
	lamp = addr;
}

bool YeelightBulb::connect()
{
	if (connection != nullptr)
	{
		if (connection->isProcessing())
			return true;

		connection->close();
	}
	else
		connection = new TcpClient(TcpClientDataDelegate(&YeelightBulb::onResponse, this));

	bool result = connection->connect(lamp, port);
	//if (result) updateState();
	return result;
}

bool isNumeric(String str)
{
  for (int i = 0; i < str.length(); i++)
  {
	  if (!isDigit(str[i]))
		  return false;
  }
  return true;
}

void YeelightBulb::sendCommand(String method, Vector<String> params)
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["id"] = requestId++;
	root["method"] = method;
	auto &arr = root.createNestedArray("params");
	for (int i = 0; i < params.count(); i++)
	{
		if (isNumeric(params[i]))
			arr.add(params[i].toInt());
		else
			arr.add(params[i]);
	}
	String request;
	root.printTo(request);
	request += "\r\n";
	debugf("LED < %s", request.c_str());
	connection->sendString(request);
	connection->flush();
}

void YeelightBulb::on()
{
	connect();
	Vector<String> params;
	params.add("on");
	sendCommand("set_power", params);
	state = true;
}

void YeelightBulb::off()
{
	connect();
	Vector<String> params;
	params.add("off");
	sendCommand("set_power", params);
	state = false;
}

void YeelightBulb::updateState()
{
	connect();
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

void YeelightBulb::ensureOn()
{
	connect();
	if (state <= 0)
		on();
}

bool YeelightBulb::onResponse(TcpClient& client, char* data, int size)
{
	String source(data, size);
	debugf("LED > %s", source.c_str());

	int p = 0;
	while (p < source.length())
	{
		int p2 = source.indexOf("\r\n", p);
		if (p2 == -1)
			p2 = source.length();
		String buf = source.substring(p, p2);
		p = p2 + 2;
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(buf);
		bool parsed = root.success();
		if (parsed)
		{
			if (root.containsKey("id") && root.containsKey("result"))
			{
				long id = root["id"];
				if (id == propsId)
				{
					auto &result = root["result"].asArray();
					String resp = result[0].asString();
					if (resp == "on")
						state = true;
					else if (resp == "off")
						state = false;
					debugf("LED state: %s", resp.c_str());
				}
			}
		}
	}
	return true;
}
